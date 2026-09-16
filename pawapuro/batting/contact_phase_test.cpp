#include "bat_contact.hpp"
#include "batting_preview.hpp"
#include "reference_scene.hpp"
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace pawapuro;
namespace {
void require(bool ok,const char* why) {if(!ok)throw std::runtime_error(why);}
bool equal(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b) {return a.x==b.x && a.y==b.y && a.z==b.z;}
double speed(DirectX::XMFLOAT3 p) {return std::sqrt(double(p.x)*p.x+double(p.y)*p.y+double(p.z)*p.z);}
std::vector<double> event_values(const BatContact& c) {
    const auto& s=c.sample;std::vector<double> v{s.preview_time_s,c.fractional_tick,s.approach.u,s.approach.distance_m,c.ball_radius_m,c.bat_radius_m};
    for(auto p:{s.ball.position_m,s.ball.velocity_mps,s.bat.barrel,s.bat.tip,s.approach.closest,c.normal,c.surface_point,c.bat_point_velocity,c.relative_velocity})
        v.insert(v.end(),{p.x,p.y,p.z});return v;
}
struct Row {double offset_ms;std::optional<BatContact> contact;ContactProbe minimum;};
Row study(const BattingPreview& p,double offset_ms,unsigned substeps=64) {
    const auto& batter=p.batter;const double release=double(p.delivery.motion.release_tick)/pitch_hz,offset=offset_ms/1000;
    Row r{offset_ms,{},probe_contact(p.delivery.pitch.initial,release,batter,128,offset)};
    require(!r.minimum.boundary_minimum,"phase minimum at time-window boundary: review bounded window");
    for(auto tick=batter.contact_area_tick-8+1;tick<=batter.contact_area_tick+8;++tick) {
        r.contact=first_bat_contact(p.delivery.pitch.initial,release,batter,p.contact_envelope,double(tick-1)/pitch_hz,double(tick)/pitch_hz,substeps,offset);
        if(r.contact)break;
    }
    return r;
}
std::vector<Row> sweep(const BattingPreview& p,int limit) {
    std::vector<Row> rows;for(int ms=-limit;ms<=limit;ms+=2)rows.push_back(study(p,ms));return rows;
}
void compare(const std::vector<Row>& a,const std::vector<Row>& b) {
    require(a.size()==b.size(),"sweep size differs");
    for(std::size_t i=0;i<a.size();++i) {
        require(a[i].offset_ms==b[i].offset_ms && bool(a[i].contact)==bool(b[i].contact)
            && a[i].minimum.minimum.preview_time_s==b[i].minimum.minimum.preview_time_s
            && a[i].minimum.minimum.approach.distance_m==b[i].minimum.minimum.approach.distance_m
            && a[i].minimum.minimum.approach.u==b[i].minimum.minimum.approach.u,"sweep nondeterminism");
        if(a[i].contact)require(event_values(*a[i].contact)==event_values(*b[i].contact),"contact map changed");
    }
}
void csv(const std::filesystem::path& path,const std::vector<Row>& rows,const BattingStaging& staging) {
    std::ofstream out(path);require(bool(out),"cannot write map CSV");out<<std::setprecision(15);
    for(std::size_t i=0;i<rows.size();++i) {
        const auto& r=rows[i];const auto& minimum=r.minimum.minimum;const auto& s=r.contact?r.contact->sample:minimum;
        const double nan=std::numeric_limits<double>::quiet_NaN();
        std::vector<std::pair<std::string,double>> values;
        const auto add=[&](std::string name,double value){values.emplace_back(name,value);};
        const auto point=[&](const char* name,DirectX::XMFLOAT3 p){add(std::string(name)+"_x",p.x);add(std::string(name)+"_y",p.y);add(std::string(name)+"_z",p.z);};
        add("phase_offset_ms",r.offset_ms);add("contact",r.contact?1:0);add("first_time_s",r.contact?s.preview_time_s:nan);add("first_tick",r.contact?r.contact->fractional_tick:nan);add("u",r.contact?s.approach.u:nan);
        add("minimum_separation_m",minimum.approach.distance_m);add("minimum_time_s",minimum.preview_time_s);add("minimum_u",minimum.approach.u);add("geometry_time_s",s.preview_time_s);
        point("ball",s.ball.position_m);point("barrel",s.bat.barrel);point("tip",s.bat.tip);point("closest",s.approach.closest);
        const DirectX::XMFLOAT3 absent{static_cast<float>(nan),static_cast<float>(nan),static_cast<float>(nan)};
        point("normal",r.contact?r.contact->normal:absent);point("ball_velocity",r.contact?s.ball.velocity_mps:absent);point("bat_velocity",r.contact?r.contact->bat_point_velocity:absent);point("relative_velocity",r.contact?r.contact->relative_velocity:absent);
        add("relative_speed_mps",r.contact?speed(r.contact->relative_velocity):nan);
        const auto n=r.contact?r.contact->normal:absent,v=r.contact?r.contact->relative_velocity:absent;
        add("normal_closing_speed_mps",-(double(v.x)*n.x+double(v.y)*n.y+double(v.z)*n.z));
        const auto screen=[&](const char* name,DirectX::XMFLOAT3 p){const auto q=project_batting_point(staging,p,16.f/9);add(std::string(name)+"_px",(q.x+1)*960);add(std::string(name)+"_py",(1-q.y)*540);};
        screen("ball",s.ball.position_m);screen("barrel",s.bat.barrel);screen("tip",s.bat.tip);screen("closest",s.approach.closest);
        if(r.contact)screen("normal_end",{s.approach.closest.x+n.x*.1f,s.approach.closest.y+n.y*.1f,s.approach.closest.z+n.z*.1f});else {add("normal_end_px",nan);add("normal_end_py",nan);}
        if(i==0){for(std::size_t j=0;j<values.size();++j){if(j)out<<',';out<<values[j].first;}out<<'\n';}
        for(std::size_t j=0;j<values.size();++j){if(j)out<<',';if(std::isfinite(values[j].second))out<<values[j].second;}out<<'\n';
    }
}
}
int main(int argc,char** argv) {
 try {
    require(argc==3 || argc==4 || argc==6,"expected batting directory, staging, optional output directory and local refinement endpoints");
    const auto staging=load_batting_staging(argv[2]);BattingPreview p(argv[1],staging);
    // Refuse to run a sweep before reproducing the accepted S1 result.
    const auto zero=study(p,0);require(bool(zero.contact),"offset zero lost accepted S1 contact");
    require(std::abs(zero.contact->sample.preview_time_s-1.9949959225487)<1e-12 && zero.contact->sample.approach.u==0
        && std::abs(zero.contact->normal.x-.195123180747032)<1e-7 && std::abs(zero.contact->normal.y+.531813263893127)<1e-7
        && std::abs(zero.contact->normal.z-.824076235294342)<1e-7,"STOP: offset-zero S1 regression");
    p.start();p.toggle_pause();while(!p.contact)p.single_step();require(event_values(*zero.contact)==event_values(*p.contact),"zero differs from runtime event");
    engine::GlbPose scratch;const double t=1.995,release=double(p.delivery.motion.release_tick)/pitch_hz;
    const auto positive=sample_contact(p.delivery.pitch.initial,release,p.batter,t,scratch,.01),negative=sample_contact(p.delivery.pitch.initial,release,p.batter,t,scratch,-.01);
    require(equal(positive.ball.position_m,negative.ball.position_m),"phase moved pitch time");
    require(equal(positive.bat.barrel,p.batter.sample_barrel(t+.01,scratch).barrel) && equal(negative.bat.barrel,p.batter.sample_barrel(t-.01,scratch).barrel)
        && !equal(positive.bat.barrel,negative.bat.barrel),"phase sign convention failed");
    int limit=40;const auto coarse=sweep(p,limit);auto rows=coarse;
    if(rows.front().contact || rows.back().contact){limit=60;rows=sweep(p,limit);}
    if(rows.front().contact || rows.back().contact)std::cout<<"Bounded sweep edge still Contact at +/-60ms; extent is censored, no further expansion.\n";
    // Before/after state guard includes raw simulation, pose buffers and event.
    const auto tick=p.tick,bt=p.batter.tick,pt=p.delivery.tick,samples=p.batter.samples;const auto phase=p.phase;const bool paused=p.paused;
    const auto pose=p.batter.pose.world;const auto triangles=p.batter.triangles;const auto pitcher_pose=p.delivery.motion.pose.world;
    const auto ball=p.delivery.pitch.current;const auto event=event_values(*p.contact);const auto count=p.contact_count;const auto debt=p.pending_ticks;
    compare(rows,sweep(p,limit));
    for(const auto& r:rows) {
        const auto low=study(p,r.offset_ms,32),high=study(p,r.offset_ms,128);
        require(bool(low.contact)==bool(r.contact) && bool(high.contact)==bool(r.contact),"contact/NoContact boundary unstable");
        if(r.contact)require(std::abs(low.contact->sample.preview_time_s-high.contact->sample.preview_time_s)<1e-7,"entry refinement unstable");
    }
    require(p.tick==tick && p.batter.tick==bt && p.delivery.tick==pt && p.batter.samples==samples && p.phase==phase && p.paused==paused
        && p.contact_count==count && p.pending_ticks==debt && p.batter.pose.world==pose && p.delivery.motion.pose.world==pitcher_pose
        && equal(p.delivery.pitch.current.position_m,ball.position_m) && equal(p.delivery.pitch.current.velocity_mps,ball.velocity_mps)
        && event_values(*p.contact)==event && std::memcmp(triangles.data(),p.batter.triangles.data(),triangles.size()*sizeof(engine::Vertex))==0,"study mutated runtime state");
    while(p.phase!=PreviewPhase::Complete)p.single_step();
    for(unsigned fps:{30u,60u,120u}) {
        p.start();std::uint64_t prior=0;for(unsigned frame=1;frame<=fps*4;++frame){const auto now=std::uint64_t{1'000'000'000}*frame/fps;p.advance(now-prior);prior=now;}
        require(event_values(*p.contact)==event_values(*zero.contact),"runtime no longer offset zero");compare(rows,sweep(p,limit));
    }
    for(unsigned replay=0;replay<20;++replay) {
        p.start();p.toggle_pause();while(p.phase!=PreviewPhase::Complete)p.single_step();
        require(event_values(*p.contact)==event_values(*zero.contact),"replay changed zero event");compare(rows,sweep(p,limit));
    }
    if(argc>=4) {
        const std::filesystem::path out=argv[3];std::filesystem::create_directories(out);csv(out/"coarse-41.csv",coarse,staging);csv(out/"phase-map.csv",rows,staging);
        if(argc==6) {
            const double low=std::stod(argv[4]),high=std::stod(argv[5]);require(low>=-limit && high<=limit && high>low && high-low<=20,"local refinement too broad");
            std::vector<Row> fine;for(double ms=low;ms<=high+1e-9;ms+=.5)fine.push_back(study(p,ms));csv(out/"local-refinement.csv",fine,staging);
        }
    }
    std::cout<<"PASS phase study: "<<rows.size()<<" samples; zero exact S1/runtime, +/- sign, 32/64/128 stable boundary, repeated maps, 30/60/120 chunks, 20 replays, no runtime mutation.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}
}
