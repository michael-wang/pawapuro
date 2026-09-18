#pragma once
#include "hit_authorization.hpp"
#include "timing_interaction.hpp"
namespace pawapuro {
struct BallResponse {
    double contact_time_s;
    DirectX::XMFLOAT3 launch_position_m;
    float exit_speed_mps,launch_angle_deg,spray_angle_deg;
    DirectX::XMFLOAT3 launch_velocity_mps;
    float temporal_transfer,spatial_transfer,energy_transfer;
};
inline float spatial_transfer(float q,const BallResponseTuning& tuning) {
    const float u=std::clamp(q,0.f,1.f);
    return 1-(1-tuning.spatial_edge_transfer)*u*u*(3-2*u);
}
// Raw collider state is deliberately not an input to gameplay contact or response.
inline std::optional<BallResponse> ball_response(const TimingInteraction& timing,
    const HitAuthorizationDecision& aim,const BatterProfile& batter,const BallResponseTuning& tuning,
    const BallState& incoming,double release_s) {
    if(!timing.overlap||!aim.authorized)return std::nullopt;
    const float temporal=static_cast<float>(timing.efficiency),spatial=spatial_transfer(aim.q,tuning);
    const float energy=temporal*spatial;
    const float ideal=std::lerp(tuning.ideal_exit_speed_min_mps,tuning.ideal_exit_speed_max_mps,float(batter.power)/120.f);
    const float speed=ideal*(tuning.minimum_exit_speed_factor+(1-tuning.minimum_exit_speed_factor)*std::sqrt(std::clamp(energy,0.f,1.f)));
    const float launch=std::clamp(tuning.trajectory_launch_degrees.at(static_cast<std::size_t>(batter.trajectory-1))
        +tuning.vertical_aim_bias_degrees*aim.normalized_error.y,-15.f,50.f);
    const float spray=static_cast<float>(std::clamp(-timing.offset_ms/tuning.full_spray_offset_ms,-1.,1.))*tuning.max_spray_degrees;
    const double time=std::clamp(timing.peak_s,timing.overlap->start_s,timing.overlap->end_s);
    const auto origin=sample_reference_pitch(incoming,time-release_s).position_m;
    const float elevation=DirectX::XMConvertToRadians(launch),azimuth=DirectX::XMConvertToRadians(spray);
    // Field center is +Z; the fixed left-handed batter pulls toward +X (first-base/right-field ray).
    const DirectX::XMFLOAT3 velocity{speed*std::cos(elevation)*std::sin(azimuth),speed*std::sin(elevation),speed*std::cos(elevation)*std::cos(azimuth)};
    return BallResponse{time,origin,speed,launch,spray,velocity,temporal,spatial,energy};
}
// Immutable launch snapshot; sample at the preview's existing world clock, never render delta.
struct BattedBallFlight {
    BallState initial;
    double start_s,ground_s;
    float ground_height_m;
    explicit BattedBallFlight(const BallResponse& response,float radius)
        :initial{response.launch_position_m,response.launch_velocity_mps},start_s(response.contact_time_s),ground_height_m(radius) {
        const double g=-double(earth_gravity_mps2),vy=initial.velocity_mps.y,h=std::max(0.,double(initial.position_m.y)-radius);
        // Positive root is the descending crossing, also for initially downward grounders.
        ground_s=start_s+(vy+std::sqrt(vy*vy+2*g*h))/g;
    }
    bool complete(double world_s) const {return world_s>=ground_s;}
    BallState sample(double world_s) const {
        auto result=sample_reference_pitch(initial,std::clamp(world_s,start_s,ground_s)-start_s);
        if(complete(world_s)){result.position_m.y=ground_height_m;result.velocity_mps={};}
        return result;
    }
};
}
