#include "manual_swing.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
using namespace pawapuro;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void until(ManualSwingPreview& p,std::uint64_t t){while(p.tick<t){p.advance(4'166'667);require(p.phase==PreviewPhase::Playing||p.tick==t,"unexpected completion");}}
bool same(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b){return a.x==b.x&&a.y==b.y&&a.z==b.z;}
int main(int argc,char** argv){try {
    require(argc==2,"expected batting directory");const std::filesystem::path d=argv[1];
    const auto s=load_batting_staging(d/"staging.toml");const auto curve=s.swing_phase_potential;
    require(curve.normal_start_ms==75&&curve.normal_peak_ms==125&&curve.normal_end_ms==190,"candidate potential values changed");
    require(normal_swing_potential(-1,curve)==0&&normal_swing_potential(.075,curve)==0
        &&normal_swing_potential(.125,curve)==1&&normal_swing_potential(.190,curve)==0&&normal_swing_potential(10,curve)==0,"potential endpoints");
    double previous=0;
    for(unsigned i=0;i<=1000;++i){const double v=normal_swing_potential(.075+.050*i/1000,curve);
        require(v>=previous&&v<=1&&std::isfinite(v),"rise monotonic");previous=v;}
    for(unsigned i=0;i<=1000;++i){const double v=normal_swing_potential(.125+.065*i/1000,curve);
        require(v<=previous&&v>=0&&std::isfinite(v),"fall monotonic");previous=v;}
    ManualSwingPreview p(d,s);const auto ball=p.ball_passage;
    const double release=double(p.delivery.motion.release_tick)/pitch_hz;
    require(s.batting_interaction.half_depth_m==.4f&&s.bat_contact.ball_radius_m==.037f&&s.bat_contact.bat_radius_m==.033f,"candidate region/radii changed");
    const auto enter=sample_reference_pitch(p.delivery.pitch.initial,ball.enter_s-release),exit=sample_reference_pitch(p.delivery.pitch.initial,ball.exit_s-release);
    require(std::abs(enter.position_m.z-(s.strike_zone_plane_z()+.4f))<1e-6
        &&std::abs(exit.position_m.z-(s.strike_zone_plane_z()-.4f))<1e-6,"slab not centered on gameplay plane");
    require(std::abs((ball.exit_s-ball.enter_s)-.8/-p.delivery.pitch.initial.velocity_mps.z)<1e-8,"slab duration");
    auto moved=s;moved.release_position_m.x=20;moved.release_position_m.y=-20;moved.reference_velocity_mps.x=-30;moved.reference_velocity_mps.y=80;
    const ReferencePitch other(moved.release_position_m,moved.reference_velocity_mps,moved.strike_zone_plane_z());
    const auto xy=batting_interaction_passage(other,release,moved.batting_interaction);
    require(xy.enter_s==ball.enter_s&&xy.exit_s==ball.exit_s,"region gated X/Y");
    auto slow=p.delivery.pitch.initial.velocity_mps;slow.z*=.5f;
    const auto slower=batting_interaction_passage(ReferencePitch(p.delivery.pitch.initial.position_m,slow,s.strike_zone_plane_z()),release,s.batting_interaction);
    require(std::abs((slower.exit_s-slower.enter_s)-2*(ball.exit_s-ball.enter_s))<1e-12,"region authored in ticks instead of trajectory");
    const auto predicted=predict_arrival(p.delivery.pitch).state.position_m;
    const DirectX::XMFLOAT2 aim{predicted.x,predicted.y};
    double best=0;
    std::cout<<std::setprecision(12)<<"BALL region_s=["<<ball.enter_s<<','<<ball.exit_s<<"] duration_ms="<<(ball.exit_s-ball.enter_s)*1000<<" reference_s="<<ball.reference_s<<'\n';
    for(std::uint64_t c:{96ull,433ull,448ull,460ull,600ull}) {
        const auto timing=timing_interaction(double(c)/240,ball,curve);
        require(timing.efficiency>=0&&timing.efficiency<=1&&std::isfinite(timing.efficiency),"efficiency range");
        require((c<448?timing.offset_ms<0:c>448?timing.offset_ms>0:true),"signed timing offset");
        if(c==96||c==600)require(!timing.overlap&&timing.efficiency==0,"extreme swing overlaps");
        else require(timing.overlap&&timing.efficiency>0,"partial/peak overlap lost");
        if(c==448)best=timing.efficiency;
        else if(c==460)require(best>timing.efficiency&&best>timing_interaction(433./240,ball,curve).efficiency,"central fixture not strongest");
        // Independent deterministic numerical check of the polynomial integral.
        double sum=0;constexpr unsigned n=10000;
        for(unsigned k=0;k<n;++k)sum+=normal_swing_potential(ball.enter_s+(ball.exit_s-ball.enter_s)*(k+.5)/n-double(c)/240,curve);
        require(std::abs(sum/n-timing.efficiency)<1e-7,"potential integral mismatch");
        if(c==600){p.reset();p.start();require(!p.record_command(c,aim),"post-passage lifecycle accepted");continue;} // Pure late potential above remains covered.
        for(unsigned hz:{30u,60u,120u,0u}) {
            p.reset();p.start();require(p.record_command(c,aim)&&!p.timing(),"queued timing computed early");
            if(!hz){p.advance(5'000'000'000);while(p.pending_ticks)p.advance(0);}
            while(p.phase==PreviewPhase::Playing)p.advance(hz?1'000'000'000/hz:16'666'667);
            require(p.committed()->consumed_tick==c&&p.timing()->efficiency==timing.efficiency&&p.timing()->offset_ms==timing.offset_ms,"cadence changes decision");
            require(p.tick==p.completion_tick(),"completion truncates pitcher or batter");
            if(!timing.overlap)require(p.contact_query_count()==0&&!p.searched_interval()&&!p.contact()&&p.geometry()==ManualGeometry::NoContactInWindow,"empty overlap queried ghost pitch");
            else {
                require(p.contact_query_count()>0&&p.searched_interval(),"overlap never searched");
                require(p.searched_interval()->start_s>=timing.overlap->start_s&&p.searched_interval()->end_s<=timing.overlap->end_s,"query escaped intersection");
                require(std::abs(p.searched_interval()->start_s-timing.overlap->start_s)<1e-14,"query skipped beginning");
                if(!p.contact())require(std::abs(p.searched_interval()->end_s-timing.overlap->end_s)<1e-14,"query stopped before end");
                if(p.contact())require(normal_swing_potential(p.contact()->sample.preview_time_s-double(c)/240,curve)>0,"contact at S=0");
            }
        }
        char info[640];p.timing_diagnostic(info,sizeof(info));std::cout<<"FIXTURE commit="<<c<<' '<<info<<" Geometry="<<p.contact_state()<<" queries="<<p.contact_query_count()<<'\n';
    }
    // Fresh early J, no buffer/clamp; pause/focus/rearm and one swing remain input rules.
    p.reset();p.start();p.input_boundary(false,false,true,aim);p.input_boundary(true,true,true,aim);
    require(p.pending&&p.pending->target_tick==1,"early J rejected/buffered");until(p,1);
    require(p.committed()->consumed_tick==1&&p.tick<p.delivery.motion.release_tick,"early commit not before release");
    p.input_boundary(false,false,true,aim);p.input_boundary(true,true,true,aim);require(!p.pending&&p.committed()->consumed_tick==1,"second swing accepted");
    until(p,100);require(p.delivery.ball_owner==BallOwner::Hand&&p.batter.tick==100&&p.contact_query_count()==0,"early swing stopped pitcher or queried negative flight");
    const auto saved=*p.timing();const auto query_count=p.contact_query_count();p.toggle_pause();p.advance(1'000'000'000);
    require(p.tick==100&&p.timing()->efficiency==saved.efficiency,"pause mutated timing");p.single_step();
    require(p.tick==101&&p.timing()->offset_ms==saved.offset_ms&&p.contact_query_count()==query_count,"step recomputed timing");
    p.reset();p.start();p.input_boundary(false,false,true,aim);p.input_boundary(true,true,true,aim);p.lose_input();
    p.input_boundary(true,true,true,aim);require(!p.pending,"focus loss rearmed held key");
    p.input_boundary(false,false,true,aim);p.toggle_pause();p.input_boundary(true,true,true,aim);require(!p.pending,"paused J accepted");
    p.reset();p.start();until(p,600);require(p.geometry()==ManualGeometry::Pending&&!p.timing(),"NoSwing finalized before completion");
    p.input_boundary(false,false,true,aim);p.input_boundary(true,true,true,aim);
    require(!p.pending&&!p.committed(),"post-passage input accepted");
    until(p,816);require(p.geometry()==ManualGeometry::NoSwing,"Take completion");
    // Debt closes intent by its target time, even when the current tick precedes exit.
    p.reset();p.start();until(p,450);p.advance(150'000'000);
    require(p.tick==466&&p.pending_ticks==20,"cutoff debt fixture");
    p.input_boundary(false,false,true,aim);p.input_boundary(true,true,true,aim);
    require(!p.pending&&!p.committed(),"backlogged post-exit target accepted");
    // Display shares the exact analytic contact trajectory throughout the slab, then freezes at its exit.
    p.reset();p.start();until(p,475);
    while(p.tick<483){p.advance(4'166'667);const double time=double(p.tick)/240;
        if(time>=ball.enter_s)require(same(p.displayed_ball_center(),sample_reference_pitch(p.delivery.pitch.initial,std::min(time,ball.exit_s)-release).position_m),"visible ball differs from slab geometry");}
    const auto frozen=p.displayed_ball_center();until(p,600);require(same(frozen,p.displayed_ball_center()),"display did not stop at slab exit");
    require(std::abs(frozen.z-(s.strike_zone_plane_z()-s.batting_interaction.half_depth_m))<1e-6,"wrong display freeze plane");
    std::cout<<"PASS timing interaction, early/late input, whole attempt lifetime, bounded query and visible analytic slab\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
