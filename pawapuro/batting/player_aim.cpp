#include "player_aim.hpp"
#include "hit_authorization.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace pawapuro {
PlayerAim::PlayerAim(const BattingStaging& s):tuning(s.player_aim),authorization(normal_authorization_region(s)),half_width(s.strike_zone_width_m/2),
    bottom(s.strike_zone_bottom_m),top(s.strike_zone_top_m),plane_z(s.strike_zone_plane_z()),
    visual_z(plane_z-0.002f) // Fixed camera is on -Z; rendering only, never aim truth.
{
    if (!std::isfinite(half_width)||half_width<=0||!std::isfinite(bottom)||!std::isfinite(top)||top<=bottom||
        !std::isfinite(authorization.normal_radius_x_m)||authorization.normal_radius_x_m<=0||authorization.normal_radius_x_m>=2*half_width||
        !std::isfinite(authorization.normal_radius_y_m)||authorization.normal_radius_y_m<=0||authorization.normal_radius_y_m>=top-bottom||
        !std::isfinite(tuning.cursor_speed_mps)||tuning.cursor_speed_mps<=0)
        throw std::runtime_error("Invalid PlayerAim tuning/zone.");
    recenter();
}
void PlayerAim::recenter() { center_m={0,(bottom+top)/2}; }
void PlayerAim::move(float x,float y,double elapsed_s) {
    if (!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(elapsed_s)||elapsed_s<=0) return;
    const float length=std::hypot(x,y);
    if(length>1) {x/=length;y/=length;}
    // Bound debugger/frame stalls; no debt or second simulation clock.
    const float distance=tuning.cursor_speed_mps*static_cast<float>(std::min(elapsed_s,0.05));
    center_m.x=std::clamp(center_m.x+x*distance,-half_width,half_width);
    center_m.y=std::clamp(center_m.y+y*distance,bottom,top);
}
AimDiagnostic PlayerAim::diagnostic(DirectX::XMFLOAT3 p) const {
    const auto a=authorize_normal_hit(center_m,{p.x,p.y},authorization);
    return {a.error.x,a.error.y,a.normalized_error.x,a.normalized_error.y,a.q};
}
void PlayerAim::append_triangles(std::vector<engine::Vertex>& v) const {
    append_triangles_at(v,center_m);
}
void PlayerAim::append_triangles_at(std::vector<engine::Vertex>& v,DirectX::XMFLOAT2 center) const {
    const auto point=[&](float x,float y,float z) {return DirectX::XMFLOAT3{center.x+x,center.y+y,z};};
    const auto quad=[&](DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b,DirectX::XMFLOAT3 c,DirectX::XMFLOAT3 d,DirectX::XMFLOAT3 color) {
        for(auto p:{a,b,c,a,c,d}) v.push_back({p,color});
    };
    const float rx=authorization.normal_radius_x_m,ry=authorization.normal_radius_y_m;
    constexpr float stroke=0.008f;
    for(unsigned i=0;i<32;++i) {
        const float a=DirectX::XM_2PI*static_cast<float>(i)/32,b=DirectX::XM_2PI*static_cast<float>(i+1)/32;
        const auto ring=[&](float angle,float scale) {return point(rx*scale*std::cos(angle),ry*scale*std::sin(angle),visual_z);};
        quad(ring(a,0.94f),ring(b,0.94f),ring(b,1),ring(a,1),{1,0.88f,0.03f});
    }
    const DirectX::XMFLOAT3 black{0.015f,0.015f,0.015f};
    quad(point(-rx,-stroke/2,visual_z-.001f),point(-rx,stroke/2,visual_z-.001f),point(rx,stroke/2,visual_z-.001f),point(rx,-stroke/2,visual_z-.001f),black);
    quad(point(-stroke/2,-ry,visual_z-.001f),point(-stroke/2,ry,visual_z-.001f),point(stroke/2,ry,visual_z-.001f),point(stroke/2,-ry,visual_z-.001f),black);
    constexpr float core=.018f;
    quad(point(-core,0,visual_z-.002f),point(0,core,visual_z-.002f),point(core,0,visual_z-.002f),point(0,-core,visual_z-.002f),black);
}
}
