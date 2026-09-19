#include "contact_panel.hpp"
#include "review_fixture.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
using namespace pawapuro;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try {
    require(argc==2,"expected batting directory");const std::filesystem::path d=argv[1];
    const auto staging=load_batting_staging(d/"staging.toml");ManualSwingPreview p(d,staging);
    require(std::wstring(batting_info_labels[0])==L"揮棒時機"&&std::wstring(batting_info_labels[1])==L"飛行時間"
        &&std::wstring(batting_info_labels[2])==L"飛行距離"&&std::wstring(batting_info_labels[3])==L"最大高度","primary fields changed");
    for(const auto* label:batting_info_labels)require(std::wstring(label)!=L"擊球成立"&&std::wstring(label)!=L"揮空"&&std::wstring(label)!=L"未出棒","old result list remains primary");
    const auto empty=batting_info(p);require(!empty.offset_ms&&!empty.flight&&empty.timing==InfoTiming::Waiting,"Ready values");
    for(const auto& row:format_batting_info(empty))require(std::strcmp(row.data(),"--")==0,"empty placeholder");
    // Presentation threshold does not feed production timing or response.
    p.attempts.push_back({});
    for(double offset:{-10.01,-10.,0.,10.,10.01}){
        p.attempts.back().timing.offset_ms=offset;
        require(batting_info(p).timing==(std::abs(offset)<=10?InfoTiming::Center:offset<0?InfoTiming::Early:InfoTiming::Late),"timing display boundary");
    }
    p.reset();
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
        for(const BattingReviewFixture fixture:{BattingReviewFixture{448,0,0},{448,0,.5f},{448,.6f,1},{80,0,0},{456,0,0},{0,0,0}}){
            p.reset();p.start();if(fixture.commit_tick)require(p.record_command(fixture.commit_tick,fixture.aim_center(p)),"fixture intent rejected");
            require(batting_info(p)==empty,"reset retained panel truth");render();
            std::optional<double> timing;std::optional<BattingFlightMetrics> previous,frozen;
            bool live_changed=false,peak_held=false;
            while(p.phase==PreviewPhase::Playing){
                p.advance(4'166'667);const auto info=batting_info(p);
                if(p.latest()){
                    fixture.verify(*p.latest());require(info.offset_ms==p.timing()->offset_ms,"timing source changed");
                    if(timing)require(timing==info.offset_ms,"timing did not persist");timing=info.offset_ms;
                }else require(!info.offset_ms,"timing before consumed swing");
                if(info.flight){
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
                    if(f.complete(now))frozen=info.flight;
                    previous=info.flight;
                }else{require(!p.flight,"missing existing flight");const auto text=format_batting_info(info);for(unsigned row=1;row<4;++row)require(std::strcmp(text[row].data(),"--")==0,"Miss/NoSwing metric is not placeholder");}
                if(p.tick%120==0)render();
            }
            require(bool(timing)==(fixture.commit_tick!=0),"NoSwing timing");
            const bool contact=fixture.commit_tick!=0&&fixture.commit_tick!=80&&fixture.ex==0;
            require(p.gameplay_result()==(contact?GameplayResult::Contact:fixture.commit_tick?GameplayResult::Miss:GameplayResult::NoSwing),"production fixture outcome changed");
            require(bool(previous)==contact,"metrics not contact-only");
            if(contact)require(live_changed&&peak_held&&frozen.has_value(),"live/apex/first-hit coverage missing");
            render();const auto held=batting_info(p);const auto geometry=drawn;
            for(unsigned i=0;i<3;++i){p.advance(9'000'000'000);require(batting_info(p)==held,"hold changed metrics");}
            render();require(std::memcmp(geometry.data(),drawn.data(),drawn.size()*sizeof(engine::Vertex))==0,"cached held geometry unstable");
            std::cout<<"info fixture "<<fixture.commit_tick<<" ey="<<fixture.ey<<" result="<<p.gameplay_state();
            for(const auto& row:format_batting_info(held))std::cout<<" | "<<row.data();std::cout<<'\n';
            require(p.start()&&batting_info(p)==empty,"Space did not clear fields");p.reset();
        }
    }
    std::cout<<"PASS batting info: live metrics, first-hit freeze, hold/reset, cached fixed geometry at 1080/1620\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
