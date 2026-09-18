#include "manual_swing.hpp"
#include "bat_contact_detail.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace pawapuro;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void until(ManualSwingPreview& p,std::uint64_t t){while(p.tick<t)p.advance(4'166'667);}
int main(int argc,char** argv){try {
    require(argc==2||(argc==3&&std::string(argv[2])=="--core"),"expected batting directory [--core]");const std::filesystem::path d=argv[1];
    const auto s=load_batting_staging(d/"staging.toml");ManualSwingPreview p(d,s);
    const SwingTempoTiming b{SwingTempo::Compact,30},a{};
    require(p.next_tempo==SwingTempo::Compact&&a.mode==SwingTempo::Original,"entry default / generic baseline");
    p.start();require(p.attempt_tempo.mode==SwingTempo::Compact&&p.attempt_tempo.compact_area_ticks==30,"first attempt snapshot");
    p.reset();require(p.toggle_tempo()&&p.next_tempo==SwingTempo::Original,"Ready switch to A");
    p.start();require(p.attempt_tempo.mode==SwingTempo::Original,"selected A snapshot");p.reset();
    require(b.sample_tick(462,432)==472&&b.end_tick(432)==878&&a.end_tick(432)==888,"area/finish timing");
    for(double x=-1;x<500;x+=.03125) {
        require(b.sample_tick(432+x+.01,432)>b.sample_tick(432+x,432),"mapping not monotonic");
        require(b.sample_tick(432+x,std::nullopt)==432+x,"Take changed");
        require(a.sample_tick(432+x,432)==432+x,"A changed");
    }
    for(double x:{0.,30.})for(double sign:{-1.,1.}) {
        const double derivative=(b.sample_tick(432+x+sign*1e-4,432)-b.sample_tick(432+x,432))/(sign*1e-4);
        require(std::abs(derivative-1)<1e-5,"boundary speed discontinuity");
    }
    std::cout<<"TEMPO B sweep="<<b.world_offset(24)<<" area="<<b.world_offset(40)<<" finish="<<b.world_offset(456)<<" ticks\n";
    engine::GlbPose raw,scratch;double max_pose_error=0;
    for(std::uint64_t c:{432ull,456ull,496ull}) {
        for(double x:{-.5,0.,.125,1.,12.5,24.,29.5,30.,31.,446.}) {
            const double world=double(c)+x,mapped=b.sample_tick(world,c);
            p.batter.sample(mapped,c,raw);
            const auto bat=p.batter.sample_barrel(world/240,c,scratch,b);
            // seconds -> ticks can round at sub-frame boundaries; retain the existing 0.1 mm guard.
            double pose_error=0;
            for(size_t bone=0;bone<raw.world.size();++bone)for(unsigned k=0;k<16;++k)
                pose_error=std::max(pose_error,double(std::abs(raw.world[bone][k]-scratch.world[bone][k])));
            require(pose_error<=.0001,"fractional body/support/attachment differs from original at mapped time");
            max_pose_error=std::max(max_pose_error,pose_error);
            const auto source_bat=p.batter.sample_barrel(mapped/240,c,raw);
            for(unsigned k=0;k<16;++k)require(std::abs(source_bat.barrel_world[k]-bat.barrel_world[k])<1e-5,"fractional semantic mapping");
            if(x>=0&&std::floor(x)==x) {
                p.batter.evaluate_tick(c+static_cast<std::uint64_t>(x),c,b);
                for(size_t bone=0;bone<scratch.world.size();++bone)for(unsigned k=0;k<16;++k)
                    require(std::abs(p.batter.pose.world[bone][k]-scratch.world[bone][k])<=.0001,"render/sample pose mismatch");
                for(unsigned k=0;k<16;++k)require(std::abs(p.batter.barrel_world[k]-bat.barrel_world[k])<=.0001,"render/sample semantics mismatch");
            }
        }
    }
    std::cout<<"Mapped whole-pose max error="<<max_pose_error<<"\n";
    require(p.toggle_tempo()&&p.next_tempo==SwingTempo::Compact,"Ready toggle");p.start();
    require(!p.toggle_tempo()&&p.attempt_tempo.mode==SwingTempo::Compact,"live switch accepted");
    p.input_boundary(false,false,true,{});require(p.swing_available(),"live input availability");
    p.input_boundary(true,true,true,{});require(p.pending&&p.pending->target_tick==1,"early edge must queue");p.lose_input();
    until(p,431);p.input_boundary(false,false,true,{});p.input_boundary(true,true,true,{});
    require(bool(p.pending),"accepted command did not clear hint");
    until(p,432);require(p.committed()->tempo.mode==SwingTempo::Compact&&p.committed()->consumed_tick==432,"tempo snapshot");
    p.toggle_pause();require(!p.toggle_tempo(),"paused switch");p.toggle_pause();
    while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);
    const auto old_pose=p.batter.pose.world;const auto old_result=p.geometry();
    require(p.tick==816&&p.toggle_tempo()&&p.next_tempo==SwingTempo::Original,"Complete next switch");
    require(p.attempt_tempo.mode==SwingTempo::Compact&&p.batter.pose.world==old_pose&&p.geometry()==old_result,"next selection relabelled last attempt");
    p.toggle_tempo();p.reset();require(p.next_tempo==SwingTempo::Compact,"reset mode/hint");
    if(argc==3) {
        p.start();require(p.attempt_tempo.mode==SwingTempo::Compact,"next attempt lost selected B");
        std::cout<<"Core timing/default/selection/snapshot checks passed; contact study skipped\n";return 0;
    }
    const double release=double(p.delivery.motion.release_tick)/240;
    std::array<std::optional<BatContact>,65> results;
    unsigned contacts=0;
    for(std::uint64_t c=432;c<=496;++c) {
        p.reset();p.start();p.record_command(c,{});until(p,c+64);
        require((c>480||p.geometry()!=ManualGeometry::Pending)&&p.contact_count()==(p.contact()?1u:0u),"window incomplete");
        // Keep the old unrestricted geometry sweep as an independent physical/motion regression.
        for(auto t=c;t<c+64&&!results[c-432];++t)results[c-432]=first_manual_contact(p.delivery.pitch.initial,release,p.batter,c,s.bat_contact,double(t)/240,double(t+1)/240,scratch,64,b);
        if(results[c-432])++contacts;
        std::cout<<"B commit="<<c<<" result="<<p.contact_state()<<" contact_tick="<<(p.contact()?p.contact()->fractional_tick:0)<<"\n";
    }
    // Refine endpoints and both sides of every contact/no-contact transition.
    for(std::uint64_t c=432;c<=496;++c) {
        if(c!=432&&c!=496&&bool(results[c-432])==bool(results[c-433])&&bool(results[c-432])==bool(results[c-431]))continue;
        for(unsigned n:{32u,128u}) {
            std::optional<BatContact> event;
            for(auto t=c;t<c+64&&!event;++t)event=first_manual_contact(p.delivery.pitch.initial,release,p.batter,c,s.bat_contact,double(t)/240,double(t+1)/240,scratch,n,b);
            require(bool(event)==bool(results[c-432]),"refinement presence differs");
            if(event){const auto& baseline=*results[c-432];
                // Compare velocity wiring against the original unwrapped evaluator at mapped t +/- epsilon.
                auto raw_bat=[&](double t){return p.batter.sample_barrel(b.sample_tick(t*240,c)/240,c,scratch);};
                const auto velocity_reference=contact_detail::result(raw_bat,baseline.sample,s.bat_contact);
                require(std::abs(velocity_reference.relative_velocity.x-baseline.relative_velocity.x)<.02&&std::abs(velocity_reference.relative_velocity.y-baseline.relative_velocity.y)<.02&&std::abs(velocity_reference.relative_velocity.z-baseline.relative_velocity.z)<.02,"velocity did not use mapped world samples");
                require(std::abs(event->sample.preview_time_s-baseline.sample.preview_time_s)<1e-7,"refinement time differs");
                require(std::abs(event->relative_velocity.x-baseline.relative_velocity.x)<.02&&std::abs(event->relative_velocity.y-baseline.relative_velocity.y)<.02&&std::abs(event->relative_velocity.z-baseline.relative_velocity.z)<.02,"refinement velocity differs");}
        }
    }
    // Same recorded tick, immutable tempo and aim under elapsed chunking / backlog.
    for(std::uint64_t c:{432ull,446ull,456ull,496ull}) {
        engine::GlbMatrix final{};std::optional<BatContact> event;std::uint64_t dispatch=0;
        for(unsigned hz:{30u,60u,120u,0u}) {
            p.reset();p.start();p.record_command(c,{.1f,.8f});
            if(!hz){p.advance(3'000'000'000);while(p.pending_ticks)p.advance(0);}
            while(p.phase==PreviewPhase::Playing)p.advance(hz?1'000'000'000/hz:16'666'667);
            require(p.tick==p.completion_tick(),"B finish truncated");
            if(hz==30){final=p.batter.barrel_world;event=p.contact();dispatch=p.contact_dispatch_tick();}
            else {require(final==p.batter.barrel_world&&bool(event)==bool(p.contact())&&dispatch==p.contact_dispatch_tick(),"replay changed result");
                if(event)require(event->sample.preview_time_s==p.contact()->sample.preview_time_s&&event->relative_velocity.x==p.contact()->relative_velocity.x,"replay changed contact");}
        }
    }
    p.reset();p.start();while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);
    require(p.tick==816&&p.geometry()==ManualGeometry::NoSwing,"B Take changed");
    require(contacts==17,"historical Compact geometry regression changed");
    std::cout<<"B contacts="<<contacts<<"/65; boundary refinement/replay/entry checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
