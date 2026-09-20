#pragma once
#include "hit_authorization.hpp"
#include "timing_interaction.hpp"
namespace pawapuro {
struct BallResponse {
    double contact_time_s;
    DirectX::XMFLOAT3 launch_position_m;
    float exit_speed_mps,longitudinal_angle_deg,spray_angle_deg;
    DirectX::XMFLOAT3 launch_velocity_mps;
    float temporal_transfer,spatial_transfer,energy_transfer;
};
inline float spatial_transfer(float q,const BallResponseTuning& tuning) {
    const float u=std::clamp(q,0.f,1.f);
    return 1-(1-tuning.spatial_edge_transfer)*u*u*(3-2*u);
}
// Signed world Y-Z angle measured from +Z; fixed ey topology, startup-tuned angles.
inline float vertical_contact_longitudinal_angle(float ey,const BallResponseTuning& tuning) {
    constexpr std::array<float,9> anchors{-1,-.75f,-.5f,-.25f,0,.25f,.5f,.75f,1};
    ey=std::clamp(ey,-1.f,1.f);
    for(std::size_t i=1;i<anchors.size();++i)
        if(ey<=anchors[i])return std::lerp(tuning.vertical_contact_longitudinal_degrees[i-1],
            tuning.vertical_contact_longitudinal_degrees[i],(ey-anchors[i-1])/(anchors[i]-anchors[i-1]));
    return tuning.vertical_contact_longitudinal_degrees.back();
}
// Contact topology owns eligibility; authoritative q controls trait expression.
inline float trajectory_airborne_angle(float base,float q,int trajectory,const BallResponseTuning& tuning) {
    if(base<=0||base>=90)return base;
    const float multiplier=1+(1-q)*(tuning.trajectory_airborne_slope_multiplier[trajectory-1]-1);
    return DirectX::XMConvertToDegrees(std::atan(std::tan(DirectX::XMConvertToRadians(base))*multiplier));
}
// Raw collider state is deliberately not an input to gameplay contact or response.
inline std::optional<BallResponse> ball_response(const TimingInteraction& timing,
    const HitAuthorizationDecision& aim,const BatterProfile& batter,const BallResponseTuning& tuning,
    const BallState& incoming,double release_s) {
    if(!timing.overlap||!aim.authorized)return std::nullopt;
    const float temporal=static_cast<float>(timing.efficiency),spatial=spatial_transfer(aim.q,tuning);
    // S0 candidate: timing gates contact and steers spray; spatial quality transfers energy.
    const float energy=spatial;
    const float ideal=std::lerp(tuning.ideal_exit_speed_min_mps,tuning.ideal_exit_speed_max_mps,float(batter.power)/120.f);
    const float speed=ideal*(tuning.minimum_exit_speed_factor+(1-tuning.minimum_exit_speed_factor)*std::sqrt(std::clamp(energy,0.f,1.f)));
    const float base=vertical_contact_longitudinal_angle(aim.normalized_error.y,tuning);
    const float longitudinal=trajectory_airborne_angle(base,aim.q,batter.trajectory,tuning);
    const float spray=static_cast<float>(std::clamp(-timing.offset_ms/tuning.full_spray_offset_ms,-1.,1.))*tuning.max_spray_degrees;
    const double time=std::clamp(timing.peak_s,timing.overlap->start_s,timing.overlap->end_s);
    const auto origin=sample_reference_pitch(incoming,time-release_s).position_m;
    const float theta=DirectX::XMConvertToRadians(longitudinal),azimuth=DirectX::XMConvertToRadians(spray);
    // Field center is +Z; the fixed left-handed batter pulls toward +X (first-base/right-field ray).
    const DirectX::XMFLOAT3 velocity{speed*std::abs(std::cos(theta))*std::sin(azimuth),speed*std::sin(theta),speed*std::cos(theta)*std::cos(azimuth)};
    return BallResponse{time,origin,speed,longitudinal,spray,velocity,temporal,spatial,energy};
}
// Immutable analytic path on the preview world clock; ground_s always means FIRST hit.
struct BattedBallFlight {
    BallState initial,first_ground{};
    double start_s,ground_s,rebound_settle_s,horizontal_stop_s,stop_s;
    float ground_height_m;
    double first_rebound_vy=0,rebound_ratio=0,rebound_duration=0;
    double ground_speed=0,ground_deceleration=0;
    explicit BattedBallFlight(const BallResponse& response,float radius,GroundBallResponseTuning tuning={})
        :initial{response.launch_position_m,response.launch_velocity_mps},start_s(response.contact_time_s),ground_height_m(radius) {
        const double g=-double(earth_gravity_mps2),vy=initial.velocity_mps.y,h=std::max(0.,double(initial.position_m.y)-radius);
        ground_s=start_s+(vy+std::sqrt(vy*vy+2*g*h))/g;
        rebound_settle_s=horizontal_stop_s=stop_s=ground_s;
        first_ground=sample_reference_pitch(initial,ground_s-start_s);
        first_ground.position_m.y=radius;
        rebound_ratio=tuning.rebound_vertical_ratio;
        first_rebound_vy=-double(first_ground.velocity_mps.y)*rebound_ratio;
        first_ground.velocity_mps={first_ground.velocity_mps.x*tuning.first_impact_horizontal_retention,0,
            first_ground.velocity_mps.z*tuning.first_impact_horizontal_retention};
        rebound_duration=(2*first_rebound_vy/g)/(1-rebound_ratio);
        rebound_settle_s=ground_s+rebound_duration;
        ground_speed=std::hypot(double(first_ground.velocity_mps.x),double(first_ground.velocity_mps.z));
        ground_deceleration=tuning.ground_horizontal_deceleration_mps2;
        horizontal_stop_s=ground_s+ground_speed/ground_deceleration;
        stop_s=std::max(rebound_settle_s,horizontal_stop_s);
    }
    double first_hit_s() const {return ground_s;}
    bool complete(double world_s) const {return world_s>=stop_s;}
    double rebound_launch_speed(unsigned k) const {return first_rebound_vy*std::pow(rebound_ratio,k);}
    double rebound_start_s(unsigned k) const {
        return k==0?ground_s:rebound_settle_s-rebound_duration*std::pow(rebound_ratio,k);
    }
    BallState sample(double world_s) const {
        if(world_s<ground_s)return sample_reference_pitch(initial,std::max(world_s,start_s)-start_s);
        auto result=first_ground;
        const double t=std::clamp(world_s,ground_s,horizontal_stop_s)-ground_s;
        if(ground_speed>0) {
            const double distance=ground_speed*t-.5*ground_deceleration*t*t;
            result.position_m.x+=static_cast<float>(double(first_ground.velocity_mps.x)/ground_speed*distance);
            result.position_m.z+=static_cast<float>(double(first_ground.velocity_mps.z)/ground_speed*distance);
            const double factor=world_s>=horizontal_stop_s?0:std::max(0.,ground_speed-ground_deceleration*t)/ground_speed;
            result.velocity_mps.x=static_cast<float>(first_ground.velocity_mps.x*factor);
            result.velocity_mps.z=static_cast<float>(first_ground.velocity_mps.z*factor);
        }
        if(world_s<rebound_settle_s) {
            // Invert the geometric remaining-duration series; no height/tick cutoff.
            auto k=static_cast<unsigned>(std::max(0.,std::floor(std::log((rebound_settle_s-world_s)/rebound_duration)/std::log(rebound_ratio))));
            // Correct log rounding against the same representable boundaries used by callers.
            // Exact boundaries belong to the outgoing (next) arc.
            while(k>0&&world_s<rebound_start_s(k))--k;
            while(world_s>=rebound_start_s(k+1))++k;
            const double local=world_s-rebound_start_s(k),u=rebound_launch_speed(k),g=-double(earth_gravity_mps2);
            // Roundoff protection for position only, never a bounce termination criterion.
            result.position_m.y+=static_cast<float>(std::max(0.,u*local-.5*g*local*local));
            result.velocity_mps.y=static_cast<float>(u-g*local);
        }
        return result;
    }
};
}
