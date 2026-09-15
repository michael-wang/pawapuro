#include "reference_pitch.hpp"
#include "reference_scene.hpp"
#include <algorithm>
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
BattingStaging fixture_staging;
ReferencePitch fixture()
{
    const auto& s = fixture_staging;
    return ReferencePitch(s.release_position_m, s.reference_velocity_mps, s.strike_zone_plane_z());
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
int main(int argc, char** argv)
{
    try {
        require(argc == 2, "Expected authored staging path");
        fixture_staging = load_batting_staging(argv[1]);
        BattingStaging geometry;
        require(geometry.home_plate_depth_m() == geometry.strike_zone_width_m
            && geometry.strike_zone_plane_z() == geometry.home_plate_depth_m() / 2, "Plate proportions/centre differ");
        geometry.strike_zone_width_m = 1.5f;
        auto wider = ReferencePitch(geometry.release_position_m, geometry.reference_velocity_mps, geometry.strike_zone_plane_z());
        const auto prediction = predict_arrival(wider);
        wider.release();
        while (wider.phase != PitchPhase::Complete) (void)wider.advance(16'666'667);
        const auto actual = wider.arrival_at_plane();
        require(wider.tick == 93 && wider.previous.position_m.z > 0.75f && wider.current.position_m.z <= 0.75f,
            "Crossing still uses the old fixed plane");
        require(prediction.tick == actual.tick && prediction.time_s == actual.time_s
            && equal(prediction.state.position_m, actual.state.position_m)
            && equal(prediction.state.velocity_mps, actual.state.velocity_mps)
            && actual.state.position_m.z == geometry.strike_zone_plane_z(), "Prediction/evaluation differ");
        const auto baseline_prediction = predict_arrival(fixture());
        for (float width : {0.8636f, 1.5f}) {
            BattingStaging s;
            s.strike_zone_width_m = width;
            const ReferencePitch pitch(s.release_position_m, s.reference_velocity_mps, s.strike_zone_plane_z());
            const auto scene = make_batting_reference(s, predict_arrival(pitch).state.position_m, 16.0f / 9);
            float left = 1e9f, right = -1e9f, back = 1e9f, front = -1e9f;
            unsigned plate_vertices = 0;
            for (unsigned i = 0; i < scene.ball_vertex_start; ++i) {
                const auto& v = scene.vertices[i];
                if (v.position.y != 0.012f) continue; // The plate's existing ground clearance.
                ++plate_vertices;
                left = std::min(left, v.position.x); right = std::max(right, v.position.x);
                back = std::min(back, v.position.z); front = std::max(front, v.position.z);
            }
            require(plate_vertices == 9 && right-left == width && front-back == s.home_plate_depth_m()
                && (front+back)/2 == s.strike_zone_plane_z(), "Rendered plate disagrees with gameplay zone/plane");
            require(scene.zone_max_ndc.x > scene.zone_min_ndc.x && scene.zone_max_ndc.y > scene.zone_min_ndc.y,
                "Projected overlay bounds invalid");
        }
        // Off-axis composition must centre gameplay while world-X lines stay horizontal.
        for (float camera_x : {-1.25f, -0.75f, 0.0f}) {
            for (float aspect : {16.0f / 9, 4.0f / 3}) {
                BattingStaging s;
                s.camera_position_m.x = s.camera_target_m.x = camera_x;
                s.camera_target_m.y = 2.10f;
                s.strike_zone_bottom_m = 0.30f; s.strike_zone_top_m = 1.25f;
                const auto focus = project_batting_point(s,
                    {0, (s.strike_zone_bottom_m + s.strike_zone_top_m) / 2, s.strike_zone_plane_z()}, aspect);
                require(std::abs(focus.x) < 1e-6f, "Lens shift did not centre gameplay focus");
                const auto scene = make_batting_reference(s, baseline_prediction.state.position_m, aspect);
                // Vertical pitch gives top/bottom different depths; allow one pixel of bbox-centre drift.
                require(std::abs(scene.zone_min_ndc.x + scene.zone_max_ndc.x) * 1920 / 4 < 1,
                    "Overlay centre disagrees with gameplay focus");
                for (float side : {-1.0f, 1.0f}) {
                    const float inside = side * (s.strike_zone_width_m / 2 + 0.18f);
                    for (float z : {-0.6f, 1.4f}) {
                        const auto a = project_batting_point(s, {inside, 0.018f, z}, aspect);
                        const auto b = project_batting_point(s, {inside + side * 1.2f, 0.018f, z}, aspect);
                        require(std::abs(a.y - b.y) < 1e-6f, "World-X chalk line tilted in projection");
                    }
                }
            }
        }
        // An independently tuned release must not create a zero-length shoulder-to-hand limb.
        {
            BattingStaging s;
            const float h = s.pitcher_blockout_height_m;
            const auto o = s.pitcher_blockout_position_m;
            s.release_position_m = {o.x - 0.16f*h, o.y + 0.57f*h + 0.035f, o.z - 0.40f*h - 0.13f};
            bool rejected = false;
            try { (void)make_batting_reference(s, {0, 1, s.strike_zone_plane_z()}, 16.0f / 9); }
            catch (const std::runtime_error&) { rejected = true; }
            require(rejected, "Coincident release-pose endpoints were not rejected");
        }
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
            require(p.previous.position_m.z > p.evaluation_plane_z && p.current.position_m.z <= p.evaluation_plane_z,
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
        const auto baseline_actual = last.arrival_at_plane();
        require(equal(baseline_prediction.state.position_m, baseline_actual.state.position_m)
            && equal(baseline_prediction.state.velocity_mps, baseline_actual.state.velocity_mps)
            && baseline_prediction.time_s == baseline_actual.time_s, "Baseline prediction differs");
        const auto& s = fixture_staging;
        const DirectX::XMFLOAT3 centre{0, (s.strike_zone_bottom_m + s.strike_zone_top_m) / 2, s.strike_zone_plane_z()};
        const auto evaluated = baseline_actual.state.position_m;
        require(std::abs(evaluated.x - centre.x) < 0.0001f
            && std::abs(evaluated.y - centre.y) < 0.0001f && evaluated.z == centre.z,
            "Authored reference pitch misses slot 5 (0.1 mm tolerance)");
        const auto predicted_screen = project_batting_point(s, baseline_prediction.state.position_m, 16.0f / 9);
        const auto actual_screen = project_batting_point(s, baseline_actual.state.position_m, 16.0f / 9);
        require(predicted_screen.x == actual_screen.x && predicted_screen.y == actual_screen.y,
            "Prediction and evaluation screen coordinates differ");
        const auto centre_screen = project_batting_point(s, centre, 16.0f / 9);
        require(std::abs(predicted_screen.x - centre_screen.x) * 960 < 0.05f
            && std::abs(predicted_screen.y - centre_screen.y) * 540 < 0.05f, "Slot 5 screen alignment differs");
        std::printf("Centre evaluation: p=[%.9f, %.9f, %.9f] centre pixel=[%.6f, %.6f] prediction pixel=[%.6f, %.6f]\n",
            evaluated.x, evaluated.y, evaluated.z, (centre_screen.x+1)*960, (1-centre_screen.y)*540,
            (predicted_screen.x+1)*960, (1-predicted_screen.y)*540);
        const auto& p = last.current.position_m;
        const auto& v = last.current.velocity_mps;
        std::printf("PASS: deterministic ticks, 30/60/120 FPS, catch-up, pause, single-step, one-shot arrival, 20 rethrows with identical per-tick trajectories.\n"
            "Reference: tick=%llu t=%.9f p=[%.6f, %.6f, %.6f] v=[%.6f, %.6f, %.6f]\n",
            last.tick, static_cast<double>(last.tick) / pitch_hz, p.x, p.y, p.z, v.x, v.y, v.z);
        return 0;
    } catch (const std::exception& error) { std::fprintf(stderr, "%s\n", error.what()); return 1; }
}
