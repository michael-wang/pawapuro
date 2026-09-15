#include "static_pitcher.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace pawapuro {
StaticPitcher load_static_pitcher(const std::filesystem::path& path, const BattingStaging& staging)
{
    try {
        auto mesh = engine::read_static_glb(path);
        if (mesh.mesh_name != "PitcherMesh" || mesh.mesh_node_name != "PitcherMesh")
            throw std::runtime_error("expected PitcherMesh mesh/node");
        const engine::StaticGlbNode* grip = nullptr;
        for (const auto& node : mesh.nodes) if (node.name == "grip") {
            if (grip) throw std::runtime_error("ambiguous grip node");
            grip = &node;
        }
        if (!grip || grip->parent_name != "hand_R") throw std::runtime_error("expected grip under hand_R");
        StaticPitcher out;
        out.source_vertex_count = mesh.source_vertex_count;
        out.local_min = {1e9f, 1e9f, 1e9f}; out.local_max = {-1e9f, -1e9f, -1e9f};
        const auto placement = staging.pitcher_blockout_position_m;
        if (!std::isfinite(placement.x) || !std::isfinite(placement.y) || !std::isfinite(placement.z))
            throw std::runtime_error("non-finite staging pitcher placement");
        const auto to_world = [&](DirectX::XMFLOAT3 p) -> DirectX::XMFLOAT3 {
            return {p.x + placement.x, p.y + placement.y, p.z + placement.z};
        };
        for (auto& vertex : mesh.triangles) {
            auto& p = vertex.position;
            p.x = -p.x; // GLB metres already include authoring proportion scale; never multiply height_m.
            out.local_min = {std::min(out.local_min.x,p.x), std::min(out.local_min.y,p.y), std::min(out.local_min.z,p.z)};
            out.local_max = {std::max(out.local_max.x,p.x), std::max(out.local_max.y,p.y), std::max(out.local_max.z,p.z)};
            p = to_world(p);
        }
        // One reflection reverses handedness. Correct each triangle once, even with culling currently off.
        for (std::size_t i = 0; i < mesh.triangles.size(); i += 3) std::swap(mesh.triangles[i+1], mesh.triangles[i+2]);
        out.world_min = to_world(out.local_min); out.world_max = to_world(out.local_max);
        out.bind_grip_world = to_world({-grip->world[12], grip->world[13], grip->world[14]});
        out.vertices = std::move(mesh.triangles);
        std::fprintf(stderr, "Static pitcher: owner=pawapuro/batting/pitcher/static_pitcher.cpp source=%s "
            "pose=bind scale=1 triangles=%zu expanded_vertices=%zu source_vertices=%zu\n"
            "Pitcher local bounds=[%.6f %.6f %.6f]..[%.6f %.6f %.6f] world bounds=[%.6f %.6f %.6f]..[%.6f %.6f %.6f]\n"
            "Pitcher bind grip game/world=[%.6f %.6f %.6f]; animation/skin/release evaluation=off\n",
            path.string().c_str(), out.vertices.size()/3, out.vertices.size(), out.source_vertex_count,
            out.local_min.x,out.local_min.y,out.local_min.z,out.local_max.x,out.local_max.y,out.local_max.z,
            out.world_min.x,out.world_min.y,out.world_min.z,out.world_max.x,out.world_max.y,out.world_max.z,
            out.bind_grip_world.x,out.bind_grip_world.y,out.bind_grip_world.z);
        return out;
    } catch (const std::exception& error) {
        throw std::runtime_error("Pawapuro static pitcher owner=pawapuro/batting/pitcher/static_pitcher.cpp source='"
            + path.string() + "': " + error.what());
    }
}
}
