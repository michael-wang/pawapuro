#pragma once
#include "engine/rendering/d3d12_view.hpp"
#include <vector>
#include "staging.hpp"

namespace pawapuro {
struct BattingReference {
    std::vector<engine::Vertex> vertices;
    unsigned ball_vertex_start = 0;
};
BattingReference make_batting_reference(const BattingStaging& staging);
DirectX::XMFLOAT4X4 batting_view_projection(const BattingStaging& staging, float aspect);
}
