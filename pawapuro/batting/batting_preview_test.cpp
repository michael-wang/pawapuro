#include "batting_preview.hpp"
#include <algorithm>
#include <cstdio>
#include <stdexcept>
using namespace pawapuro;
namespace {
void require(bool ok,const char* why) { if (!ok) throw std::runtime_error(why); }
bool same(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b) {return a.x==b.x&&a.y==b.y&&a.z==b.z;}
bool same(BallState a,BallState b) {return same(a.position_m,b.position_m)&&same(a.velocity_mps,b.velocity_mps);}
struct Sample {
    BallState ball;
    std::uint64_t delivery_tick,pitcher_tick,batter_tick,pitch_tick,arrival_tick;
    DeliveryPhase delivery_phase;PreviewPhase preview_phase;BallOwner owner;
    unsigned releases;
    std::vector<engine::GlbMatrix> pitcher_pose,batter_pose;
};
Sample sample(const BattingPreview& p) {
    return {p.delivery.pitch.current,p.delivery.tick,p.delivery.motion.tick,p.batter.tick,p.delivery.pitch.tick,p.arrival_tick,
        p.delivery.phase,p.phase,p.delivery.ball_owner,p.delivery.release_count,p.delivery.motion.pose.world,p.batter.pose.world};
}
void equal(const BattingPreview& p,const Sample& s) {
    const auto a=sample(p);require(same(a.ball,s.ball) && a.delivery_tick==s.delivery_tick && a.pitcher_tick==s.pitcher_tick
        && a.batter_tick==s.batter_tick && a.pitch_tick==s.pitch_tick && a.arrival_tick==s.arrival_tick
        && a.delivery_phase==s.delivery_phase && a.preview_phase==s.preview_phase && a.owner==s.owner && a.releases==s.releases
        && a.pitcher_pose==s.pitcher_pose && a.batter_pose==s.batter_pose,"same preview tick changed state/poses");
}
void frozen(BattingPreview& p) {const auto s=sample(p);const auto debt=p.pending_ticks,tick=p.tick;p.advance(9'000'000'000);equal(p,s);require(p.tick==tick&&p.pending_ticks==debt,"paused clock advanced");}
}
int main(int argc,char** argv) {
 try {
    require(argc==3,"expected batting directory, staging");const auto staging=load_batting_staging(argv[2]);BattingPreview p(argv[1],staging);
    ReferencePitch reference(staging.release_position_m,staging.reference_velocity_mps,staging.strike_zone_plane_z());
    const auto prediction=predict_arrival(reference);reference.release();std::vector<BallState> path{reference.initial};
    while(reference.phase!=PitchPhase::Complete) {reference.step_tick();path.push_back(reference.current);}
    const auto arrival=reference.arrival_at_plane();
    require(p.delivery.motion.release_tick==384 && p.delivery.motion.end_tick==816 && p.batter.end_tick==896,"accepted timeline changed");
    p.advance(1'000'000'000);p.single_step();require(!p.tick && p.phase==PreviewPhase::Ready,"Ready advanced");
    std::vector<Sample> history{sample(p)};require(p.start()&&!p.start(),"start guard");p.toggle_pause();history[0]=sample(p); // Replay begins Playing at tick zero, not Ready.
    for(std::uint64_t tick=1;tick<=896;++tick) {
        if(tick==284||tick==384||tick==479||tick==480||tick==817) frozen(p);
        const bool complete=p.single_step();require(p.tick==tick && complete==(tick==896),"single-step/complete mismatch");
        require(p.delivery.tick==std::min(tick,std::uint64_t{816}) && p.delivery.motion.tick==std::min(tick,std::uint64_t{816})
            && p.batter.tick==tick,"actors not on shared tick");
        require(!p.delivery.pending_ticks&&!p.delivery.motion.pending_ticks&&!p.delivery.pitch.pending_ticks
            && !p.delivery.paused&&!p.delivery.motion.paused&&!p.delivery.pitch.paused,"child consumed clock/debt/pause");
        if(tick<384) require(p.delivery.ball_owner==BallOwner::Hand&&!p.delivery.release_count&&!p.delivery.pitch.tick,"pre-release changed");
        else {
            const auto pt=std::min(tick-384,std::uint64_t{95});
            require(p.delivery.ball_owner==BallOwner::Simulation && p.delivery.release_count==1 && p.delivery.pitch.tick==pt
                && same(p.delivery.pitch.current,path[pt]),"physics/handoff differs from reference");
        }
        if(tick==479) {
            const auto a=p.delivery.pitch.arrival_at_plane();require(a.tick==arrival.tick&&a.time_s==arrival.time_s&&same(a.state,arrival.state),"gameplay result delayed/changed");
            require(p.arrival_tick==479 && p.batter.contact_area_tick-p.arrival_tick==1 && !p.start(),"arrival / preview replay guard");
        }
        if(tick>=816) require(p.delivery.phase==DeliveryPhase::Complete,"PitchDelivery completion extended");
        if(tick==817) require(p.batter.pose.world!=history.back().batter_pose&&p.delivery.motion.pose.world==history.back().pitcher_pose,"batter finish stopped with pitcher");
        if(tick<896) require(p.phase==PreviewPhase::Playing,"premature preview completion");
        history.push_back(sample(p));
    }
    const auto final=sample(p);p.advance(1'000'000'000);p.single_step();equal(p,final);
    for(unsigned fps:{30u,60u,120u}) {
        p.start();std::uint64_t previous=0;
        for(unsigned frame=1;frame<=fps*4;++frame) {
            const std::uint64_t now=1'000'000'000ull*frame/fps;p.advance(now-previous);previous=now;
            const auto tick=std::min<std::uint64_t>(now*240/1'000'000'000,896);require(p.tick==tick,"chunking drift");equal(p,history[tick]);
        }
    }
    for(unsigned replay=0;replay<20;++replay) {
        require(p.start()&&!p.start()&&!p.pending_ticks&&!p.arrival_tick,"replay reset");equal(p,history[0]);
        p.advance(4'166'666);require(p.tick==0,"fractional replay debt");p.advance(1);require(p.tick==1,"fresh fractional credit");
        p.advance(10'000'000'000);require(p.tick==17&&p.pending_ticks>0,"bounded catch-up debt");p.toggle_pause();frozen(p);
        while(p.phase!=PreviewPhase::Complete) {p.single_step();equal(p,history[p.tick]);}
        require(!p.pending_ticks&&p.delivery.release_count==1,"final debt/one-shot");
    }
    require(same(p.delivery.pitch.arrival_at_plane().state,prediction.state),"prediction changed");
    std::printf("PASS BattingPreview: single 240 Hz clock; release384 arrival479 contact_area480 delivery816 batter/preview896; 30/60/120 chunking, 20 deterministic replays, pause/step/debt, unchanged raw Complete ball and final pitcher.\n");
    return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}
}
