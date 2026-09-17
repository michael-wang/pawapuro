#include "manual_swing.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace pawapuro;
void require(bool x,const char* why){if(!x)throw std::runtime_error(why);}
void until(ManualSwingPreview& p,std::uint64_t target){while(p.tick<target){p.advance(4'166'667);require(p.phase==PreviewPhase::Playing||p.tick==target,"unexpected completion");}}
int main(int argc,char**argv){try{
    require(argc==2,"expected directory");const std::filesystem::path d=argv[1];const auto s=load_batting_staging(d/"staging.toml");
    ManualSwingPreview p(d,s);const DirectX::XMFLOAT2 a{.1f,.8f},b{-.2f,.5f};
    p.input_boundary(true,true,true,a);require(!p.pending,"Ready must reject");p.start();
    p.input_boundary(false,false,true,a);p.input_boundary(true,true,true,a);require(!p.pending,"early input must reject without buffering");
    until(p,431);p.input_boundary(true,false,true,a);require(!p.pending,"held cannot trigger");
    p.input_boundary(false,false,true,a);p.input_boundary(true,true,true,a);require(p.pending&&p.pending->target_tick==432,"lower endpoint");
    p.input_boundary(false,false,true,b);p.input_boundary(true,true,true,b);require(p.pending->aim_center.x==a.x,"queued snapshot immutable");
    until(p,432);require(p.committed&&p.committed->consumed_tick==432,"consume at recorded tick");
    p.toggle_pause();const auto t=p.tick;p.advance(1'000'000'000);require(p.tick==t,"pause advances");p.single_step();require(p.tick==t+1&&p.committed,"step loses committed");
    p.lose_input();p.toggle_pause();p.input_boundary(true,true,true,b);require(p.committed->aim_center.x==a.x,"committed snapshot changed");
    while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);require(p.tick==888,"early finish truncated");
    p.start();require(!p.pending&&!p.committed,"reset leaks command");until(p,495);p.input_boundary(false,false,true,a);p.input_boundary(true,true,true,a);until(p,496);require(p.committed&&p.committed->consumed_tick==496,"upper endpoint");
    while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);require(p.tick==952,"late finish truncated");
    p.start();until(p,496);p.input_boundary(false,false,true,a);p.input_boundary(true,true,true,a);require(!p.pending,"out-of-domain target not rejected");
    while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);require(p.tick==816&&!p.committed,"Take completion");
    // Account old elapsed debt first; target after debt, never tick+1 while behind.
    p.start();until(p,420);p.advance(100'000'000);require(p.tick==436&&p.pending_ticks==8,"controlled catch-up setup");
    p.input_boundary(false,false,true,a);p.input_boundary(true,true,true,a);require(p.pending&&p.pending->target_tick==445&&p.pending->backlog==8,"backlog target");
    p.advance(0);require(p.tick==444&&!p.committed,"command must wait behind pre-boundary debt");
    p.advance(4'166'667);require(p.committed&&p.committed->consumed_tick==445&&p.committed->backlog==8,"consume scheduled backlog command");
    p.reset();p.start();until(p,431);p.input_boundary(false,false,true,a);p.input_boundary(true,true,true,a);
    p.lose_input();require(!p.pending,"focus/minimize must clear pending");p.input_boundary(true,true,true,a);require(!p.pending,"held after focus must release");
    p.input_boundary(false,false,true,a);p.input_boundary(true,true,true,a);p.toggle_pause();require(!p.pending,"pause must clear pending");
    p.start(); // Playing cannot reset a live attempt.
    require(p.paused,"start must preserve active attempt");
    p.reset();p.start();until(p,475);p.advance(100'000'000);
    require(p.tick==491&&p.pending_ticks==8,"outside-target backlog setup");
    p.input_boundary(false,false,true,a);p.input_boundary(true,true,true,a);
    require(!p.pending,"target 500 rejected even though current tick 491 is in domain");
    engine::GlbMatrix final{};
    for(unsigned hz:{30u,60u,120u}){
        p.reset();p.start();require(p.record_command(457,a),"recorded replay command");
        std::uint64_t elapsed=0;unsigned frame=0;
        while(p.phase==PreviewPhase::Playing){++frame;const auto next=static_cast<std::uint64_t>(frame)*1'000'000'000/hz;p.advance(next-elapsed);elapsed=next;}
        require(p.tick==913&&p.committed->consumed_tick==457,"cadence replay result");
        if(hz==30)final=p.batter.barrel_world;else require(final==p.batter.barrel_world,"cadence changes final pose");
    }
    p.reset();p.start();p.record_command(457,a);p.advance(3'000'000'000);while(p.pending_ticks)p.advance(0);
    while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);require(p.tick==913&&p.batter.barrel_world==final,"backlog replay result");
    std::cout<<"Manual input/state/snapshot/domain/completion/replay checks passed\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
