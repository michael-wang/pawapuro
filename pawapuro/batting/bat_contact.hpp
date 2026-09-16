#pragma once
#include "contact_probe.hpp"
#include <optional>
namespace pawapuro {
// Pawapuro event, recorded once by BattingPreview; no trajectory response.
struct BatContact {
    ContactSample sample;
    double fractional_tick=0;
    float ball_radius_m=0,bat_radius_m=0;
    DirectX::XMFLOAT3 normal{},surface_point{},bat_point_velocity{},relative_velocity{};
};
// Fixed substeps bracket first entry, then 24 bisections retain the inside end.
// Contract is the current smooth accepted clip/window, not arbitrary-motion CCD.
std::optional<BatContact> first_bat_contact(const BallState& initial,double release_time_s,
    const BatterMotion& batter,BatContactEnvelope envelope,double interval_start_s,double interval_end_s,
    unsigned substeps=64);
}
