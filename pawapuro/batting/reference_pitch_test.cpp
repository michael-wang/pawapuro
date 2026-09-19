#include "reference_pitch.hpp"
#include "reference_scene.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
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
        {
        const auto normal=predict_arrival(fixture()).state.position_m;
        const auto strict=project_batting_point(fixture_staging,normal,16.f/9);
        const auto optional=try_project_batting_point(fixture_staging,normal,16.f/9);
        require(optional&&optional->x==strict.x&&optional->y==strict.y,"safe/strict normal projection differs");
        std::vector<engine::Vertex> hidden_aid;hidden_aid.reserve(ball_readability_vertex_count);
        const auto* storage=hidden_aid.data();const auto capacity=hidden_aid.capacity();
        const auto check_hidden=[&](const BattingStaging& s,DirectX::XMFLOAT3 point,bool enabled=true) {
            hidden_aid.clear();append_ball_readability(hidden_aid,s,point,PitchPhase::InFlight,enabled,1920,1080);
            require(hidden_aid.size()==ball_readability_vertex_count&&hidden_aid.data()==storage&&hidden_aid.capacity()==capacity,"hidden aid count/allocation");
            for(const auto& v:hidden_aid)require(equal(v.position,{})&&equal(v.color,{}),"invalid aid did not append finite degenerate vertices");
        };
        auto behind=fixture_staging.camera_position_m;behind.z-=1;
        require(!try_project_batting_point(fixture_staging,behind,16.f/9),"behind safe projection fabricated NDC");
        bool threw=false;try{(void)project_batting_point(fixture_staging,behind,16.f/9);}catch(const std::runtime_error&){threw=true;}
        require(threw,"strict behind projection no longer fails");check_hidden(fixture_staging,behind);
        for(float bad:{std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}) {
            require(!try_project_batting_point(fixture_staging,{bad,0,0},16.f/9),"nonfinite point projected");
            check_hidden(fixture_staging,{bad,0,0});
        }
        auto invalid_camera=fixture_staging;invalid_camera.camera_target_m=invalid_camera.camera_position_m;
        check_hidden(invalid_camera,behind,false); // OFF must never evaluate even an invalid camera.
        // Camera-right is mathematically parallel to the image plane. Near w=0,
        // finite-precision point construction can still put only the sizing edge behind it.
        auto near_camera=fixture_staging;near_camera.camera_position_m={0,0,-5};near_camera.camera_target_m={-2,0,0};
        const DirectX::XMFLOAT3 point{-3e-7f,0,-5};
        const auto forward=DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&near_camera.camera_target_m),DirectX::XMLoadFloat3(&near_camera.camera_position_m));
        const auto right=DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVectorSet(0,1,0,0),forward));
        DirectX::XMFLOAT3 edge;
        DirectX::XMStoreFloat3(&edge,DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&point),DirectX::XMVectorScale(right,near_camera.gameplay_ball.radius_m)));
        require(try_project_batting_point(near_camera,point,16.f/9)&&!try_project_batting_point(near_camera,edge,16.f/9),"center-valid edge-invalid fixture");
        check_hidden(near_camera,point);
        require(!try_project_batting_point(fixture_staging,{std::numeric_limits<float>::max(),1,0},16.f/9),"nonfinite normalized coordinate accepted");
        std::puts("PASS safe/strict projection, center/edge unavailable, finite hidden geometry, OFF without camera evaluation.");
        }
        // One gameplay radius drives the sphere, both cue footprints and BallAid.
        for(float radius:{.085f,.1f}) {
            auto s=fixture_staging;s.gameplay_ball.radius_m=radius;
            const auto position=predict_arrival(fixture()).state.position_m;
            constexpr float aspect=16.f/9;
            const auto scene=make_batting_reference(s,position,aspect,{});
            unsigned body_count=0,seam_count=0;
            for(unsigned i=scene.ball_vertex_start;i<scene.overlay_vertex_start;++i) {
                const auto p=scene.vertices[i].position;
                const auto color=scene.vertices[i].color;
                const bool seam=color.y==.12f;
                if(seam){++seam_count;require(color.x==.95f&&color.z==.16f,"world seam red");}
                else {++body_count;require(color.x>=.864f&&color.y>=.855f&&color.z>=.783f,"world body not baseball-white");}
                require(std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z),"nonfinite seam/body");
                require(std::abs(std::hypot(p.x-s.release_position_m.x,p.y-s.release_position_m.y,p.z-s.release_position_m.z)-radius*(seam?1.003f:1.f))<1e-6f,"world body/seam radius differs from Data/offset");
            }
            require(body_count==8*16*6&&seam_count==2*24*6,"translated baseball range missing body/seams");
            for(auto style:{ArrivalCueStyle::Baseball,ArrivalCueStyle::Ring}) {
                const auto& current=style==ArrivalCueStyle::Baseball?scene.arrival_baseball:scene.arrival_ring;
                const auto& previous=style==ArrivalCueStyle::Baseball?scene.previous_baseball:scene.previous_ring;
                require(current.size()==636&&previous.size()==636,"ghost geometry count");
                for(std::size_t i=0;i<current.size();++i) {
                    require(current[i].position.x==previous[i].position.x&&current[i].position.y==previous[i].position.y&&current[i].position.z==previous[i].position.z,"ghost changed contour/point");
                    const auto c=previous[i].color;
                    require(c.x<.6f&&c.y<.6f&&c.z<.6f&&c.x!=current[i].color.x,"ghost not muted");
                }
            }
            const auto forward=DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&s.camera_target_m),DirectX::XMLoadFloat3(&s.camera_position_m));
            const auto right=DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVectorSet(0,1,0,0),forward));
            DirectX::XMFLOAT3 edge;
            DirectX::XMStoreFloat3(&edge,DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),DirectX::XMVectorScale(right,radius)));
            const auto center=project_batting_point(s,position,aspect);
            const float projected_radius=std::abs(project_batting_point(s,edge,aspect).x-center.x);
            for(const auto* cue:{&scene.arrival_baseball,&scene.arrival_ring}) {
                float extent=0;for(const auto& v:*cue)extent=std::max(extent,std::abs(v.position.x-center.x));
                require(std::abs(extent-projected_radius)<1e-6f,"arrival footprint differs from gameplay Data");
            }
            std::vector<engine::Vertex> aid;
            append_ball_readability(aid,s,position,PitchPhase::InFlight,true,1920,1080);
            float extent=0;for(const auto& v:aid)extent=std::max(extent,std::abs(v.position.x-center.x));
            require(std::abs(extent-(std::max(5.f,projected_radius*960)+2)/960)<1e-6f,"BallAid radius derivation changed");
        }
        // Pixel constants scale with height; projected radius must not be scaled twice.
        for (auto position:{fixture_staging.release_position_m,predict_arrival(fixture()).state.position_m}) {
            std::vector<engine::Vertex> a,b;
            append_ball_readability(a,fixture_staging,position,PitchPhase::InFlight,true,1920,1080);
            append_ball_readability(b,fixture_staging,position,PitchPhase::InFlight,true,2880,1620);
            require(a.size()==b.size(),"BallAid scaling changes capacity");
            for (size_t i=0;i<a.size();++i)
                require(std::abs(a[i].position.x-b[i].position.x)<1e-6f
                    && std::abs(a[i].position.y-b[i].position.y)<1e-6f,"BallAid double-scaled projection or unscaled pixel stroke");
        }
        // Current-ball marker follows every authoritative flight sample, never the arrival prediction.
        for (const auto size : {DirectX::XMUINT2{1920,1080}, DirectX::XMUINT2{1280,720}, DirectX::XMUINT2{2880,1620}}) {
            auto flight=fixture();
            std::vector<engine::Vertex> marker;
            marker.reserve(ball_readability_vertex_count);
            const auto hidden=[&](bool enabled) {
                marker.clear();
                append_ball_readability(marker,fixture_staging,flight.current.position_m,flight.phase,enabled,size.x,size.y);
                require(marker.size()==ball_readability_vertex_count,"Marker capacity changed");
                for (const auto& vertex:marker)
                    require(equal(vertex.position,{0,0,0}),"Hidden ball marker draws geometry");
            };
            hidden(true); // Before release.
            flight.release();
            while (flight.phase==PitchPhase::InFlight) {
                hidden(false);
                const auto before=flight.current;
                marker.clear();
                append_ball_readability(marker,fixture_staging,flight.current.position_m,flight.phase,true,size.x,size.y);
                require(equal(before.position_m,flight.current.position_m) && equal(before.velocity_mps,flight.current.velocity_mps),
                    "Presentation changed flight state");
                float left=1e9f,right=-1e9f,bottom=1e9f,top=-1e9f;
                for (const auto& vertex:marker) {
                    require(std::isfinite(vertex.position.x)&&std::isfinite(vertex.position.y),"Nonfinite marker");
                    left=std::min(left,vertex.position.x);right=std::max(right,vertex.position.x);
                    bottom=std::min(bottom,vertex.position.y);top=std::max(top,vertex.position.y);
                }
                const auto matrix=batting_view_projection(fixture_staging,float(size.x)/float(size.y));
                const auto center=DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&flight.current.position_m),DirectX::XMLoadFloat4x4(&matrix));
                require(std::abs((left+right)/2-DirectX::XMVectorGetX(center))*size.x/2<0.02f
                    && std::abs((bottom+top)/2-DirectX::XMVectorGetY(center))*size.y/2<0.02f,"Marker/ball projection differs");
                require(std::abs((right-left)*size.x-(top-bottom)*size.y)<0.02f,"Marker is not circular in pixels");
                flight.step_tick();
            }
            hidden(true); // Frozen Complete ball must not retain the aid.
        }
        std::puts("PASS: ball readability flight projection, OFF/Ready/Complete suppression, fixed capacity, unchanged flight.");
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
            const auto scene = make_batting_reference(s, predict_arrival(pitch).state.position_m, 16.0f / 9, {});
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
                const auto scene = make_batting_reference(s, baseline_prediction.state.position_m, aspect, {});
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
        // Pitcher transport no longer depends on release; a former procedural overlap is valid.
        {
            BattingStaging s;
            const float h = s.pitcher_blockout_height_m;
            const auto o = s.pitcher_blockout_position_m;
            s.release_position_m = {o.x - 0.16f*h, o.y + 0.57f*h + 0.035f, o.z - 0.40f*h - 0.13f};
            (void)make_batting_reference(s, {0, 1, s.strike_zone_plane_z()}, 16.0f / 9, {});
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
