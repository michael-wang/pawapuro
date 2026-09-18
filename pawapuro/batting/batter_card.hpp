#pragma once
#include "batter_profile.hpp"
#include "engine/rendering/d3d12_view.hpp"
#include <array>
#include <string>
#include <vector>
namespace pawapuro {
// App-owned immutable startup presentation. No per-frame formatting or rasterization.
struct BatterCard {
    BatterCard(const BatterProfile& profile,unsigned pixel_width,unsigned pixel_height);
    std::array<std::wstring,4> lines;
    std::vector<engine::Vertex> triangles;
    unsigned vertex_count() const { return static_cast<unsigned>(triangles.size()); }
    void append(std::vector<engine::Vertex>& target) const {
        target.insert(target.end(),triangles.begin(),triangles.end());
    }
};
}
