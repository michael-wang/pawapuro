#include "bat_contact.hpp"
#include <cmath>
#include <cstring>
#include <stdexcept>
namespace pawapuro {
namespace {
DirectX::XMMATRIX matrix(const engine::GlbMatrix& m) {
    DirectX::XMFLOAT4X4 rows;std::memcpy(&rows,m.data(),sizeof(rows));return DirectX::XMLoadFloat4x4(&rows);
}
BatContact result(const BatterMotion& batter,const ContactSample& s,BatContactEnvelope e) {
    const auto c=s.ball.position_m,q=s.approach.closest;const double d=s.approach.distance_m;
    if(d==0) throw std::runtime_error("Contact normal undefined for coincident centerline; review initial overlap.");
    const DirectX::XMFLOAT3 n{static_cast<float>((c.x-q.x)/d),static_cast<float>((c.y-q.y)/d),static_cast<float>((c.z-q.z)/d)};
    const DirectX::XMFLOAT3 surface{q.x+n.x*e.bat_radius_m,q.y+n.y*e.bat_radius_m,q.z+n.z*e.bat_radius_m};
    // Hold this surface material point in the bat's rigid local frame. Do not
    // differentiate a sliding nearest point or a changing world-space normal.
    using namespace DirectX;
    const auto local=XMVector3TransformCoord(XMLoadFloat3(&surface),XMMatrixInverse(nullptr,matrix(s.bat.barrel_world)));
    constexpr double epsilon=.0001;engine::GlbPose scratch;
    const auto a=batter.sample_barrel(s.preview_time_s-epsilon,scratch),b=batter.sample_barrel(s.preview_time_s+epsilon,scratch);
    XMFLOAT3 velocity;XMStoreFloat3(&velocity,XMVectorScale(XMVectorSubtract(XMVector3TransformCoord(local,matrix(b.barrel_world)),XMVector3TransformCoord(local,matrix(a.barrel_world))),static_cast<float>(1/(2*epsilon))));
    return {s,s.preview_time_s*pitch_hz,e.ball_radius_m,e.bat_radius_m,n,surface,velocity,
        {s.ball.velocity_mps.x-velocity.x,s.ball.velocity_mps.y-velocity.y,s.ball.velocity_mps.z-velocity.z}};
}
}
std::optional<BatContact> first_bat_contact(const BallState& initial,double release,const BatterMotion& batter,
    BatContactEnvelope envelope,double start,double end,unsigned substeps)
{
    if((substeps!=32 && substeps!=64 && substeps!=128) || !std::isfinite(start) || !std::isfinite(end)
        || start<release || end<=start || end-start>1.0/pitch_hz+1e-12
        || !std::isfinite(envelope.ball_radius_m) || !std::isfinite(envelope.bat_radius_m)
        || envelope.ball_radius_m<=0 || envelope.bat_radius_m<=0)
        throw std::runtime_error("Invalid bounded bat contact interval/envelope.");
    const double radius=double(envelope.ball_radius_m)+envelope.bat_radius_m;
    engine::GlbPose scratch;
    auto previous=sample_contact(initial,release,batter,start,scratch);
    if(previous.approach.distance_m<=radius) return result(batter,previous,envelope);
    for(unsigned i=1;i<=substeps;++i) {
        auto next=sample_contact(initial,release,batter,start+(end-start)*i/substeps,scratch);
        if(next.approach.distance_m<=radius) {
            double low=previous.preview_time_s,high=next.preview_time_s;
            for(unsigned step=0;step<24;++step) {
                const double middle=(low+high)*.5;const auto candidate=sample_contact(initial,release,batter,middle,scratch);
                if(candidate.approach.distance_m<=radius) {high=middle;next=candidate;} else low=middle;
            }
            return result(batter,next,envelope);
        }
        previous=next;
    }
    return std::nullopt;
}
}
