#pragma once
#include "manual_swing.hpp"
#include "player_aim.hpp"

namespace pawapuro {
inline constexpr std::size_t contact_review_marker_vertex_count=6;
inline constexpr std::size_t contact_review_vertex_count=PlayerAim::vertex_count+contact_review_marker_vertex_count;
// Presentation borrows the existing attempt only for this append; no second snapshot or gameplay owner.
inline void append_contact_review(std::vector<engine::Vertex>& vertices,const PlayerAim& aim,const ManualSwingPreview& preview) {
    const auto* contact=preview.gameplay_result()==GameplayResult::Contact?preview.latest():nullptr;
    aim.append_triangles_at(vertices,contact?contact->command.aim_center:aim.center());
    if(!contact) {
        for(std::size_t i=0;i<contact_review_marker_vertex_count;++i)vertices.push_back({{}, {}});
        return;
    }
    const auto p=contact->authorization.pitch_point;
    // Small diamond inside the existing black core when centered. Only Z is presentation-offset.
    constexpr float radius=.012f;
    const float z=preview.tuning.strike_zone_plane_z()-.006f;
    const DirectX::XMFLOAT3 left{p.x-radius,p.y,z},top{p.x,p.y+radius,z},right{p.x+radius,p.y,z},bottom{p.x,p.y-radius,z};
    for(const auto position:{left,top,right,left,right,bottom})vertices.push_back({position,{.35f,1.f,1.f}});
}
}
