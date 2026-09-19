#pragma once
#include "staging.hpp"
#include <algorithm>
#include <cmath>

namespace pawapuro {
// Contact75 anchors the accepted Normal ellipse; presentation and permission share this value.
inline HitAuthorizationTuning normal_authorization_region(const BattingStaging& s) {
    const float scale=.5f+float(s.batter_profile.contact)/150.f;
    return {s.hit_authorization.normal_radius_x_m*scale,s.hit_authorization.normal_radius_y_m*scale};
}
struct HitAuthorizationDecision {
    bool authorized;
    DirectX::XMFLOAT2 pitch_point, error, normalized_error;
    float q;
};
// Outside an ellipse, the closest point has coordinates a^2*x/(lambda+a^2),
// b^2*y/(lambda+b^2). Its ellipse equation decreases strictly for lambda >= 0.
// Solve in double precision with a fixed iteration count; no gameplay epsilon.
inline bool gameplay_ball_overlaps_reticle(DirectX::XMFLOAT2 error,
    const HitAuthorizationTuning& ellipse,const GameplayBallTuning& ball) {
    const double x=std::abs(double(error.x)),y=std::abs(double(error.y));
    const double a=ellipse.normal_radius_x_m,b=ellipse.normal_radius_y_m,r=ball.radius_m;
    // Axis reaches share the float addition used by normalized intent, including tangency.
    if(y==0)return x<=ellipse.normal_radius_x_m+ball.radius_m;
    if(x==0)return y<=ellipse.normal_radius_y_m+ball.radius_m;
    if(x*x/(a*a)+y*y/(b*b)<=1)return true;
    double low=0,high=std::hypot(a*x,b*y);
    for(unsigned i=0;i<80;++i) {
        const double lambda=(low+high)/2;
        const double ex=a*x/(lambda+a*a),ey=b*y/(lambda+b*b);
        if(ex*ex+ey*ey>1)low=lambda;else high=lambda;
    }
    const double lambda=(low+high)/2;
    const double dx=x-a*a*x/(lambda+a*a),dy=y-b*b*y/(lambda+b*b);
    return dx*dx+dy*dy<=r*r;
}
// Authorization is shape overlap; q is independent normalized spatial quality.
inline HitAuthorizationDecision authorize_normal_hit(DirectX::XMFLOAT2 aim,
    DirectX::XMFLOAT2 pitch, const HitAuthorizationTuning& tuning,const GameplayBallTuning& ball) {
    const DirectX::XMFLOAT2 error{pitch.x-aim.x,pitch.y-aim.y};
    const DirectX::XMFLOAT2 normalized{error.x/(tuning.normal_radius_x_m+ball.radius_m),error.y/(tuning.normal_radius_y_m+ball.radius_m)};
    const float q=std::clamp(normalized.x*normalized.x+normalized.y*normalized.y,0.f,1.f);
    return {gameplay_ball_overlaps_reticle(error,tuning,ball),pitch,error,normalized,q};
}
}
