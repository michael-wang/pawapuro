#include "manual_swing.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace pawapuro;
void require(bool v,const char* message){if(!v)throw std::runtime_error(message);}
bool same(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b){return a.x==b.x&&a.y==b.y&&a.z==b.z;}
void until(ManualSwingPreview& p,std::uint64_t tick){while(p.tick<tick){p.advance(4'166'667);require(p.phase==PreviewPhase::Playing||p.tick==tick,"unexpected completion");}}
int main(int argc,char** argv){try {
    require(argc==2,"expected batting directory");const std::filesystem::path d=argv[1];
    const auto s=load_batting_staging(d/"staging.toml");require(s.normal_finish_ticks==240,"candidate finish changed");
    ManualSwingPreview p(d,s),baseline(d,s);engine::GlbPose old_pose,new_pose;
    for(auto mode:{SwingTempo::Original,SwingTempo::Compact}) {
        const SwingTempoTiming old{mode,30},candidate{mode,30,240};
        require(candidate.sample_tick(448+45.6,448)==old.sample_tick(448+45.6,448),"world boundary mapped differently");
        require(candidate.end_tick(448)==688,"finish not commit+240");
        double previous=candidate.sample_tick(448,448);
        for(unsigned i=1;i<=8000;++i){const double t=448+i*.03125,v=candidate.sample_tick(t,448);
            require(v>=previous&&v<=904,"tail not monotonic/bounded");previous=v;}
        for(double x:{45.6,240.})for(double direction:{-1.,1.}) {
            const double slope=(candidate.sample_tick(448+x+direction*1e-5,448)-candidate.sample_tick(448+x,448))/(direction*1e-5);
            require(std::abs(slope-(x==45.6?1:0))<1e-5,"retime velocity discontinuity");
        }
        for(std::uint64_t c:{1ull,96ull,432ull,448ull,456ull,496ull,600ull,816ull}) {
            // Exact mapping and every world-matrix channel, including grip/barrel/tip; fractional world samples.
            for(unsigned i=0;i<=456;++i){const double world=double(c)+i*.1;
                require(candidate.sample_tick(world,c)==old.sample_tick(world,c),"protected mapping changed");
                p.batter.sample(candidate.sample_tick(world,c),c,new_pose);p.batter.sample(old.sample_tick(world,c),c,old_pose);
                require(new_pose.world==old_pose.world&&new_pose.skin==old_pose.skin,"protected whole pose changed");
                const auto a=p.batter.sample_barrel(world/240,c,new_pose,candidate),b=p.batter.sample_barrel(world/240,c,old_pose,old);
                require(a.barrel_world==b.barrel_world&&same(a.barrel,b.barrel)&&same(a.tip,b.tip),"protected barrel changed");
            }
            const double boundary=double(c)+45.6;
            p.batter.sample(candidate.sample_tick(boundary,c),c,old_pose);
            p.batter.sample(candidate.sample_tick(boundary+1e-6,c),c,new_pose);
            for(size_t n=0;n<old_pose.world.size();++n)for(unsigned k=0;k<16;++k)
                require(std::abs(old_pose.world[n][k]-new_pose.world[n][k])<1e-5,"boundary pose jump");
            for(std::uint64_t x:{46ull,96ull,162ull,239ull,240ull,300ull}) {
                p.batter.evaluate_tick(c+x,c,candidate);
                const auto bat=p.batter.sample_barrel(double(c+x)/240,c,new_pose,candidate);
                for(size_t n=0;n<new_pose.world.size();++n)for(unsigned k=0;k<16;++k)
                    require(std::abs(new_pose.world[n][k]-p.batter.pose.world[n][k])<.0001,"render/semantic whole-pose mismatch");
                for(unsigned k=0;k<16;++k)require(std::abs(bat.barrel_world[k]-p.batter.barrel_world[k])<.0001,"render/semantic barrel mismatch");
                if(x>=240){p.batter.sample(double(c)+456,c,old_pose);require(old_pose.world==p.batter.pose.world,"final authored pose truncated");}
            }
        }
        std::cout<<(mode==SwingTempo::Original?"A":"B")<<" boundary_sample_offset="<<candidate.sample_tick(45.6,0)
            <<" sample_400ms="<<candidate.sample_tick(96,0)<<" sample_675ms="<<candidate.sample_tick(162,0)<<" finish="<<candidate.end_tick(0)<<'\n';
        for(std::uint64_t c:{96ull,432ull,433ull,448ull,456ull,460ull}) {
            p.reset();baseline.reset();p.next_tempo=baseline.next_tempo=mode;p.start();baseline.start();
            baseline.attempt_tempo.tail_finish_ticks=0; // Accepted 892e7b4 motion, no runtime switch.
            const auto pt=predict_arrival(p.delivery.pitch).state.position_m;const DirectX::XMFLOAT2 aim{pt.x,pt.y};
            p.record_command(c,aim);baseline.record_command(c,aim);
            until(p,c+46);until(baseline,c+46);
            require(p.timing()->state==baseline.timing()->state&&p.timing()->efficiency==baseline.timing()->efficiency&&p.timing()->offset_ms==baseline.timing()->offset_ms,"timing changed");
            require(p.authorization()->authorized==baseline.authorization()->authorized&&p.authorization()->q==baseline.authorization()->q,"spatial changed");
            require(p.geometry()==baseline.geometry()&&bool(p.contact())==bool(baseline.contact()),"geometry changed");
            if(p.contact()){const auto& a=*p.contact();const auto& b=*baseline.contact();
                require(a.sample.preview_time_s==b.sample.preview_time_s&&a.sample.approach.u==b.sample.approach.u&&same(a.normal,b.normal)&&same(a.relative_velocity,b.relative_velocity),"contact event changed");}
            if(mode==SwingTempo::Compact&&c==448)require(p.contact()&&std::abs(p.contact()->sample.preview_time_s-1.9913564701)<1e-10,"448 baseline event lost");
        }
    }
    p.reset();p.start();p.record_command(96,{});until(p,336);
    require(p.rearmed()&&p.phase==PreviewPhase::Playing&&p.delivery.ball_owner==BallOwner::Hand,"early finish stopped delivery/rearm");
    engine::GlbPose prep;p.batter.sample(336,std::nullopt,prep);require(prep.world==p.batter.pose.world,"rearm preparation mismatch");
    until(p,480);require(p.delivery.pitch.phase==PitchPhase::Complete,"continuing pitch changed");
    p.input_boundary(false,false,true,{});p.input_boundary(true,true,true,{});require(!p.pending&&p.committed()->consumed_tick==96,"post-exit swing enabled");
    engine::GlbPose cadence_pose;std::uint64_t final_tick=0;
    for(unsigned hz:{30u,60u,120u,0u}) {
        p.reset();p.start();p.record_command(448,{});
        if(!hz){p.advance(5'000'000'000);while(p.pending_ticks)p.advance(0);}
        while(p.phase==PreviewPhase::Playing)p.advance(hz?1'000'000'000/hz:16'666'667);
        require(p.tick==816&&p.batter.tick==688,"attempt used old long completion");
        if(hz==30){cadence_pose=p.batter.pose;final_tick=p.tick;}
        else require(p.tick==final_tick&&p.batter.pose.world==cadence_pose.world,"cadence changed final pose");
    }
    std::cout<<"PASS exact protected prefix/physical events; continuous monotonic tail; authored finish/hold; early delivery; cadence/backlog\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
