#pragma once
#include "reference_pitch.hpp"
#include <algorithm>
#include <cmath>
#include <optional>
#include <stdexcept>

namespace pawapuro {
struct BattingInteractionPassage { double enter_s,exit_s,reference_s; };
struct TemporalInterval { double start_s,end_s; };
enum class SwingTimingState { Early, Overlap, Late };
struct TimingInteraction {
    SwingTimingState state;
    double start_s,peak_s,end_s;
    std::optional<TemporalInterval> overlap;
    double efficiency,offset_ms;
};
// Current reference pitch has constant negative Z velocity. X/Y never gate this slab.
inline BattingInteractionPassage batting_interaction_passage(const ReferencePitch& pitch,double release_s,
    BattingInteractionTuning tuning) {
    const double z=pitch.initial.position_m.z,vz=pitch.initial.velocity_mps.z,plane=pitch.evaluation_plane_z;
    if(!std::isfinite(vz)||vz>=0||!std::isfinite(tuning.half_depth_m)||tuning.half_depth_m<=0
        ||z<=plane+tuning.half_depth_m)throw std::runtime_error("Invalid reference pitch interaction slab");
    return {release_s+(plane+tuning.half_depth_m-z)/vz,release_s+(plane-tuning.half_depth_m-z)/vz,
        release_s+predict_arrival(pitch).time_s};
}
inline double normal_swing_potential(double tau_s,SwingPhasePotentialTuning tuning) {
    const double start=tuning.normal_start_ms/1000.,peak=tuning.normal_peak_ms/1000.,end=tuning.normal_end_ms/1000.;
    if(tau_s<=start||tau_s>=end)return 0;
    const double u=tau_s<=peak?(tau_s-start)/(peak-start):(end-tau_s)/(end-peak);
    return u*u*(3-2*u);
}
inline TimingInteraction timing_interaction(double commit_s,BattingInteractionPassage ball,SwingPhasePotentialTuning tuning) {
    const double start=commit_s+tuning.normal_start_ms/1000.,peak=commit_s+tuning.normal_peak_ms/1000.,end=commit_s+tuning.normal_end_ms/1000.;
    TimingInteraction result{end<=ball.enter_s?SwingTimingState::Early:SwingTimingState::Late,
        start,peak,end,std::nullopt,0,(peak-ball.reference_s)*1000};
    const double lo=std::max(start,ball.enter_s),hi=std::min(end,ball.exit_s);
    if(lo>=hi)return result;
    result.state=SwingTimingState::Overlap;result.overlap=TemporalInterval{lo,hi};
    // Exact polynomial primitive of smoothstep, split at its peak. No frame-dependent accumulation.
    const auto primitive=[](double u){return u*u*u*(1-u*.5);};
    double integral=0;
    if(lo<peak){const double b=std::min(hi,peak),duration=peak-start;
        integral+=duration*(primitive((b-start)/duration)-primitive((lo-start)/duration));}
    if(hi>peak){const double a=std::max(lo,peak),duration=end-peak;
        integral+=duration*(primitive((end-a)/duration)-primitive((end-hi)/duration));}
    result.efficiency=std::clamp(integral/(ball.exit_s-ball.enter_s),0.,1.);
    return result;
}
}
