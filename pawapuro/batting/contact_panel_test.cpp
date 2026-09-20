#include "contact_panel.hpp"
#include "review_fixture.hpp"
#include "ground_projection.hpp"
#include "reference_scene.hpp"
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
    const double g=-double(earth_gravity_mps2);
    std::array<double,2> high{},low{};
    for(float ey:{-.75f,-.5f,-1.f,-.25f,0.f,.25f,.5f,.75f,.9f}) {
        std::string control;
        for(unsigned fps:{30u,60u,120u,1u}) {
            p.reset();p.start();const BattingReviewFixture fixture{448,0,ey};
            require(p.record_command(448,fixture.aim_center(p)),"ground fixture intent");
            while(p.phase==PreviewPhase::Playing)p.advance(1'000'000'000ull/fps);
            require(p.flight.has_value(),"ground fixture Contact");fixture.verify(*p.latest());
            const auto f=*p.flight;const auto r=*p.latest()->response;
            std::ostringstream signature;signature<<std::setprecision(17)<<p.tick<<','<<f.ground_s<<','<<f.rebound_settle_s<<','<<f.horizontal_stop_s<<','<<f.stop_s;
            const auto record=[&](double t){const auto b=f.sample(t);signature<<','<<t<<','<<b.position_m.x<<','<<b.position_m.y<<','<<b.position_m.z<<','<<b.velocity_mps.x<<','<<b.velocity_mps.y<<','<<b.velocity_mps.z;};
            for(double t:{f.start_s,f.ground_s,f.rebound_settle_s,f.horizontal_stop_s,f.stop_s,f.stop_s+10})record(t);
            const double legacy_ground=f.start_s+(double(r.launch_velocity_mps.y)+std::sqrt(double(r.launch_velocity_mps.y)*r.launch_velocity_mps.y+2*g*std::max(0.,double(r.launch_position_m.y)-f.ground_height_m)))/g;
            require(f.ground_s==legacy_ground,"first impact time changed");
            for(unsigned i=0;i<100;++i) {
                const double time=f.start_s+(legacy_ground-f.start_s)*i/100.;
                const auto old=sample_reference_pitch(f.initial,time-f.start_s);
                require(same(f.sample(time).position_m,old.position_m)&&same(f.sample(time).velocity_mps,old.velocity_mps),"pre-ground baseline sample changed");record(time);
            }
            {
                const auto impact=sample_reference_pitch(f.initial,f.ground_s-f.start_s);
                require(distance(impact.position_m,f.first_ground.position_m)==0&&f.first_ground.position_m.y==f.ground_height_m,"first impact continuity");
                require(f.first_ground.velocity_mps.x==impact.velocity_mps.x*.85f&&f.first_ground.velocity_mps.z==impact.velocity_mps.z*.85f,"one-time horizontal retention");
                require(f.first_rebound_vy==-double(impact.velocity_mps.y)*double(.50f),"first rebound ratio");
                require(f.rebound_duration==(2*f.first_rebound_vy/g)/(1-double(.50f))&&f.rebound_settle_s==f.ground_s+f.rebound_duration,"infinite geometric settle");
                require(f.ground_deceleration==5.5&&f.rebound_ratio==.5,"authored ground response tuning");
                require(f.sample(f.ground_s).velocity_mps.y>0&&!f.complete(f.ground_s),"first impact instantly froze");
                if(ey==-.25f)require(r.launch_velocity_mps.y>0&&r.longitudinal_angle_deg>0&&r.longitudinal_angle_deg<5,"low-positive launch regression premise");
                double previous_apex=0,previous_u=0,previous_duration=0;
                for(unsigned k=0;k<6;++k) {
                    const double u=f.rebound_launch_speed(k),start=f.rebound_start_s(k),end=f.rebound_start_s(k+1),duration=2*u/g,apex=u*u/(2*g);
                    if(k){require(std::abs(u/previous_u-f.rebound_ratio)<1e-14&&std::abs(apex/previous_apex-f.rebound_ratio*f.rebound_ratio)<1e-14&&std::abs(duration/previous_duration-f.rebound_ratio)<1e-14,"geometric ratios");}
                    require(std::abs((end-start)-duration)<1e-14,"arc duration");
                    const auto top=f.sample((start+end)/2);
                    require(std::abs(double(top.position_m.y)-f.ground_height_m-apex)<1e-5&&std::abs(top.velocity_mps.y)<1e-5,"analytic apex");
                    if(apex<.05)require(top.position_m.y>f.ground_height_m,"low hops were discarded");
                    const auto boundary=f.sample(start);
                    require(boundary.position_m.y==f.ground_height_m&&boundary.velocity_mps.y==static_cast<float>(u),"exact impact uses outgoing arc");
                    for(double t:{std::nextafter(start,-INFINITY),start,std::nextafter(start,INFINITY)}) {
                        const auto v=f.sample(t);record(t);
                        require(std::isfinite(v.position_m.y)&&v.position_m.y>=f.ground_height_m,"impact finite/nonpenetrating");
                        require(distance(v.position_m,boundary.position_m)<1e-5&&std::abs(v.position_m.y-boundary.position_m.y)<1e-5,"impact position continuity");
                        if(k>0)require(std::abs(v.velocity_mps.x-boundary.velocity_mps.x)<1e-5&&std::abs(v.velocity_mps.z-boundary.velocity_mps.z)<1e-5,"later impact horizontal discontinuity");
                    }
                    const double t=start-f.ground_s,expected=std::max(0.,f.ground_speed-f.ground_deceleration*t);
                    require(std::abs(std::hypot(double(boundary.velocity_mps.x),double(boundary.velocity_mps.z))-expected)<1e-5,"later impact multiplied horizontal speed");
                    if(k<2){if(ey==-.75f)high[k]=apex;if(ey==-.5f)low[k]=apex;}
                    if(fps==30)std::cout<<std::setprecision(12)<<"REBOUND ey="<<ey<<" index="<<k<<" start_elapsed="<<start-f.start_s<<" end_elapsed="<<end-f.start_s<<" vy="<<u<<" apex="<<apex<<'\n';
                    previous_apex=apex;previous_u=u;previous_duration=duration;
                }
                require(f.horizontal_stop_s==f.ground_s+f.ground_speed/f.ground_deceleration&&f.stop_s==std::max(f.horizontal_stop_s,f.rebound_settle_s),"component stop times");
                double previous=f.ground_speed;
                for(unsigned i=0;i<=200;++i) {
                    const double time=f.ground_s+(f.stop_s-f.ground_s)*i/200.;const auto b=f.sample(time);record(time);
                    const double speed=std::hypot(double(b.velocity_mps.x),double(b.velocity_mps.z));
                    require(speed<=previous+1e-6&&b.position_m.y>=f.ground_height_m,"continuous ground deceleration");previous=speed;
                    require(b.velocity_mps.z*f.first_ground.velocity_mps.z>=0,"horizontal reversal");
                }
                const auto stopped=f.sample(f.stop_s),horizontal=f.sample(f.horizontal_stop_s);
                require(horizontal.velocity_mps.x==0&&horizontal.velocity_mps.z==0&&distance(horizontal.position_m,stopped.position_m)==0,"horizontal stop hold");
                const double displacement=f.ground_speed*f.ground_speed/(2*f.ground_deceleration);
                require(horizontal.position_m.x==f.first_ground.position_m.x+static_cast<float>(f.first_ground.velocity_mps.x/f.ground_speed*displacement)&&horizontal.position_m.z==f.first_ground.position_m.z+static_cast<float>(f.first_ground.velocity_mps.z/f.ground_speed*displacement),"analytic horizontal stop position");
                for(double t:{std::nextafter(f.rebound_settle_s,-INFINITY),f.rebound_settle_s,std::nextafter(f.rebound_settle_s,INFINITY),f.stop_s+100}) {
                    const auto v=f.sample(t);require(std::isfinite(v.position_m.y)&&v.position_m.y>=f.ground_height_m,"settle boundary finite");
                    if(t>=f.rebound_settle_s)require(v.position_m.y==f.ground_height_m&&v.velocity_mps.y==0,"vertical settle exact");
                }
                require(!f.complete(std::nextafter(f.stop_s,-INFINITY))&&f.complete(f.stop_s)&&same(stopped.velocity_mps,DirectX::XMFLOAT3{})&&same(stopped.position_m,f.sample(f.stop_s+100).position_m),"complete/indefinite hold");
                // Also exercise horizontal stop before the rebound train settles.
                auto fast=p.tuning.ground_ball_response;fast.ground_horizontal_deceleration_mps2=1000;
                const BattedBallFlight in_place(r,f.ground_height_m,fast);
                require(in_place.horizontal_stop_s<in_place.rebound_settle_s&&!in_place.complete(in_place.horizontal_stop_s),"remaining in-place rebounds");
                const auto moving_y=in_place.sample((in_place.horizontal_stop_s+in_place.rebound_settle_s)/2);
                require(moving_y.velocity_mps.x==0&&moving_y.velocity_mps.z==0&&moving_y.position_m.y>f.ground_height_m,"independent vertical/horizontal components");
                if(ey==-1||ey==.9f){
                    std::vector<engine::Vertex> aid;aid.reserve(ball_readability_vertex_count);const auto storage=aid.data();
                    for(unsigned i=0;i<=100;++i){
                        const auto b=f.sample(f.start_s+(f.stop_s-f.start_s)*i/100.);require(b.velocity_mps.z<=0,"backward lost sign");
                        aid.clear();append_ball_readability(aid,p.tuning,b.position_m,PitchPhase::InFlight,true,1920,1080);
                        require(aid.size()==ball_readability_vertex_count&&aid.data()==storage,"backward aid fixed allocation");
                        for(const auto& v:aid)require(std::isfinite(v.position.x)&&std::isfinite(v.position.y),"backward aid invalid projection");
                    }
                }
                if(fps==30)std::cout<<std::setprecision(12)<<"GROUND ey="<<ey<<" speed="<<r.exit_speed_mps<<" angle="<<r.longitudinal_angle_deg<<" first_elapsed="<<f.ground_s-f.start_s<<" first_distance="<<distance(impact.position_m,f.initial.position_m)<<" impact_vy="<<impact.velocity_mps.y<<" rebound_vy="<<f.first_rebound_vy<<" settle_elapsed="<<f.rebound_settle_s-f.start_s<<" ground_speed="<<f.ground_speed<<" horizontal_stop_elapsed="<<f.horizontal_stop_s-f.start_s<<" stop_elapsed="<<f.stop_s-f.start_s<<" final_distance="<<distance(stopped.position_m,f.initial.position_m)<<'\n';
            }
            if(control.empty())control=signature.str();else require(control==signature.str(),"30/60/120/backlog analytic determinism");
        }
        if(ey<=0)for(unsigned stage:{0u,1u,2u}) {
            p.reset();p.start();const BattingReviewFixture fixture{448,0,ey};p.record_command(448,fixture.aim_center(p));
            while(!p.flight)p.advance(4'166'667);
            const auto& f=*p.flight;
            const double target=stage==2?(f.rebound_settle_s+f.horizontal_stop_s)/2:stage==1?(f.rebound_start_s(4)+f.rebound_start_s(5))/2:(f.rebound_start_s(0)+f.rebound_start_s(1))/2;
            while(double(p.tick)/pitch_hz<target)p.advance(4'166'667);
            require(p.start()&&p.tick==0&&!p.flight&&p.attempts.empty()&&p.pending_ticks==0,"Space during high/low/settled ground travel");
            p.advance(4'166'666);require(p.tick==0,"Space retained fractional debt");
        }
    }
    require(high[0]>5&&low[0]>1&&low[0]<2&&high[0]>low[0]*3&&high[1]>low[1]*3,"high chopper not clearly distinct; do not tune by ey");
    p.reset();
}
int main(int argc,char** argv){try {
    require(argc==2,"expected batting directory");const std::filesystem::path d=argv[1];
    const auto staging=load_batting_staging(d/"staging.toml");ManualSwingPreview p(d,staging);
    require(std::wstring(batting_info_labels[0])==L"揮棒時機"&&std::wstring(batting_info_labels[1])==L"飛行時間"
        &&std::wstring(batting_info_labels[2])==L"飛行距離"&&std::wstring(batting_info_labels[3])==L"最大高度","primary fields changed");
    require(std::wstring(batting_info_labels[4])==L"擊球初速","exit speed label");
    for(const auto* label:batting_info_labels)require(std::wstring(label)!=L"擊球成立"&&std::wstring(label)!=L"揮空"&&std::wstring(label)!=L"未出棒","old result list remains primary");
    const auto empty=batting_info(p);require(!empty.offset_ms&&!empty.exit_speed_kmh&&!empty.flight&&empty.timeline.marker==TimingMarker::None,"Ready values");
    for(const auto& row:format_batting_info(empty))require(std::strcmp(row.data(),"--")==0,"empty placeholder");
    const double duration=p.ball_passage.exit_s-p.ball_passage.enter_s;
    const double display_start=p.ball_passage.enter_s-.25*duration,display_end=p.ball_passage.exit_s+.25*duration;
    const auto timeline_position=[&](double time){return std::clamp((time-display_start)/(display_end-display_start),0.,1.);};
    require(std::abs(empty.timeline.passage_enter-1./6)<1e-12&&std::abs(1-empty.timeline.passage_exit-1./6)<1e-12
        &&std::abs(empty.timeline.passage_exit-empty.timeline.passage_enter-2./3)<1e-12,"passage dominance and equal margins");
    std::cout<<std::setprecision(17)<<"TIMELINE duration="<<duration<<" start="<<display_start<<" end="<<display_end
        <<" yellow_fraction="<<empty.timeline.passage_exit-empty.timeline.passage_enter<<'\n';
    require(empty.timeline.passage_enter==timeline_position(p.ball_passage.enter_s)&&empty.timeline.passage_exit==timeline_position(p.ball_passage.exit_s)
        &&empty.timeline.passage_enter>0&&empty.timeline.passage_enter<empty.timeline.passage_exit&&empty.timeline.passage_exit<1,"visible authoritative yellow interval");
    std::array<double,3> contact_positions{},old_positions{};unsigned contact_index=0;
    for(const BattingReviewFixture fixture:{BattingReviewFixture{441,0,0},{448,0,0},{455,0,0},{80,0,0},{480,0,0},{448,.6f,1}}){
        std::optional<BattingInfo> control;
        for(const std::uint64_t cadence:{4'166'667ULL,33'333'333ULL,8'333'333ULL,250'000'000ULL}){
            p.reset();p.start();require(p.record_command(fixture.commit_tick,fixture.aim_center(p)),"timeline fixture intent");
            while(!p.latest()||p.latest()->gameplay==GameplayResult::Pending)p.advance(cadence);
            fixture.verify(*p.latest());const auto info=batting_info(p);const bool contact=p.latest()->gameplay==GameplayResult::Contact;
            require(info.timeline.marker==(contact?TimingMarker::Contact:TimingMarker::SwingPeak),"timeline marker kind");
            const double event=contact?p.latest()->response->contact_time_s:p.timing()->peak_s;
            if(!control){
                std::cout<<"TIMELINE tick="<<fixture.commit_tick<<" event="<<event<<" position="<<info.timeline.marker_position<<" ms="<<*info.offset_ms<<'\n';
                if(contact_index<3)old_positions[contact_index]=std::clamp(.5+(event-p.ball_passage.reference_s)/.080,0.,1.);
            }
            require(info.timeline.marker_position==timeline_position(event)&&info.offset_ms==p.timing()->offset_ms,"timeline event time or secondary offset");
            require(info.timeline.passage_enter==empty.timeline.passage_enter&&info.timeline.passage_exit==empty.timeline.passage_exit,"attempt moved interval");
            if(control)require(info.timeline==control->timeline&&info.offset_ms==control->offset_ms,"timeline replay/cadence changed");else control=info;
            if(fixture.commit_tick==80){
                require(info.timeline.marker_position==0&&*info.offset_ms<-1000,"early peak clamp preserves full offset");
                char expected[32];std::snprintf(expected,sizeof(expected),"%+.1f ms",p.timing()->offset_ms);
                require(std::strcmp(format_batting_info(info)[0].data(),expected)==0,"clamped peak changed ms text");
            }
            if(fixture.commit_tick==480)require(info.timeline.marker_position==1,"late peak clamp");
        }
        if(contact_index<3)contact_positions[contact_index++]=control->timeline.marker_position;
    }
    require(contact_positions[0]<contact_positions[1]&&contact_positions[1]<contact_positions[2],"early/center/late vertical ordering");
    for(unsigned i=1;i<3;++i)require(contact_positions[i]-contact_positions[i-1]>old_positions[i]-old_positions[i-1],"focus did not increase contact separation");
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
            const auto band=panel.base.size(),marker_start=band+6;
            require(drawn[band].position.y==1-(100+144*float(before.timeline.passage_enter))*panel.y_scale
                &&drawn[band+2].position.y==1-(100+144*float(before.timeline.passage_exit))*panel.y_scale
                &&drawn[band].position.y>drawn[band+2].position.y,"yellow screen-space ordering/position");
            if(before.timeline.marker==TimingMarker::None){
                for(std::size_t i=0;i<panel.timing_markers[0].size();++i)require(drawn[marker_start+i].position.x==0&&drawn[marker_start+i].position.y==0,"no-swing marker visible");
            }else{
                const bool filled=before.timeline.marker==TimingMarker::Contact;
                const auto& marker_template=panel.timing_markers[filled?1:0];
                require(marker_template[0].color.x==(filled?.95f:.66f),"filled white/hollow gray variant");
                if(filled)require(drawn[marker_start].position.x==232*panel.x_scale-1
                    &&drawn[marker_start].position.y==1-(100+144*float(before.timeline.marker_position))*panel.y_scale,"filled center does not match contact time");
                else require(drawn[marker_start].position.x==(240*panel.x_scale-1),"hollow outline radius");
            }
        };
        render();require(panel.vertex_count%3==0,"triangle count");
        // Each real fixture consumes normal player intent; no forced authorization/result/flight.
        for(const BattingReviewFixture fixture:{BattingReviewFixture{448,0,0},{448,0,.5f},{448,0,.75f},{448,0,.9f},{448,0,-.5f},{448,0,-.75f},{448,0,-.25f},{448,0,.25f},{448,.6f,1},{80,0,0},{456,0,0},{0,0,0}}){
            p.reset();p.start();if(fixture.commit_tick)require(p.record_command(fixture.commit_tick,fixture.aim_center(p)),"fixture intent rejected");
            require(batting_info(p)==empty,"reset retained panel truth");render();
            std::optional<double> timing,speed;std::optional<BattingFlightMetrics> previous,frozen;
            bool live_changed=false,peak_held=false;
            while(p.phase==PreviewPhase::Playing){
                p.advance(4'166'667);check_marker();const auto info=batting_info(p);
                if(p.latest()){
                    fixture.verify(*p.latest());require(info.offset_ms==p.timing()->offset_ms,"timing source changed");
                    const bool dispatched=p.latest()->gameplay==GameplayResult::Contact;
                    require(info.timeline.marker==(dispatched?TimingMarker::Contact:TimingMarker::SwingPeak)
                        &&info.timeline.marker_position==timeline_position(dispatched?p.latest()->response->contact_time_s:p.timing()->peak_s),"planned contact presented as dispatched or wrong latest attempt");
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
