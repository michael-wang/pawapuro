#pragma once
#include "engine/rendering/d3d12_view.hpp"
#include <vector>

namespace pawapuro {
std::vector<engine::Vertex> make_batting_reference();
DirectX::XMFLOAT4X4 batting_view_projection(float aspect);
}
