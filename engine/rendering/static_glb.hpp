#pragma once
#include "d3d12_view.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace engine {
struct StaticGlbNode {
    std::string name, parent_name;
    // glTF column-major, parent-composed static transform. No animation evaluation.
    std::array<float, 16> world{};
};
struct StaticGlb {
    std::vector<Vertex> triangles; // Expanded and mesh-node transformed, still glTF space.
    std::vector<StaticGlbNode> nodes;
    std::string mesh_name, mesh_node_name;
    std::size_t source_vertex_count = 0;
};

// Synchronous owned result; parser and file storage die before returning.
// One embedded GLB mesh/primitive, POSITION float3, opaque COLOR_0 unorm16x4,
// ushort triangle indices, static nodes. Skin/animation are parsed but not evaluated.
StaticGlb read_static_glb(const std::filesystem::path& path);
}
