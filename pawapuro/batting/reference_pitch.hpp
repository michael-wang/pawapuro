#pragma once
#include "staging.hpp"
#include <cstdint>

namespace pawapuro {
inline constexpr std::uint64_t pitch_hz = 240;
inline constexpr float pitch_dt = 1.0f / static_cast<float>(pitch_hz);
inline constexpr float earth_gravity_mps2 = -9.80665f;
inline constexpr unsigned max_pitch_ticks_per_frame = 16;

enum class PitchPhase { Ready, InFlight, Complete };
struct BallState {
    DirectX::XMFLOAT3 position_m;
    DirectX::XMFLOAT3 velocity_mps;
};

// Retain the startup fixture so each completed pitch can be repeated exactly.
struct ReferencePitch {
    ReferencePitch(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 velocity)
        : initial{position, velocity}, previous(initial), current(initial) {}
    const BallState initial;
    BallState previous, current;
    PitchPhase phase = PitchPhase::Ready;
    bool paused = false;
    std::uint64_t tick = 0;
    std::uint64_t pending_ticks = 0;
    bool release();
    void toggle_pause();
    bool single_step(); // True only on the one arrival transition.
    bool advance(std::uint64_t elapsed_ns);
    const char* state_name() const;

private:
    // ns * Hz credit avoids rounding a 240 Hz tick to a whole nanosecond.
    std::uint64_t fractional_credit = 0;
    bool integrate_tick();
};
}
