#pragma once
#include "staging.hpp"

namespace pawapuro {
struct HitAuthorizationDecision {
    bool authorized;
    DirectX::XMFLOAT2 pitch_point, error, normalized_error;
    float q;
};
// Normal spatial permission only. Inputs use the authoritative evaluation-plane X/Y in metres.
inline HitAuthorizationDecision authorize_normal_hit(DirectX::XMFLOAT2 aim,
    DirectX::XMFLOAT2 pitch, const HitAuthorizationTuning& tuning) {
    const DirectX::XMFLOAT2 error{pitch.x-aim.x,pitch.y-aim.y};
    const DirectX::XMFLOAT2 normalized{error.x/tuning.normal_radius_x_m,error.y/tuning.normal_radius_y_m};
    const float q=normalized.x*normalized.x+normalized.y*normalized.y;
    return {q<=1,pitch,error,normalized,q};
}
}
