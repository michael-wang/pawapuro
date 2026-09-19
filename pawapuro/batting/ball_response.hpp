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
    const float energy=temporal*spatial;
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
    BallState initial;
    double start_s,ground_s,second_ground_s,stop_s;
    float ground_height_m;
    BallState rebound{},roll{};
    double roll_speed=0,roll_deceleration=0;
    explicit BattedBallFlight(const BallResponse& response,float radius,GroundBallResponseTuning tuning={})
        :initial{response.launch_position_m,response.launch_velocity_mps},start_s(response.contact_time_s),ground_height_m(radius) {
        const double g=-double(earth_gravity_mps2),vy=initial.velocity_mps.y,h=std::max(0.,double(initial.position_m.y)-radius);
        ground_s=start_s+(vy+std::sqrt(vy*vy+2*g*h))/g;
        second_ground_s=stop_s=ground_s;
        if(vy>=0)return; // Accepted airborne-family first-ground hold stays identical.
        rebound=sample_reference_pitch(initial,ground_s-start_s);
        rebound.position_m.y=radius;
        rebound.velocity_mps={rebound.velocity_mps.x*tuning.impact_horizontal_retention,
            -rebound.velocity_mps.y*tuning.rebound_vertical_ratio,rebound.velocity_mps.z*tuning.impact_horizontal_retention};
        second_ground_s=ground_s+2*double(rebound.velocity_mps.y)/g;
        roll=sample_reference_pitch(rebound,second_ground_s-ground_s);
        roll.position_m.y=radius;
        roll.velocity_mps={roll.velocity_mps.x*tuning.impact_horizontal_retention,0,roll.velocity_mps.z*tuning.impact_horizontal_retention};
        roll_speed=std::hypot(double(roll.velocity_mps.x),double(roll.velocity_mps.z));
        roll_deceleration=tuning.roll_deceleration_mps2;
        stop_s=second_ground_s+roll_speed/roll_deceleration;
    }
    double first_hit_s() const {return ground_s;}
    bool complete(double world_s) const {return world_s>=stop_s;}
    BallState sample(double world_s) const {
        if(initial.velocity_mps.y>=0) {
            auto result=sample_reference_pitch(initial,std::clamp(world_s,start_s,ground_s)-start_s);
            if(complete(world_s)){result.position_m.y=ground_height_m;result.velocity_mps={};}
            return result;
        }
        if(world_s<ground_s)return sample_reference_pitch(initial,std::max(world_s,start_s)-start_s);
        if(world_s<second_ground_s)return sample_reference_pitch(rebound,world_s-ground_s);
        auto result=roll;
        const double t=std::clamp(world_s,second_ground_s,stop_s)-second_ground_s;
        if(roll_speed>0) {
            const double distance=roll_speed*t-.5*roll_deceleration*t*t;
            result.position_m.x+=static_cast<float>(double(roll.velocity_mps.x)/roll_speed*distance);
            result.position_m.z+=static_cast<float>(double(roll.velocity_mps.z)/roll_speed*distance);
            const double factor=std::max(0.,roll_speed-roll_deceleration*t)/roll_speed;
            result.velocity_mps.x=static_cast<float>(roll.velocity_mps.x*factor);
            result.velocity_mps.z=static_cast<float>(roll.velocity_mps.z*factor);
        }
        if(complete(world_s))result.velocity_mps={};
        return result;
    }
};
}
