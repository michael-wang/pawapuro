#pragma once
#include "manual_swing.hpp"
#include <cmath>
namespace pawapuro {
// Review aid only: current XZ projection, never a landing prediction or collider.
inline constexpr unsigned ground_projection_segments=32;
inline constexpr unsigned ground_projection_vertex_count=3*ground_projection_segments;
inline constexpr float ground_projection_radius_m=.35f;
inline constexpr float ground_projection_y_m=.02f;
inline constexpr DirectX::XMFLOAT3 ground_projection_color{1,.1f,1};
inline void append_ground_projection(std::vector<engine::Vertex>& target,const ManualSwingPreview& preview) {
    if(!preview.flight){target.insert(target.end(),ground_projection_vertex_count,{});return;}
    auto center=preview.flight->sample(double(preview.tick)/pitch_hz).position_m;
    center.y=ground_projection_y_m;
    const auto edge=[&](unsigned i){
        const float angle=6.283185307179586f*float(i)/ground_projection_segments;
        return DirectX::XMFLOAT3{center.x+ground_projection_radius_m*std::cos(angle),center.y,
            center.z+ground_projection_radius_m*std::sin(angle)};
    };
    for(unsigned i=0;i<ground_projection_segments;++i)
        target.insert(target.end(),{{center,ground_projection_color},{edge(i),ground_projection_color},{edge(i+1),ground_projection_color}});
}
}
