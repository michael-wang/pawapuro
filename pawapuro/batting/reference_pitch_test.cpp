#include "reference_pitch.hpp"
#include <cmath>
#include <cstdio>
#include <stdexcept>

using namespace pawapuro;
namespace {
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
bool equal(DirectX::XMFLOAT3 a, DirectX::XMFLOAT3 b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
bool equal(const ReferencePitch& a, const ReferencePitch& b)
{
    return a.tick == b.tick && a.phase == b.phase && equal(a.current.position_m, b.current.position_m)
        && equal(a.current.velocity_mps, b.current.velocity_mps);
}
ReferencePitch fixture()
{
    const BattingStaging s;
    return ReferencePitch(s.release_position_m, s.reference_velocity_mps);
}
ReferencePitch ticks(unsigned count)
{
    auto p = fixture();
    require(p.release(), "Ready release failed");
    p.toggle_pause();
    for (unsigned i = 0; i < count; ++i) (void)p.single_step();
    return p;
}
}
int main()
{
    try {
        // Compare fields exactly in one build/platform; never compare struct padding.
        const auto expected = ticks(48);
        for (int run = 0; run < 20; ++run) require(equal(ticks(48), expected), "Fixed N ticks differ");
        const auto initial = fixture().current;
        const double t = 48.0 / pitch_hz;
        const double analytic_y = initial.position_m.y + initial.velocity_mps.y * t + 0.5 * earth_gravity_mps2 * t * t;
        require(std::abs(expected.current.position_m.y - analytic_y) < 0.0002, "Gravity integration drift");
        for (unsigned fps : {30u, 60u, 120u}) {
            auto p = fixture(); p.release();
            std::uint64_t previous_time = 0;
            unsigned arrivals = 0;
            for (unsigned frame = 1; frame <= fps; ++frame) {
                const auto time = static_cast<std::uint64_t>(frame) * 1'000'000'000 / fps;
                arrivals += p.advance(time - previous_time) ? 1u : 0u;
                previous_time = time;
                if (frame == fps / 5) require(equal(p, expected), "Render chunking changed tick/state at 0.2 s");
            }
            require(equal(p, ticks(95)), "Render chunking changed arrival state");
            require(arrivals == 1 && p.tick == 95, "Arrival must occur once at tick 95");
            require(p.previous.position_m.z > plate_front_z_m && p.current.position_m.z <= plate_front_z_m,
                "Arrival does not bracket plate plane");
            require(!p.advance(1'000'000'000) && !p.single_step(), "Complete advanced without release");
        }
        auto slow = fixture(); slow.release();
        (void)slow.advance(250'000'000);
        require(slow.tick == 16 && slow.pending_ticks == 44, "Catch-up cap/debt incorrect");
        while (slow.pending_ticks) (void)slow.advance(0);
        require(equal(slow, ticks(60)), "Catch-up dropped ticks");
        auto paused = fixture();
        (void)paused.advance(1'000'000'000); paused.single_step();
        require(paused.tick == 0, "Ready advanced");
        paused.release(); (void)paused.advance(10'000'000); paused.toggle_pause();
        const auto snapshot = paused;
        (void)paused.advance(5'000'000'000);
        require(equal(paused, snapshot), "Pause advanced simulation");
        require(!paused.single_step() && paused.tick == snapshot.tick + 1 && paused.paused, "Single-step not exactly one tick");
        (void)paused.advance(1'000'000'000);
        require(paused.tick == snapshot.tick + 1, "Paused step resumed automatically");
        paused.toggle_pause(); (void)paused.advance(2'500'000);
        require(paused.tick == 4, "Pause/resume lost fractional accumulator or included paused wall time");
        auto last = ticks(94);
        require(last.single_step() && last.phase == PitchPhase::Complete && !last.single_step(), "Paused arrival not one-shot");
        auto repeated = fixture();
        for (unsigned run = 0; run <= 20; ++run) {
            require(repeated.release(), "Rethrow rejected");
            require(repeated.tick == 0 && repeated.pending_ticks == 0 && !repeated.paused
                && repeated.phase == PitchPhase::InFlight && equal(repeated.current.position_m, initial.position_m)
                && equal(repeated.current.velocity_mps, initial.velocity_mps)
                && equal(repeated.previous.position_m, initial.position_m)
                && equal(repeated.previous.velocity_mps, initial.velocity_mps), "Rethrow retained old state");
            require(!repeated.release(), "In-flight release accepted");
            (void)repeated.advance(4'166'666);
            require(repeated.tick == 0, "Rethrow retained fractional time");
            (void)repeated.advance(1);
            require(equal(repeated, ticks(1)), "Fresh fractional time/state incorrect");
            repeated.toggle_pause();
            require(!repeated.release(), "Paused release accepted");
            unsigned arrivals = 0;
            for (unsigned tick = 2; tick <= 95; ++tick) {
                arrivals += repeated.single_step() ? 1u : 0u;
                require(equal(repeated, ticks(tick)), "Rethrow trajectory differs");
            }
            require(arrivals == 1 && !repeated.advance(1'000'000'000)
                && !repeated.single_step() && equal(repeated, last), "Rethrow arrival differs");
        }
        const auto& p = last.current.position_m;
        const auto& v = last.current.velocity_mps;
        std::printf("PASS: deterministic ticks, 30/60/120 FPS, catch-up, pause, single-step, one-shot arrival, 20 rethrows with identical per-tick trajectories.\n"
            "Reference: tick=%llu t=%.9f p=[%.6f, %.6f, %.6f] v=[%.6f, %.6f, %.6f]\n",
            last.tick, static_cast<double>(last.tick) / pitch_hz, p.x, p.y, p.z, v.x, v.y, v.z);
        return 0;
    } catch (const std::exception& error) { std::fprintf(stderr, "%s\n", error.what()); return 1; }
}
