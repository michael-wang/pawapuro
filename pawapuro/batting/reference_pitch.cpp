#include "reference_pitch.hpp"

namespace pawapuro {
bool ReferencePitch::release()
{
    if (phase != PitchPhase::Ready) return false;
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
    if (previous.position_m.z > plate_front_z_m && p.z <= plate_front_z_m) {
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
}
