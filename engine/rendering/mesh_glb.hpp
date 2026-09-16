#pragma once
#include "d3d12_view.hpp"
#include <filesystem>
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace engine {
using GlbMatrix = std::array<float,16>;
struct GlbTrs {
    DirectX::XMFLOAT3 translation{0,0,0}, scale{1,1,1};
    DirectX::XMFLOAT4 rotation{0,0,0,1};
};
struct MeshGlbNode {
    std::string name, parent_name;
    // glTF column-major, parent-composed static transform. No animation evaluation.
    GlbMatrix world{};
    GlbTrs local;
    int parent = -1;
};
enum class GlbPath { Translation, Rotation, Scale };
struct GlbChannel {
    std::size_t node;
    GlbPath path;
    std::vector<DirectX::XMFLOAT4> values;
};
struct GlbInfluence { std::array<unsigned,4> joints; std::array<float,4> weights; };
struct GlbPrimitive {
    std::vector<Vertex> triangles; // Expanded and mesh-node transformed, still glTF space.
    std::string mesh_name, mesh_node_name;
    std::size_t mesh_node = 0; // Index into the asset shared hierarchy.
    std::size_t source_vertex_count = 0;
    std::vector<Vertex> bind_vertices; // Raw accessor geometry, before mesh-node transforms.
    std::vector<std::uint16_t> indices;
    std::vector<GlbInfluence> influences;
};
struct MeshGlb {
    std::vector<GlbPrimitive> primitives;
    std::vector<MeshGlbNode> nodes;
    std::vector<std::size_t> joints, hierarchy_order;
    std::vector<GlbMatrix> inverse_binds;
    std::vector<float> times; // One shared timeline in this concrete LINEAR subset.
    std::vector<GlbChannel> channels;
    std::string skin_name, clip_name, sha256;
};

// Synchronous owned result; parser and file storage die before returning.
// Named mesh nodes, one primitive each, sharing one hierarchy/skin/clip.
// Embedded GLB, POSITION float3, opaque COLOR_0 unorm16x4,
// ushort triangle indices, local TRS, one skin and one LINEAR TRS clip.
MeshGlb read_mesh_glb(const std::filesystem::path& path);
struct GlbPose {
    std::vector<GlbTrs> local;
    std::vector<GlbMatrix> world, skin;
};
// Reusable CPU workspaces own their storage. nullopt evaluates static/bind nodes.
void evaluate_glb_pose(const MeshGlb& mesh, std::optional<float> time_s, GlbPose& pose);
void skin_glb_vertices(const GlbPrimitive& mesh, const GlbPose& pose, std::vector<Vertex>& vertices);
}
