#include "pitch_delivery.hpp"
#include "pawapuro/batting/reference_scene.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <vector>
using namespace pawapuro;
namespace {
void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
bool equal(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b) { return a.x==b.x && a.y==b.y && a.z==b.z; }
bool equal(BallState a,BallState b) { return equal(a.position_m,b.position_m) && equal(a.velocity_mps,b.velocity_mps); }
bool equal(PitchArrival a,PitchArrival b) { return a.tick==b.tick && a.time_s==b.time_s && equal(a.state,b.state); }
float distance(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b) { return std::sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)+(a.z-b.z)*(a.z-b.z)); }
DirectX::XMFLOAT3 grip(const PitchDelivery& d) { return {d.motion.grip_world[12],d.motion.grip_world[13],d.motion.grip_world[14]}; }
struct Sample {
    BallState ball;
    std::uint64_t animation_tick,pitch_tick;
    PitchPhase phase;
    BallOwner owner;
    unsigned release_count;
    std::vector<engine::GlbMatrix> pose;
};
Sample sample(const PitchDelivery& d) { return {d.pitch.current,d.motion.tick,d.pitch.tick,d.pitch.phase,d.ball_owner,d.release_count,d.motion.pose.world}; }
void same(const PitchDelivery& d,const Sample& s) {
    require(equal(d.pitch.current,s.ball) && d.pitch.tick==s.pitch_tick && d.pitch.phase==s.phase
        && d.motion.tick==s.animation_tick && d.ball_owner==s.owner && d.release_count==s.release_count
        && d.motion.pose.world==s.pose,"Delivery state/pose differs at identical tick");
}
void frozen(PitchDelivery& d) { auto s=sample(d);const auto tick=d.tick,credit=d.pending_ticks;d.advance(9'000'000'000);same(d,s);require(d.tick==tick && d.pending_ticks==credit,"Pause consumed wall time"); }
}
int main(int argc,char** argv) {
 try {
    require(argc==4,"Expected GLB, metadata, staging");const auto staging=load_batting_staging(argv[3]);
    PitchDelivery d(argv[1],argv[2],staging);
    ReferencePitch standalone(staging.release_position_m,staging.reference_velocity_mps,staging.strike_zone_plane_z());
    const auto prediction=predict_arrival(standalone);std::vector<BallState> path{standalone.initial};standalone.release();
    while (standalone.phase!=PitchPhase::Complete) {standalone.step_tick();path.push_back(standalone.current);}
    require(path.size()==96 && prediction.tick==95,"Existing fixture arrival changed");
    const auto arrival=standalone.arrival_at_plane();require(equal(arrival,prediction),"Standalone prediction mismatch");
    require(d.motion.release_tick==384 && d.motion.end_tick==816,"Accepted clip metadata changed");
    d.advance(1'000'000'000);d.single_step();require(d.tick==0 && d.pitch.phase==PitchPhase::Ready,"Ready advanced");
    require(d.ball_owner==BallOwner::Hand && !d.release_count && d.pitch.tick==0 && equal(d.ball_center(),grip(d)),"Ready held ball mismatch");
    const auto ready=grip(d);
    std::printf("HELD tick=0 grip=(%.9g,%.9g,%.9g) rendered=(%.9g,%.9g,%.9g) delta=0\n",ready.x,ready.y,ready.z,ready.x,ready.y,ready.z);
    std::vector<Sample> history{sample(d)};d.start();d.toggle_pause();
    DirectX::XMFLOAT3 last_held{};
    for (unsigned tick=1;tick<=816;++tick) {
        if (tick==192 || tick==384 || tick==385 || tick==500) frozen(d);
        const bool complete=d.single_step();require(d.tick==tick && complete==(tick==816),"Single-step/completion ordering");
        require(!d.motion.pending_ticks && !d.pitch.pending_ticks && !d.motion.paused && !d.pitch.paused,"Child clock consumed elapsed time");
        if (tick<384) {
            require(d.ball_owner==BallOwner::Hand && d.pitch.phase==PitchPhase::Ready && d.pitch.tick==0 && !d.release_count,"Pre-release owner/physics changed");
            require(equal(d.ball_center(),grip(d)),"Held centre not same authoritative grip");
        } else {
            const auto pt=std::min<std::uint64_t>(tick-384,95);
            require(d.ball_owner==BallOwner::Simulation && d.release_count==1 && d.pitch.tick==pt && equal(d.pitch.current,path[pt]),"Release integrated early or flight differs");
        }
        if (tick==192 || tick==284 || tick==312 || tick==383) {
            const auto g=grip(d),b=d.ball_center();const auto t=d.ball_translation();
            require(distance({staging.release_position_m.x+t.x,staging.release_position_m.y+t.y,staging.release_position_m.z+t.z},g)<1e-6f,"Translated range centre mismatch");
            std::printf("HELD tick=%u grip=(%.9g,%.9g,%.9g) rendered=(%.9g,%.9g,%.9g) delta=%.9g\n",tick,g.x,g.y,g.z,b.x,b.y,b.z,distance(g,b));
        }
        if (tick==383) last_held=d.ball_center();
        if (tick==384) {
            require(d.pitch.phase==PitchPhase::InFlight && d.pitch.tick==0 && equal(d.pitch.current,d.pitch.initial),"Release tick advanced physics");
            const auto g=grip(d),initial=d.pitch.initial.position_m;
            const auto pg=project_batting_point(staging,g,16.0f/9),pi=project_batting_point(staging,initial,16.0f/9),pl=project_batting_point(staging,last_held,16.0f/9);
            const float px=std::hypot((pg.x-pi.x)*960,(pg.y-pi.y)*540);
            require(distance(g,initial)<.0001f,"Release tolerance violated");
            std::printf("HANDOFF same_tick_world_m=%.9g pixel=%.9g previous_held_to_initial_m=%.9g pixel=%.9g (includes normal hand tick motion)\n",distance(g,initial),px,distance(last_held,initial),std::hypot((pl.x-pi.x)*960,(pl.y-pi.y)*540));
        }
        if (tick==479) {
            require(d.phase==DeliveryPhase::Delivering && d.pitch.phase==PitchPhase::Complete && d.motion.phase==MotionPhase::Playing,"Gameplay completion was delayed to recovery");
            require(equal(d.pitch.arrival_at_plane(),arrival),"Immediate plane evaluation unavailable/different");
            require(!equal(d.pitch.current.position_m,arrival.state.position_m),"Raw state snapped to plane");
            require(!d.start(),"Replay accepted during recovery");
        }
        history.push_back(sample(d));
    }
    require(d.phase==DeliveryPhase::Complete && equal(d.pitch.arrival_at_plane(),prediction),"Final completion/prediction mismatch");
    require(!d.advance(1'000'000'000) && !d.single_step(),"Complete advanced");
    for (unsigned fps:{30u,60u,120u}) {
        d.start();std::uint64_t previous=0;
        for (unsigned f=1;f<=fps*4;++f) {
            const std::uint64_t now=1'000'000'000ull*f/fps;d.advance(now-previous);previous=now;
            const auto tick=std::min<std::uint64_t>(now*240/1'000'000'000,816);
            require(d.tick==tick,"Chunking changed authoritative tick");same(d,history[tick]);
        }
        require(equal(d.pitch.arrival_at_plane(),prediction),"Chunking changed arrival");
    }
    // Replay from the same owner, including a large backlog and fractional credit.
    for (unsigned run=0;run<20;++run) {
        require(d.start() && !d.start(),"Replay/start phase guard");
        require(d.tick==0 && !d.pending_ticks && !d.release_count && d.ball_owner==BallOwner::Hand && d.pitch.phase==PitchPhase::Ready && equal(d.ball_center(),grip(d)),"Replay did not reset ownership/state/debt");
        d.advance(4'166'666);require(d.tick==0,"Replay retained fractional credit");d.advance(1);require(d.tick==1,"Fresh fraction wrong");
        d.advance(10'000'000'000);require(d.tick==17 && d.pending_ticks>0,"Catch-up cap lost debt");
        d.toggle_pause();frozen(d);
        while (d.phase!=DeliveryPhase::Complete) {d.single_step();same(d,history[d.tick]);}
        require(equal(d.pitch.arrival_at_plane(),prediction) && d.release_count==1 && !d.pending_ticks,"Replay result/debt differs");
    }
    // In-memory slow fixture covers animation-end-before-arrival; authored Data is untouched.
    auto slow_staging=staging;slow_staging.reference_velocity_mps.z=-5;
    PitchDelivery slow(argv[1],argv[2],slow_staging);slow.start();slow.toggle_pause();
    while (slow.motion.phase!=MotionPhase::Complete) slow.single_step();
    require(slow.tick==816 && slow.pitch.phase==PitchPhase::InFlight && slow.phase==DeliveryPhase::Delivering,"Animation end incorrectly completed slow delivery");
    const auto final_pose=slow.motion.pose.world;frozen(slow);const auto ball_tick=slow.pitch.tick;slow.single_step();
    require(slow.pitch.tick==ball_tick+1 && slow.motion.pose.world==final_pose && slow.motion.tick==816,"After-animation step altered pose or missed flight tick");
    while (slow.phase!=DeliveryPhase::Complete) slow.single_step();
    require(slow.motion.pose.world==final_pose,"Final pose changed while waiting for flight");
    std::printf("PASS delivery: release=384/pitch0, first-flight=385/pitch1, gameplay=479/pitch95, recovery-end=816; 20 replays, 30/60/120 chunking, pause/step/debt, held centres, late flight.\n");
    return 0;
 } catch (const std::exception& e) {std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}
}
