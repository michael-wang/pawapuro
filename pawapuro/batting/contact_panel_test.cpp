#include "contact_panel.hpp"
#include "review_fixture.hpp"
#include "ground_projection.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
using namespace pawapuro;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void ground_response_checks(ManualSwingPreview& p) {
    const auto same=[](auto a,auto b){return a.x==b.x&&a.y==b.y&&a.z==b.z;};
    const auto distance=[](auto a,auto b){return std::hypot(double(a.x)-b.x,double(a.z)-b.z);};
    double high=0,low=0;
    for(float ey:{-.75f,-.5f,-1.f,0.f,.25f,.5f,.75f}) {
        std::string control;
        for(unsigned fps:{30u,60u,120u,1u}) {
            p.reset();p.start();const BattingReviewFixture fixture{448,0,ey};
            require(p.record_command(448,fixture.aim_center(p)),"ground fixture intent");
            while(p.phase==PreviewPhase::Playing)p.advance(1'000'000'000ull/fps);
            require(p.flight.has_value(),"ground fixture Contact");fixture.verify(*p.latest());
            const auto f=*p.flight;const auto r=*p.latest()->response;
            std::ostringstream signature;signature<<std::setprecision(17)<<p.tick<<','<<f.ground_s<<','<<f.second_ground_s<<','<<f.stop_s;
            for(double t:{f.start_s,f.ground_s,(f.ground_s+f.second_ground_s)/2,f.second_ground_s,(f.second_ground_s+f.stop_s)/2,f.stop_s,f.stop_s+10}) {
                const auto b=f.sample(t);signature<<','<<b.position_m.x<<','<<b.position_m.y<<','<<b.position_m.z<<','<<b.velocity_mps.x<<','<<b.velocity_mps.y<<','<<b.velocity_mps.z;
            }
            if(control.empty())control=signature.str();else require(control==signature.str(),"30/60/120/backlog piecewise determinism");
            const double g=-double(earth_gravity_mps2);
            const double legacy_ground=f.start_s+(double(r.launch_velocity_mps.y)+std::sqrt(double(r.launch_velocity_mps.y)*r.launch_velocity_mps.y+2*g*std::max(0.,double(r.launch_position_m.y)-f.ground_height_m)))/g;
            require(f.ground_s==legacy_ground,"first impact time changed");
            if(r.launch_velocity_mps.y>=0) {
                require(f.stop_s==legacy_ground,"airborne acquired ground response");
                for(unsigned i=0;i<=100;++i) {
                    const double time=f.start_s+(legacy_ground-f.start_s)*i/80.;
                    auto old=sample_reference_pitch(f.initial,std::clamp(time,f.start_s,legacy_ground)-f.start_s);
                    if(time>=legacy_ground){old.position_m.y=f.ground_height_m;old.velocity_mps={};}
                    require(same(f.sample(time).position_m,old.position_m)&&same(f.sample(time).velocity_mps,old.velocity_mps),"airborne baseline sample changed");
                }
                continue;
            }
            const auto impact=sample_reference_pitch(f.initial,f.ground_s-f.start_s);
            require(distance(impact.position_m,f.rebound.position_m)==0&&f.rebound.position_m.y==f.ground_height_m,"first position continuity");
            require(f.rebound.velocity_mps.y==-impact.velocity_mps.y*.25f&&f.rebound.velocity_mps.y>0&&f.rebound.velocity_mps.x==impact.velocity_mps.x*.7f&&f.rebound.velocity_mps.z==impact.velocity_mps.z*.7f,"first rebound velocity");
            const auto second=sample_reference_pitch(f.rebound,f.second_ground_s-f.ground_s);
            require(distance(second.position_m,f.roll.position_m)==0&&f.roll.position_m.y==f.ground_height_m&&f.roll.velocity_mps.y==0,"second position continuity");
            require(f.roll.velocity_mps.x==second.velocity_mps.x*.7f&&f.roll.velocity_mps.z==second.velocity_mps.z*.7f,"second retention");
            double previous=f.roll_speed;
            for(unsigned i=0;i<=100;++i) {
                const auto b=f.sample(f.second_ground_s+(f.stop_s-f.second_ground_s)*i/100.);
                const double speed=std::hypot(double(b.velocity_mps.x),double(b.velocity_mps.z));
                require(speed<=previous+1e-6&&b.position_m.y==f.ground_height_m&&b.velocity_mps.y==0,"ground deceleration");previous=speed;
                require(b.velocity_mps.z*f.roll.velocity_mps.z>=0,"roll reversal");
            }
            require(same(f.sample(f.stop_s).velocity_mps,DirectX::XMFLOAT3{})&&same(f.sample(f.stop_s).position_m,f.sample(f.stop_s+100).position_m),"final hold");
            if(ey==-1)require(f.rebound.velocity_mps.z<0&&f.roll.velocity_mps.z<0,"backward ground lost sign");
            const double apex=double(f.rebound.velocity_mps.y)*f.rebound.velocity_mps.y/(2*g);
            if(ey==-.75f)high=apex;if(ey==-.5f)low=apex;
            if(fps==30)std::cout<<std::setprecision(12)<<"GROUND ey="<<ey<<" speed="<<r.exit_speed_mps<<" angle="<<r.longitudinal_angle_deg<<" first_s="<<f.ground_s<<" first_distance="<<distance(f.rebound.position_m,f.initial.position_m)<<" impact_vy="<<impact.velocity_mps.y<<" rebound_vy="<<f.rebound.velocity_mps.y<<" rebound_apex_bottom="<<apex<<" second_s="<<f.second_ground_s<<" second_distance="<<distance(f.roll.position_m,f.initial.position_m)<<" roll_speed="<<f.roll_speed<<" stop_s="<<f.stop_s<<" final_distance="<<distance(f.sample(f.stop_s).position_m,f.initial.position_m)<<'\n';
        }
        if(ey<0)for(bool rolling:{false,true}) {
            p.reset();p.start();const BattingReviewFixture fixture{448,0,ey};p.record_command(448,fixture.aim_center(p));
            while(!p.flight)p.advance(4'166'667);
            const double target=rolling?(p.flight->second_ground_s+p.flight->stop_s)/2:(p.flight->ground_s+p.flight->second_ground_s)/2;
            while(double(p.tick)/pitch_hz<target)p.advance(4'166'667);
            require(p.start()&&p.tick==0&&!p.flight&&p.attempts.empty()&&p.pending_ticks==0,"Space during bounce/roll");
            p.advance(4'166'666);require(p.tick==0,"Space retained fractional debt");
        }
    }
    require(high>1&&low<1&&high>low*3,"high chopper not clearly distinct; do not tune by ey");
    p.reset();
}
int main(int argc,char** argv){try {
    require(argc==2,"expected batting directory");const std::filesystem::path d=argv[1];
    const auto staging=load_batting_staging(d/"staging.toml");ManualSwingPreview p(d,staging);
    require(std::wstring(batting_info_labels[0])==L"揮棒時機"&&std::wstring(batting_info_labels[1])==L"飛行時間"
        &&std::wstring(batting_info_labels[2])==L"飛行距離"&&std::wstring(batting_info_labels[3])==L"最大高度","primary fields changed");
    require(std::wstring(batting_info_labels[4])==L"擊球初速","exit speed label");
    for(const auto* label:batting_info_labels)require(std::wstring(label)!=L"擊球成立"&&std::wstring(label)!=L"揮空"&&std::wstring(label)!=L"未出棒","old result list remains primary");
    const auto empty=batting_info(p);require(!empty.offset_ms&&!empty.exit_speed_kmh&&!empty.flight&&empty.timing==InfoTiming::Waiting,"Ready values");
    for(const auto& row:format_batting_info(empty))require(std::strcmp(row.data(),"--")==0,"empty placeholder");
    // Presentation threshold does not feed production timing or response.
    p.attempts.push_back({});
    for(double offset:{-10.01,-10.,0.,10.,10.01}){
        p.attempts.back().timing.offset_ms=offset;
        require(batting_info(p).timing==(std::abs(offset)<=10?InfoTiming::Center:offset<0?InfoTiming::Early:InfoTiming::Late),"timing display boundary");
    }
    p.reset();ground_response_checks(p);
    std::vector<engine::Vertex> marker;
    marker.reserve(ground_projection_vertex_count);
    const auto* marker_storage=marker.data();
    const auto check_marker=[&]{
        const auto tick=p.tick;const auto info=batting_info(p);
        marker.clear();append_ground_projection(marker,p);
        require(marker.size()==96&&marker.data()==marker_storage,"marker fixed count/allocation");
        for(unsigned i=0;i<marker.size();++i){
            const auto& v=marker[i];
            require(std::isfinite(v.position.x)&&std::isfinite(v.position.y)&&std::isfinite(v.position.z),"marker finite");
            if(!p.flight){require(v.position.x==0&&v.position.y==0&&v.position.z==0,"hidden marker");continue;}
            const auto ball=p.flight->sample(double(p.tick)/pitch_hz).position_m;
            require(v.position.y==ground_projection_y_m,"marker ground height");
            require(v.color.x==.72f&&v.color.y==.74f&&v.color.z==.76f,"marker grey-white");
            if(i%3==0)require(v.position.x==ball.x&&v.position.z==ball.z,"current ball XZ projection");
            else require(std::abs(std::hypot(v.position.x-ball.x,v.position.z-ball.z)-.18f)<1e-5,"marker radius");
        }
        require(p.tick==tick&&batting_info(p)==info,"marker mutated state");
    };
    check_marker();
    for(unsigned height:{1080u,1620u}){
        ContactResultPanel panel(height*16/9,height);std::vector<engine::Vertex> drawn;
        for(const auto& v:panel.glyphs.back())require(v.position.x==0&&v.position.y==0&&v.position.z==0,"blank glyph is not degenerate");
        drawn.reserve(panel.vertex_count);const auto* allocation=drawn.data();const auto capacity=drawn.capacity();
        const auto render=[&]{
            const auto tick=p.tick;const auto pose=p.batter.pose.world;const auto before=batting_info(p);
            drawn.clear();panel.append(drawn,p);
            require(drawn.size()==panel.vertex_count&&drawn.data()==allocation&&drawn.capacity()==capacity,"fixed geometry/allocation contract");
            for(const auto& v:drawn)require(std::isfinite(v.position.x)&&std::isfinite(v.position.y),"nonfinite info geometry");
            require(tick==p.tick&&pose==p.batter.pose.world&&before==batting_info(p),"panel mutated production state");
        };
        render();require(panel.vertex_count%3==0,"triangle count");
        // Each real fixture consumes normal player intent; no forced authorization/result/flight.
        for(const BattingReviewFixture fixture:{BattingReviewFixture{448,0,0},{448,0,.5f},{448,0,.75f},{448,0,.9f},{448,0,-.5f},{448,0,-.75f},{448,.6f,1},{80,0,0},{456,0,0},{0,0,0}}){
            p.reset();p.start();if(fixture.commit_tick)require(p.record_command(fixture.commit_tick,fixture.aim_center(p)),"fixture intent rejected");
            require(batting_info(p)==empty,"reset retained panel truth");render();
            std::optional<double> timing,speed;std::optional<BattingFlightMetrics> previous,frozen;
            bool live_changed=false,peak_held=false;
            while(p.phase==PreviewPhase::Playing){
                p.advance(4'166'667);check_marker();const auto info=batting_info(p);
                if(p.latest()){
                    fixture.verify(*p.latest());require(info.offset_ms==p.timing()->offset_ms,"timing source changed");
                    if(timing)require(timing==info.offset_ms,"timing did not persist");timing=info.offset_ms;
                }else require(!info.offset_ms,"timing before consumed swing");
                if(info.flight){
                    require(info.exit_speed_kmh==double(p.latest()->response->exit_speed_mps)*3.6,"initial km/h source");
                    if(speed)require(speed==info.exit_speed_kmh,"initial speed changed during flight/hold");
                    speed=info.exit_speed_kmh;
                    const auto& f=*p.flight;const double now=double(p.tick)/pitch_hz,stop=std::clamp(now,f.start_s,f.ground_s);
                    require(info.flight->elapsed_s==stop-f.start_s,"flight time not launch-to-first-hit");
                    const auto position=f.sample(stop).position_m;
                    require(std::abs(info.flight->distance_m-std::hypot(double(position.x)-f.initial.position_m.x,double(position.z)-f.initial.position_m.z))<1e-9,"XZ distance definition");
                    const double peak_time=std::clamp(-double(f.initial.velocity_mps.y)/earth_gravity_mps2,0.,stop-f.start_s);
                    const double expected_peak=std::max(0.,double(f.initial.position_m.y)+double(f.initial.velocity_mps.y)*peak_time+.5*earth_gravity_mps2*peak_time*peak_time-f.ground_height_m);
                    require(std::abs(info.flight->max_height_m-expected_peak)<1e-5,"max bottom height includes future peak or wrong datum");
                    if(previous){
                        require(info.flight->elapsed_s>=previous->elapsed_s&&info.flight->distance_m>=previous->distance_m&&info.flight->max_height_m>=previous->max_height_m,"metrics went backwards");
                        live_changed|=info.flight->elapsed_s>previous->elapsed_s&&info.flight->distance_m>previous->distance_m;
                        peak_held|=info.flight->max_height_m==previous->max_height_m&&now<f.ground_s;
                    }
                    if(frozen)require(*info.flight==*frozen,"first-hit metrics changed");
                    if(now>=f.first_hit_s())frozen=info.flight;
                    previous=info.flight;
                }else{require(!info.exit_speed_kmh,"speed before dispatched Contact");require(!p.flight,"missing existing flight");const auto text=format_batting_info(info);for(unsigned row=1;row<5;++row)require(std::strcmp(text[row].data(),"--")==0,"Miss/NoSwing metric is not placeholder");}
                if(p.tick%120==0)render();
            }
            require(bool(timing)==(fixture.commit_tick!=0),"NoSwing timing");
            const bool contact=fixture.commit_tick!=0&&fixture.commit_tick!=80&&fixture.ex==0;
            require(p.gameplay_result()==(contact?GameplayResult::Contact:fixture.commit_tick?GameplayResult::Miss:GameplayResult::NoSwing),"production fixture outcome changed");
            require(bool(previous)==contact,"metrics not contact-only");
            if(contact)require(live_changed&&peak_held&&frozen.has_value(),"live/apex/first-hit coverage missing");
            render();const auto held=batting_info(p);const auto geometry=drawn;check_marker();const auto held_marker=marker;
            for(unsigned i=0;i<3;++i){p.advance(9'000'000'000);require(batting_info(p)==held,"hold changed metrics");}
            check_marker();require(std::memcmp(held_marker.data(),marker.data(),marker.size()*sizeof(engine::Vertex))==0,"marker ground hold");
            render();require(std::memcmp(geometry.data(),drawn.data(),drawn.size()*sizeof(engine::Vertex))==0,"cached held geometry unstable");
            std::cout<<"info fixture "<<fixture.commit_tick<<" ey="<<fixture.ey<<" result="<<p.gameplay_state();
            for(const auto& row:format_batting_info(held))std::cout<<" | "<<row.data();std::cout<<'\n';
            require(p.start()&&batting_info(p)==empty,"Space did not clear fields");check_marker();p.reset();
        }
    }
    std::cout<<"PASS batting info: live metrics, first-hit freeze, hold/reset, cached fixed geometry at 1080/1620\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
