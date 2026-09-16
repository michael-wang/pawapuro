#include "contact_probe.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace pawapuro {
namespace {
DirectX::XMFLOAT3 along(BatBarrelSample b,double u) {
    return {static_cast<float>(b.barrel.x+(b.tip.x-b.barrel.x)*u),
        static_cast<float>(b.barrel.y+(b.tip.y-b.barrel.y)*u),
        static_cast<float>(b.barrel.z+(b.tip.z-b.barrel.z)*u)};
}
}
BarrelApproach closest_barrel_point(DirectX::XMFLOAT3 c,BatBarrelSample b)
{
    const double x=double(b.tip.x)-b.barrel.x,y=double(b.tip.y)-b.barrel.y,z=double(b.tip.z)-b.barrel.z;
    const double length2=x*x+y*y+z*z;
    // A collapsed segment reduces to its barrel endpoint; no collision envelope.
    const double u=length2>0?std::clamp(((double(c.x)-b.barrel.x)*x+(double(c.y)-b.barrel.y)*y+(double(c.z)-b.barrel.z)*z)/length2,0.0,1.0):0;
    const auto q=along(b,u);
    const double dx=double(c.x)-q.x,dy=double(c.y)-q.y,dz=double(c.z)-q.z;
    return {q,u,std::sqrt(dx*dx+dy*dy+dz*dz)};
}
ContactSample sample_contact(const BallState& initial,double release,const BatterMotion& batter,double time,engine::GlbPose& scratch)
{
    const auto ball=sample_reference_pitch(initial,time-release);const auto bat=batter.sample_barrel(time,scratch);
    return {time,ball,bat,closest_barrel_point(ball.position_m,bat)};
}
ContactProbe probe_contact(const BallState& initial,double release,const BatterMotion& batter,unsigned refinement)
{
    if (refinement!=32 && refinement!=64 && refinement!=128) throw std::runtime_error("Contact study supports fixed 32/64/128 refinements.");
    const double marker=static_cast<double>(batter.contact_area_tick)/pitch_hz;
    const double low=marker-8.0/pitch_hz,high=marker+8.0/pitch_hz;
    if (low<release || high>static_cast<double>(batter.end_tick)/pitch_hz) throw std::runtime_error("Contact window outside flight/clip.");
    engine::GlbPose scratch;
    auto best=sample_contact(initial,release,batter,low,scratch);
    constexpr unsigned coarse=16*16;
    const double step=(high-low)/coarse;
    unsigned index=0;
    for (unsigned i=1;i<=coarse;++i) {
        const auto candidate=sample_contact(initial,release,batter,low+i*step,scratch);
        if (candidate.approach.distance_m<best.approach.distance_m) {best=candidate;index=i;}
    }
    const double a=low+(index?index-1:0)*step,b=low+std::min(index+1,coarse)*step;
    for (unsigned i=0;i<=refinement;++i) {
        const auto candidate=sample_contact(initial,release,batter,a+(b-a)*i/refinement,scratch);
        if (candidate.approach.distance_m<best.approach.distance_m) best=candidate;
    }
    return {best,low,high,refinement,best.preview_time_s==low || best.preview_time_s==high};
}
ContactVelocity contact_velocity(const BatterMotion& batter,const ContactSample& sample,double epsilon)
{
    if (!std::isfinite(epsilon) || epsilon<=0) throw std::runtime_error("Contact velocity epsilon must be positive and finite.");
    engine::GlbPose scratch;
    const auto a=along(batter.sample_barrel(sample.preview_time_s-epsilon,scratch),sample.approach.u);
    const auto b=along(batter.sample_barrel(sample.preview_time_s+epsilon,scratch),sample.approach.u);
    const DirectX::XMFLOAT3 v{static_cast<float>((double(b.x)-a.x)/(2*epsilon)),static_cast<float>((double(b.y)-a.y)/(2*epsilon)),static_cast<float>((double(b.z)-a.z)/(2*epsilon))};
    return {v,{sample.ball.velocity_mps.x-v.x,sample.ball.velocity_mps.y-v.y,sample.ball.velocity_mps.z-v.z}};
}
}
