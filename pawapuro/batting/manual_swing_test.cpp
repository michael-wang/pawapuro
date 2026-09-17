#include "manual_swing.hpp"
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
using namespace pawapuro;
void require(bool x,const char* why){if(!x)throw std::runtime_error(why);}
void until(ManualSwingPreview& p,std::uint64_t target){while(p.tick<target){p.advance(4'166'667);require(p.phase==PreviewPhase::Playing||p.tick==target,"unexpected completion");}}
int main(int argc,char**argv){try{
    require(argc==2,"expected directory");const std::filesystem::path d=argv[1];const auto s=load_batting_staging(d/"staging.toml");
    ManualSwingPreview p(d,s);p.next_tempo=SwingTempo::Original; // Explicit A regression baseline.
    const DirectX::XMFLOAT2 a{.1f,.8f},b{-.2f,.5f};
    p.input_boundary(true,true,true,a);require(!p.pending,"Ready must reject");p.start();
    p.input_boundary(false,false,true,a);p.input_boundary(true,true,true,a);require(!p.pending,"early input must reject without buffering");
    until(p,431);p.input_boundary(true,false,true,a);require(!p.pending,"held cannot trigger");
    p.input_boundary(false,false,true,a);p.input_boundary(true,true,true,a);require(p.pending&&p.pending->target_tick==432,"lower endpoint");
    p.input_boundary(false,false,true,b);p.input_boundary(true,true,true,b);require(p.pending->aim_center.x==a.x,"queued snapshot immutable");
    until(p,432);require(p.committed&&p.committed->consumed_tick==432,"consume at recorded tick");
    p.toggle_pause();const auto paused_tick=p.tick;p.advance(1'000'000'000);require(p.tick==paused_tick,"pause advances");p.single_step();require(p.tick==paused_tick+1&&p.committed,"step loses committed");
    p.lose_input();p.toggle_pause();p.input_boundary(true,true,true,b);require(p.committed->aim_center.x==a.x,"committed snapshot changed");
    while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);require(p.tick==888,"early finish truncated");
    p.start();require(!p.pending&&!p.committed&&!p.contact&&!p.contact_count&&p.geometry==ManualGeometry::Pending,"reset leaks command/result");until(p,495);p.input_boundary(false,false,true,a);p.input_boundary(true,true,true,a);until(p,496);require(p.committed&&p.committed->consumed_tick==496,"upper endpoint");
    while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);require(p.tick==952,"late finish truncated");
    p.start();until(p,496);p.input_boundary(false,false,true,a);p.input_boundary(true,true,true,a);require(!p.pending,"out-of-domain target not rejected");
    while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);require(p.tick==816&&!p.committed&&!p.contact&&p.geometry==ManualGeometry::NoSwing,"Take completion");
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
    engine::GlbMatrix final{};std::optional<BatContact> replay_contact;
    for(unsigned hz:{30u,60u,120u}){
        p.reset();p.start();require(p.record_command(457,a),"recorded replay command");
        std::uint64_t elapsed=0;unsigned frame=0;
        while(p.phase==PreviewPhase::Playing){++frame;const auto next=static_cast<std::uint64_t>(frame)*1'000'000'000/hz;p.advance(next-elapsed);elapsed=next;}
        require(p.tick==913&&p.committed->consumed_tick==457,"cadence replay result");
        if(hz==30){final=p.batter.barrel_world;replay_contact=p.contact;}
        else {require(final==p.batter.barrel_world,"cadence changes final pose");
            require(bool(replay_contact)==bool(p.contact),"cadence changes contact presence");
            if(p.contact)require(p.contact->sample.preview_time_s==replay_contact->sample.preview_time_s,"cadence changes contact time");}
    }
    p.reset();p.start();p.record_command(457,a);p.advance(3'000'000'000);while(p.pending_ticks)p.advance(0);
    while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);require(p.tick==913&&p.batter.barrel_world==final,"backlog replay result");
    // Independent saved .blend semantic samples, including fractional entry/support times.
    engine::GlbPose scratch;
    std::ifstream fixtures(d/"batter/ingame_s0/motion_expected.txt");require(bool(fixtures),"missing saved-source fixtures");
    std::string line;unsigned source_checks=0;double source_error=0;
    while(std::getline(fixtures,line)) {
        if(line.empty()||line[0]=='#')continue;
        std::istringstream row(line);std::string name,bone;std::uint64_t c;double t;row>>name>>c>>t>>bone;
        if(!c || (bone!="bat_barrel"&&bone!="bat_tip"))continue;
        engine::GlbMatrix expected;for(auto& v:expected)row>>v;require(bool(row),"bad fixture");
        const auto actual=p.batter.sample_barrel(t/240,c,scratch);
        if(bone=="bat_barrel") {
            auto world=expected;
            for(unsigned col=0;col<4;++col)for(unsigned index=0;index<4;++index)
                world[col*4+index]*=(col==0?-1.f:1.f)*(index==0?-1.f:1.f);
            world[12]+=s.batter_blockout_position_m.x;world[13]+=s.batter_blockout_position_m.y;world[14]+=s.batter_blockout_position_m.z;
            for(unsigned k=0;k<16;++k)require(std::abs(world[k]-actual.barrel_world[k])<=0.0001f,"saved-source rigid barrel matrix differs");
        }
        const auto point=bone=="bat_barrel"?actual.barrel:actual.tip;
        source_error=std::max({source_error,double(std::abs(point.x-(-expected[12]+s.batter_blockout_position_m.x))),
            double(std::abs(point.y-(expected[13]+s.batter_blockout_position_m.y))),double(std::abs(point.z-(expected[14]+s.batter_blockout_position_m.z)))});++source_checks;
    }
    require(source_checks>0&&source_error<=0.0001,"manual sampler differs from saved source");
    std::cout<<"Saved-source bat samples="<<source_checks<<" max_error_m="<<source_error<<"\n";
    const double release=double(p.delivery.motion.release_tick)/pitch_hz;
    unsigned contacts=0;
    for(std::uint64_t c=432;c<=496;++c) {
        std::optional<BatContact> expected;
        double closest=1e9,closest_tick=0;
        for(unsigned refinement:{32u,64u,128u}) {
            std::optional<BatContact> found;
            for(std::uint64_t t=c;t<c+manual_contact_window_ticks;++t) {
                if(!found)found=first_manual_contact(p.delivery.pitch.initial,release,p.batter,c,s.bat_contact,double(t)/240,double(t+1)/240,scratch,refinement);
                if(refinement==64)for(unsigned k=0;k<=8;++k) {
                    const auto sample=sample_manual_contact(p.delivery.pitch.initial,release,p.batter,c,(double(t)+k/8.)/240,scratch);
                    if(sample.approach.distance_m<closest){closest=sample.approach.distance_m;closest_tick=sample.preview_time_s*240;}
                }
            }
            if(refinement==32)expected=found;
            else {
                require(bool(found)==bool(expected),"refinement changes contact presence");
                if(found)require(std::abs(found->sample.preview_time_s-expected->sample.preview_time_s)<1e-7,"refinement changes entry time");
            }
        }
        // Render semantics match the sampler; queries must leave the displayed pose untouched.
        p.batter.evaluate_tick(c+24,c);const auto saved=p.batter.pose.world;
        const auto sampled=p.batter.sample_barrel(double(c+24)/240,c,scratch);
        require(sampled.barrel_world==p.batter.barrel_world,"sampler/render semantic mismatch");
        p.batter.sample_barrel((c+23.5)/240,c,scratch);require(saved==p.batter.pose.world,"sample mutates displayed pose");
        p.reset();p.start();p.record_command(c,a);p.toggle_pause();
        // Pause cancels pending by design, so replay the recorded command before pausing after consumption.
        p.toggle_pause();p.record_command(c,a);until(p,c);p.toggle_pause();
        while(p.tick<c+manual_contact_window_ticks) {
            p.advance(100'000'000);const auto before=p.tick;p.single_step();require(p.tick==before+1,"contact step clock");
            if(p.contact)require(p.contact->fractional_tick<=double(p.tick)+1e-9,"event dispatched early");
        }
        require(bool(p.contact)==bool(expected)&&p.contact_count==(expected?1u:0u),"runtime query/result count mismatch");
        require(p.geometry==(expected?ManualGeometry::Contact:ManualGeometry::NoContactInWindow),"pending result after window");
        if(expected){++contacts;require(std::abs(p.contact->sample.preview_time_s-expected->sample.preview_time_s)<1e-7,"runtime entry differs");}
        for(unsigned k=0;k<4;++k)p.single_step();require(p.contact_count==(expected?1u:0u),"duplicate event");
        std::cout<<"GEOMETRY commit="<<c<<" result="<<p.contact_state()<<" contact_tick="<<(expected?expected->fractional_tick:0)
            <<" closest_grid_m="<<closest<<" closest_tick="<<closest_tick<<" query=["<<c<<","<<c+manual_contact_window_ticks<<"]\n";
    }
    // Contact and no-contact replay, including interval split/initial overlap boundaries.
    for(std::uint64_t c:{432ull,442ull,443ull,496ull}) {
        std::optional<BatContact> reference;std::uint64_t dispatch=0;
        for(unsigned hz:{30u,60u,120u,0u}) {
            p.reset();p.start();p.record_command(c,b);
            std::uint64_t elapsed=0;unsigned frame=0;
            if(!hz){p.advance(3'000'000'000);while(p.pending_ticks)p.advance(0);}
            while(p.phase==PreviewPhase::Playing) {
                const auto next=static_cast<std::uint64_t>(++frame)*1'000'000'000/(hz?hz:60);
                p.advance(next-elapsed);elapsed=next;
            }
            if(hz==30){reference=p.contact;dispatch=p.contact_dispatch_tick;}
            else {require(bool(reference)==bool(p.contact)&&dispatch==p.contact_dispatch_tick,"geometry replay dispatch differs");
                if(reference)require(reference->sample.preview_time_s==p.contact->sample.preview_time_s,"geometry replay time differs");}
        }
        if(reference) {
            const double event=reference->sample.preview_time_s,lo=std::floor(event*240)/240,hi=lo+1./240,mid=(lo+hi)/2;
            auto split=first_manual_contact(p.delivery.pitch.initial,release,p.batter,c,s.bat_contact,lo,mid,scratch);
            if(!split)split=first_manual_contact(p.delivery.pitch.initial,release,p.batter,c,s.bat_contact,mid,hi,scratch);
            require(split&&std::abs(split->sample.preview_time_s-event)<1e-7,"interval split lost earliest entry");
            auto overlap=first_manual_contact(p.delivery.pitch.initial,release,p.batter,c,s.bat_contact,event,event+0.00001,scratch);
            require(overlap&&overlap->sample.preview_time_s==event,"inside boundary not reported at start");
        }
    }
    std::cout<<"GEOMETRY contacts="<<contacts<<"/65; refinements 32/64/128 agree\n";
    std::cout<<"Manual input/state/snapshot/domain/completion/replay checks passed\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
