#pragma once
#include "engine/rendering/d3d12_view.hpp"
#include <vector>
#include "staging.hpp"

namespace pawapuro {
struct BattingReference {
    std::vector<engine::Vertex> vertices;
    unsigned ball_vertex_start = 0;
    unsigned overlay_vertex_start = 0;
    DirectX::XMFLOAT2 zone_min_ndc{}, zone_max_ndc{}, prediction_ndc{};
};
BattingReference make_batting_reference(const BattingStaging& staging,
    DirectX::XMFLOAT3 predicted_position, float aspect);
DirectX::XMFLOAT2 project_batting_point(const BattingStaging& staging, DirectX::XMFLOAT3 point, float aspect);
DirectX::XMFLOAT4X4 batting_view_projection(const BattingStaging& staging, float aspect);
}
