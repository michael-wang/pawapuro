#include "mesh_glb.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <numeric>
#include <bcrypt.h>

// Keep third-party implementation warnings out of the project's /W4 /WX code.
#pragma warning(push, 0)
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#pragma warning(pop)

namespace engine {
MeshGlb read_mesh_glb(const std::filesystem::path& path)
{
    const auto require = [&](bool ok, const char* reason) {
        if (!ok) throw std::runtime_error("Engine mesh GLB '" + path.string() + "': " + reason);
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

    MeshGlb result;
    std::array<unsigned char,32> digest{};
    require(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,reinterpret_cast<PUCHAR>(bytes.data()),
        static_cast<ULONG>(bytes.size()),digest.data(),static_cast<ULONG>(digest.size()))>=0,"SHA256 failed");
    for (const auto b:digest) { result.sha256 += "0123456789abcdef"[b>>4]; result.sha256 += "0123456789abcdef"[b&15]; }
    std::vector<std::size_t> depths;
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
        MeshGlbNode out;
        require(!node.has_matrix,"only node TRS supported");
        out.parent=node.parent ? static_cast<int>(node.parent-data->nodes) : -1;
        if (node.has_translation) out.local.translation={node.translation[0],node.translation[1],node.translation[2]};
        if (node.has_rotation) out.local.rotation={node.rotation[0],node.rotation[1],node.rotation[2],node.rotation[3]};
        if (node.has_scale) out.local.scale={node.scale[0],node.scale[1],node.scale[2]};
        const auto q=out.local.rotation;
        require(std::abs(q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w-1)<2e-5f,"non-unit node quaternion");
        require(out.local.scale.x>0 && out.local.scale.y>0 && out.local.scale.z>0,"non-positive node scale");
        depths.push_back(depth);
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
    result.hierarchy_order.resize(result.nodes.size());
    std::iota(result.hierarchy_order.begin(),result.hierarchy_order.end(),std::size_t{0});
    std::stable_sort(result.hierarchy_order.begin(),result.hierarchy_order.end(),[&](auto a,auto b){return depths[a]<depths[b];});
    require(data->skins_count==1 && mesh_node->skin==&data->skins[0],"requires one skin on the mesh node");
    const auto& skin=data->skins[0];
    const auto* ibm=skin.inverse_bind_matrices;
    require(ibm && ibm->type==cgltf_type_mat4 && ibm->component_type==cgltf_component_type_r_32f
        && !ibm->normalized && ibm->count==skin.joints_count && skin.joints_count>0,"inverse binds must be float MAT4 per joint");
    result.skin_name=skin.name ? skin.name : "";
    for (std::size_t i=0;i<skin.joints_count;++i) {
        const auto joint=static_cast<std::size_t>(skin.joints[i]-data->nodes);
        require(std::find(result.joints.begin(),result.joints.end(),joint)==result.joints.end(),"duplicate skin joint");
        result.joints.push_back(joint);
        GlbMatrix matrix;
        require(cgltf_accessor_read_float(ibm,i,matrix.data(),16),"cannot read inverse bind");
        require(std::all_of(matrix.begin(),matrix.end(),[](float v){return std::isfinite(v);})
            && matrix[3]==0 && matrix[7]==0 && matrix[11]==0 && matrix[15]==1,"invalid inverse bind matrix");
        result.inverse_binds.push_back(matrix);
    }
    auto& mesh = data->meshes[0];
    auto& primitive = mesh.primitives[0];
    require(mesh.weights_count == 0 && primitive.targets_count == 0 && !primitive.has_draco_mesh_compression,
        "morph targets/Draco unsupported");
    require(primitive.type == cgltf_primitive_type_triangles && primitive.material == nullptr,
        "requires unmaterialed triangles");
    const cgltf_accessor* positions = nullptr;
    const cgltf_accessor* colors = nullptr;
    const cgltf_accessor* joints = nullptr;
    const cgltf_accessor* weights = nullptr;
    for (std::size_t i = 0; i < primitive.attributes_count; ++i) {
        const auto& attr = primitive.attributes[i];
        require(attr.index == 0, "only attribute set 0 supported");
        if (attr.type == cgltf_attribute_type_position) {
            require(!positions, "duplicate POSITION"); positions = attr.data;
        } else if (attr.type == cgltf_attribute_type_color) {
            require(!colors, "duplicate COLOR_0"); colors = attr.data;
        } else {
            if (attr.type==cgltf_attribute_type_joints) { require(!joints,"duplicate JOINTS_0"); joints=attr.data; }
            else if (attr.type==cgltf_attribute_type_weights) { require(!weights,"duplicate WEIGHTS_0"); weights=attr.data; }
            else require(false,"unsupported vertex attribute");
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
    require(joints && weights && joints->type==cgltf_type_vec4 && joints->component_type==cgltf_component_type_r_8u
        && !joints->normalized && joints->count==positions->count,"JOINTS_0 must be ubyte VEC4 matching POSITION");
    require(weights->type==cgltf_type_vec4 && weights->component_type==cgltf_component_type_r_32f
        && !weights->normalized && weights->count==positions->count,"WEIGHTS_0 must be float VEC4 matching POSITION");
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
        result.bind_vertices.push_back({{p[0],p[1],p[2]},{c[0],c[1],c[2]}});
        GlbInfluence influence{};
        require(cgltf_accessor_read_uint(joints,i,influence.joints.data(),4)
            && cgltf_accessor_read_float(weights,i,influence.weights.data(),4),"cannot read influences");
        float sum=0; unsigned nonzero=0;
        for (unsigned j=0;j<4;++j) {
            const float w=influence.weights[j];
            require(influence.joints[j]<skin.joints_count && std::isfinite(w) && w>=0 && w<=1,"invalid joint index/weight");
            if (w>0) {
                ++nonzero;
                for (unsigned k=0;k<j;++k) require(influence.weights[k]==0 || influence.joints[k]!=influence.joints[j],"duplicate weighted joint");
            }
            sum+=w;
        }
        require(std::abs(sum-1)<2e-5f && nonzero<=2,"weight sum/nonzero influence count outside sample contract");
        result.influences.push_back(influence);
    }
    result.triangles.reserve(indices->count);
    for (std::size_t i = 0; i < indices->count; ++i) {
        const auto index = cgltf_accessor_read_index(indices, i);
        require(index < vertices.size(), "triangle index out of bounds");
        result.triangles.push_back(vertices[index]);
        result.indices.push_back(static_cast<std::uint16_t>(index));
    }
    result.mesh_name = mesh.name ? mesh.name : "";
    result.mesh_node_name = mesh_node->name ? mesh_node->name : "";
    result.source_vertex_count = vertices.size();
    require(data->animations_count==1,"requires one animation clip");
    const auto& animation=data->animations[0];
    require(animation.channels_count>0 && animation.samplers_count==animation.channels_count,"requires one sampler per channel");
    result.clip_name=animation.name ? animation.name : "";
    const auto* times=animation.samplers[0].input;
    require(times && times->type==cgltf_type_scalar && times->component_type==cgltf_component_type_r_32f
        && !times->normalized && times->count>=2,"timeline must be float SCALAR");
    for (std::size_t i=0;i<times->count;++i) {
        float t;
        require(cgltf_accessor_read_float(times,i,&t,1) && std::isfinite(t) && t>=0
            && (i==0 || t>result.times.back()),"animation times must be finite and strictly increasing");
        result.times.push_back(t);
    }
    require(result.times.front()==0,"clip must start at zero");
    std::vector<std::array<bool,3>> seen(result.nodes.size());
    for (std::size_t i=0;i<animation.channels_count;++i) {
        const auto& channel=animation.channels[i]; const auto& sampler=*channel.sampler;
        require(channel.target_node && sampler.interpolation==cgltf_interpolation_type_linear && sampler.input==times,
            "requires LINEAR channels on a shared timeline");
        const auto node=static_cast<std::size_t>(channel.target_node-data->nodes);
        GlbPath channel_path;
        if (channel.target_path==cgltf_animation_path_type_translation) channel_path=GlbPath::Translation;
        else if (channel.target_path==cgltf_animation_path_type_rotation) channel_path=GlbPath::Rotation;
        else { require(channel.target_path==cgltf_animation_path_type_scale,"only TRS channels supported"); channel_path=GlbPath::Scale; }
        require(!seen[node][static_cast<unsigned>(channel_path)],"duplicate node/channel_path channel");
        seen[node][static_cast<unsigned>(channel_path)]=true;
        const auto* output=sampler.output;
        const bool rotation=channel_path==GlbPath::Rotation;
        require(output && output->type==(rotation ? cgltf_type_vec4 : cgltf_type_vec3)
            && output->component_type==cgltf_component_type_r_32f && !output->normalized && output->count==times->count,
            "TRS output must match float VEC3/VEC4 and timeline count");
        GlbChannel out{node,channel_path,{}};
        for (std::size_t j=0;j<times->count;++j) {
            float v[4]{};
            require(cgltf_accessor_read_float(output,j,v,rotation ? 4 : 3)
                && std::all_of(v,v+4,[](float x){return std::isfinite(x);}),"non-finite channel value");
            if (rotation) require(std::abs(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]+v[3]*v[3]-1)<2e-5f,"non-unit animation quaternion");
            if (channel_path==GlbPath::Scale) require(v[0]>0 && v[1]>0 && v[2]>0,"non-positive animation scale");
            out.values.push_back({v[0],v[1],v[2],v[3]});
        }
        result.channels.push_back(std::move(out));
    }
    return result;
}
}
