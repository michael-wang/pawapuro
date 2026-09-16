#include "reference_pitch.hpp"
#include <stdexcept>
#include <cmath>

namespace pawapuro {
BallState sample_reference_pitch(const BallState& initial, double t)
{
    if (!std::isfinite(t) || t < 0) throw std::runtime_error("Pitch study time must be finite and nonnegative.");
    const auto& p=initial.position_m; const auto& v=initial.velocity_mps;
    return {{static_cast<float>(p.x+v.x*t),static_cast<float>(p.y+v.y*t+0.5*earth_gravity_mps2*t*t),static_cast<float>(p.z+v.z*t)},
        {v.x,static_cast<float>(v.y+earth_gravity_mps2*t),v.z}};
}
void ReferencePitch::reset()
{
    previous = current = initial;
    tick = pending_ticks = fractional_credit = 0;
    paused = false; phase = PitchPhase::Ready;
}
bool ReferencePitch::step_tick()
{
    return phase == PitchPhase::InFlight ? integrate_tick() : false;
}
bool ReferencePitch::release()
{
    if (phase == PitchPhase::InFlight) return false;
    previous = current = initial;
    tick = pending_ticks = fractional_credit = 0;
    paused = false;
    phase = PitchPhase::InFlight;
    return true;
}
void ReferencePitch::toggle_pause()
{
    if (phase == PitchPhase::InFlight) paused = !paused;
}
bool ReferencePitch::single_step()
{
    return phase == PitchPhase::InFlight && paused ? integrate_tick() : false;
}
bool ReferencePitch::integrate_tick()
{
    previous = current;
    auto& p = current.position_m;
    auto& v = current.velocity_mps;
    // Constant-acceleration update; the fixed dt never comes from the render loop.
    p.x += v.x * pitch_dt;
    p.y += v.y * pitch_dt + 0.5f * earth_gravity_mps2 * pitch_dt * pitch_dt;
    p.z += v.z * pitch_dt;
    v.y += earth_gravity_mps2 * pitch_dt;
    ++tick;
    if (previous.position_m.z > evaluation_plane_z && p.z <= evaluation_plane_z) {
        phase = PitchPhase::Complete;
        paused = false;
        pending_ticks = fractional_credit = 0;
        return true;
    }
    return false;
}
bool ReferencePitch::advance(std::uint64_t elapsed_ns)
{
    if (phase != PitchPhase::InFlight || paused) return false;
    constexpr std::uint64_t ns_per_second = 1'000'000'000;
    // Split whole seconds before scaling to avoid elapsed_ns * Hz overflow.
    pending_ticks += (elapsed_ns / ns_per_second) * pitch_hz;
    fractional_credit += (elapsed_ns % ns_per_second) * pitch_hz;
    pending_ticks += fractional_credit / ns_per_second;
    fractional_credit %= ns_per_second;
    // Retain debt rather than dropping ticks. Pause retains debt/fraction but adds no wall time.
    for (unsigned count = 0; count < max_pitch_ticks_per_frame && pending_ticks; ++count) {
        --pending_ticks;
        if (integrate_tick()) return true;
    }
    return false;
}
const char* ReferencePitch::state_name() const
{
    if (phase == PitchPhase::Ready) return "Ready";
    if (phase == PitchPhase::Complete) return "Complete";
    return paused ? "Paused" : "InFlight";
}
PitchArrival ReferencePitch::arrival_at_plane() const
{
    if (phase != PitchPhase::Complete)
        throw std::runtime_error("Pitch evaluation requires a completed plane crossing.");
    // A sample within the last fixed tick; do not overwrite the integrated state.
    const float fraction = (previous.position_m.z - evaluation_plane_z)
        / (previous.position_m.z - current.position_m.z);
    const auto lerp = [fraction](DirectX::XMFLOAT3 a, DirectX::XMFLOAT3 b) {
        return DirectX::XMFLOAT3{a.x + (b.x - a.x) * fraction,
            a.y + (b.y - a.y) * fraction, a.z + (b.z - a.z) * fraction};
    };
    BallState evaluated{lerp(previous.position_m, current.position_m),
        lerp(previous.velocity_mps, current.velocity_mps)};
    evaluated.position_m.z = evaluation_plane_z;
    return {evaluated, (static_cast<double>(tick - 1) + fraction) / pitch_hz, tick};
}

PitchArrival predict_arrival(const ReferencePitch& pitch)
{
    ReferencePitch prediction(pitch.initial.position_m, pitch.initial.velocity_mps, pitch.evaluation_plane_z);
    prediction.release();
    prediction.toggle_pause();
    // Validated startup fixtures cross in under one second; bound failures explicitly.
    for (std::uint64_t tick = 0; tick < pitch_hz * 2; ++tick)
        if (prediction.single_step()) return prediction.arrival_at_plane();
    throw std::runtime_error("Reference pitch prediction did not reach its evaluation plane within 2 s.");
}

}
