#include "static_glb.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string_view>

// Keep third-party implementation warnings out of the project's /W4 /WX code.
#pragma warning(push, 0)
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#pragma warning(pop)

namespace engine {
StaticGlb read_static_glb(const std::filesystem::path& path)
{
    const auto require = [&](bool ok, const char* reason) {
        if (!ok) throw std::runtime_error("Engine static GLB '" + path.string() + "': " + reason);
    };
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    require(file.is_open(), "cannot open source file");
    const auto length = file.tellg();
    require(length >= 20 && length <= 16 * 1024 * 1024, "invalid container size (limit 16 MiB)");
    std::vector<char> bytes(static_cast<std::size_t>(length));
    file.seekg(0);
    require(static_cast<bool>(file.read(bytes.data(), static_cast<std::streamsize>(bytes.size()))), "file read failed");
    cgltf_options options{};
    cgltf_data* raw = nullptr;
    const auto parsed = cgltf_parse(&options, bytes.data(), bytes.size(), &raw);
    std::unique_ptr<cgltf_data, decltype(&cgltf_free)> data(raw, cgltf_free);
    require(parsed == cgltf_result_success, "invalid GLB/JSON container");
    require(data->file_type == cgltf_file_type_glb && data->asset.version
        && std::string_view(data->asset.version) == "2.0", "requires GLB 2.0");
    require(data->buffers_count == 1 && data->buffers[0].uri == nullptr && data->bin != nullptr,
        "requires a single embedded BIN buffer; external/data URIs unsupported");
    require(data->extensions_used_count == 0 && data->extensions_required_count == 0,
        "extensions/compression unsupported");
    require(data->materials_count == 0 && data->textures_count == 0 && data->images_count == 0,
        "materials/textures unsupported");
    require(data->meshes_count == 1 && data->meshes[0].primitives_count == 1,
        "requires one mesh with one primitive");
    require(data->scenes_count == 1 && data->scene == &data->scenes[0], "requires one default scene");
    for (std::size_t i = 0; i < data->accessors_count; ++i)
        require(!data->accessors[i].is_sparse, "sparse accessors unsupported");
    for (std::size_t i = 0; i < data->buffer_views_count; ++i)
        require(!data->buffer_views[i].has_meshopt_compression, "meshopt buffer unsupported");
    require(cgltf_load_buffers(&options, data.get(), nullptr) == cgltf_result_success, "BIN buffer load failed");
    require(cgltf_validate(data.get()) == cgltf_result_success, "invalid accessors, indices or hierarchy");

    StaticGlb result;
    const cgltf_node* mesh_node = nullptr;
    for (std::size_t i = 0; i < data->nodes_count; ++i) {
        const auto& node = data->nodes[i];
        require(!node.has_mesh_gpu_instancing && node.weights_count == 0, "instancing/morph weights unsupported");
        // Bound parent traversal before cgltf's static transform helper walks it.
        const cgltf_node* root = &node;
        std::size_t depth = 0;
        while (root->parent) {
            require(++depth <= data->nodes_count, "cyclic node hierarchy");
            root = root->parent;
        }
        require(std::find(data->scene->nodes, data->scene->nodes + data->scene->nodes_count, root)
            != data->scene->nodes + data->scene->nodes_count, "node outside default scene");
        StaticGlbNode out;
        out.name = node.name ? node.name : "";
        out.parent_name = node.parent && node.parent->name ? node.parent->name : "";
        cgltf_node_transform_world(&node, out.world.data());
        require(std::all_of(out.world.begin(), out.world.end(), [](float x) { return std::isfinite(x); }),
            "non-finite node transform");
        const auto& m = out.world;
        require(m[3] == 0 && m[7] == 0 && m[11] == 0 && m[15] == 1, "non-affine node transform");
        const float determinant = m[0]*(m[5]*m[10]-m[6]*m[9]) - m[4]*(m[1]*m[10]-m[2]*m[9])
            + m[8]*(m[1]*m[6]-m[2]*m[5]);
        require(std::isfinite(determinant) && determinant > 0, "singular/reflected node transforms unsupported");
        result.nodes.push_back(std::move(out));
        if (node.mesh) {
            require(mesh_node == nullptr && node.mesh == &data->meshes[0], "requires one mesh instance");
            mesh_node = &node;
        }
    }
    require(mesh_node != nullptr, "mesh has no scene node");
    auto& mesh = data->meshes[0];
    auto& primitive = mesh.primitives[0];
    require(mesh.weights_count == 0 && primitive.targets_count == 0 && !primitive.has_draco_mesh_compression,
        "morph targets/Draco unsupported");
    require(primitive.type == cgltf_primitive_type_triangles && primitive.material == nullptr,
        "requires unmaterialed triangles");
    const cgltf_accessor* positions = nullptr;
    const cgltf_accessor* colors = nullptr;
    for (std::size_t i = 0; i < primitive.attributes_count; ++i) {
        const auto& attr = primitive.attributes[i];
        require(attr.index == 0, "only attribute set 0 supported");
        if (attr.type == cgltf_attribute_type_position) {
            require(!positions, "duplicate POSITION"); positions = attr.data;
        } else if (attr.type == cgltf_attribute_type_color) {
            require(!colors, "duplicate COLOR_0"); colors = attr.data;
        } else {
            require(attr.type == cgltf_attribute_type_joints || attr.type == cgltf_attribute_type_weights,
                "unsupported vertex attribute");
        }
    }
    require(positions && colors && primitive.indices, "POSITION, COLOR_0 and indices required");
    require(positions->type == cgltf_type_vec3 && positions->component_type == cgltf_component_type_r_32f
        && !positions->normalized, "POSITION must be float VEC3");
    require(colors->type == cgltf_type_vec4 && colors->component_type == cgltf_component_type_r_16u
        && colors->normalized && colors->count == positions->count, "COLOR_0 must be normalized ushort VEC4 matching POSITION");
    const auto* indices = primitive.indices;
    require(indices->type == cgltf_type_scalar && indices->component_type == cgltf_component_type_r_16u
        && !indices->normalized && indices->count > 0 && indices->count % 3 == 0, "indices must be ushort triangle triples");
    require(positions->count > 0 && positions->buffer_view && colors->buffer_view && indices->buffer_view,
        "vertex/index data missing");
    float m[16];
    cgltf_node_transform_world(mesh_node, m);
    std::vector<Vertex> vertices(positions->count);
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        float p[3], c[4];
        require(cgltf_accessor_read_float(positions, i, p, 3) && cgltf_accessor_read_float(colors, i, c, 4),
            "cannot read position/color accessor");
        require(std::all_of(p, p+3, [](float x) { return std::isfinite(x); })
            && std::all_of(c, c+4, [](float x) { return std::isfinite(x) && x >= 0 && x <= 1; }) && c[3] == 1,
            "non-finite position or invalid/non-opaque color");
        // Column-vector glTF math; do not mix with DirectX row-vector matrix storage.
        const DirectX::XMFLOAT3 transformed{m[0]*p[0]+m[4]*p[1]+m[8]*p[2]+m[12],
            m[1]*p[0]+m[5]*p[1]+m[9]*p[2]+m[13], m[2]*p[0]+m[6]*p[1]+m[10]*p[2]+m[14]};
        require(std::isfinite(transformed.x) && std::isfinite(transformed.y) && std::isfinite(transformed.z),
            "non-finite transformed position");
        vertices[i] = {transformed, {c[0], c[1], c[2]}};
    }
    result.triangles.reserve(indices->count);
    for (std::size_t i = 0; i < indices->count; ++i) {
        const auto index = cgltf_accessor_read_index(indices, i);
        require(index < vertices.size(), "triangle index out of bounds");
        result.triangles.push_back(vertices[index]);
    }
    result.mesh_name = mesh.name ? mesh.name : "";
    result.mesh_node_name = mesh_node->name ? mesh_node->name : "";
    result.source_vertex_count = vertices.size();
    return result;
}
}
