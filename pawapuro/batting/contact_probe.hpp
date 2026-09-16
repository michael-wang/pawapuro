#pragma once
#include "batter/batter_motion.hpp"
#include "reference_pitch.hpp"

namespace pawapuro {
// Diagnostic centerline geometry only: no radii, hit rule or response.
struct BarrelApproach {
    DirectX::XMFLOAT3 closest;
    double u=0,distance_m=0;
};
BarrelApproach closest_barrel_point(DirectX::XMFLOAT3 ball,BatBarrelSample bat);
struct ContactSample {
    double preview_time_s=0;
    BallState ball;
    BatBarrelSample bat;
    BarrelApproach approach;
};
ContactSample sample_contact(const BallState& initial,double release_time_s,const BatterMotion& batter,
    double preview_time_s,engine::GlbPose& scratch,double diagnostic_phase_offset_s=0);
struct ContactProbe {
    ContactSample minimum;
    double window_start_s=0,window_end_s=0;
    unsigned refinement=0;
    bool boundary_minimum=false;
};
// One bounded marker +/- 8-tick window. Fixed 16/tick coarse grid, then a
// fixed subdivision of the two coarse cells adjacent to the best sample.
ContactProbe probe_contact(const BallState& initial,double release_time_s,const BatterMotion& batter,unsigned refinement,double diagnostic_phase_offset_s=0);
struct ContactVelocity {
    DirectX::XMFLOAT3 bat_point,relative;
};
// Material point at the minimum's fixed u, not the sliding closest-point locus.
ContactVelocity contact_velocity(const BatterMotion& batter,const ContactSample& minimum,double epsilon_s);
}
