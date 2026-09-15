#pragma once
#include "engine/rendering/static_glb.hpp"
#include "pawapuro/batting/staging.hpp"

namespace pawapuro {
struct StaticPitcher {
    std::vector<engine::Vertex> vertices;
    DirectX::XMFLOAT3 local_min{}, local_max{}, world_min{}, world_max{}, bind_grip_world{};
    std::size_t source_vertex_count = 0;
};
// Startup only. Metres, reflect glTF X once, then use staging's existing placement.
StaticPitcher load_static_pitcher(const std::filesystem::path& path, const BattingStaging& staging);
}
