#pragma once
#include "engine/rendering/mesh_glb.hpp"
#include "pawapuro/batting/staging.hpp"

namespace pawapuro {
struct StaticBatter {
    std::vector<engine::Vertex> vertices;
    DirectX::XMFLOAT3 local_min{}, local_max{}, world_min{}, world_max{};
    DirectX::XMFLOAT3 grip_world{}, barrel_world{}, tip_world{};
};
// Startup-owned triangles. Parser/hierarchy storage dies here; scene copies before GPU upload.
// Bind/rest only, metres, one X reflection and staging placement; never sample swing_L.
StaticBatter load_static_batter(const std::filesystem::path& path, const BattingStaging& staging);
}
