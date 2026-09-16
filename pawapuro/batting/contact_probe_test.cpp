#include "contact_probe.hpp"
#include "batting_preview.hpp"
#include "reference_scene.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
using namespace pawapuro;
namespace {
void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
double distance(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b) {
    return std::sqrt(std::pow(double(a.x)-b.x,2)+std::pow(double(a.y)-b.y,2)+std::pow(double(a.z)-b.z,2));
}
bool same(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b) {return a.x==b.x && a.y==b.y && a.z==b.z;}
bool same(const ContactSample& a,const ContactSample& b) {
    return a.preview_time_s==b.preview_time_s && a.approach.u==b.approach.u && a.approach.distance_m==b.approach.distance_m
        && same(a.ball.position_m,b.ball.position_m) && same(a.ball.velocity_mps,b.ball.velocity_mps)
        && same(a.bat.barrel,b.bat.barrel) && same(a.bat.tip,b.bat.tip) && same(a.approach.closest,b.approach.closest);
}
void point(std::ostream& o,DirectX::XMFLOAT3 p) {o<<'['<<p.x<<','<<p.y<<','<<p.z<<']';}
void sample_json(std::ostream& o,const ContactSample& s,const BattingStaging& staging) {
    o<<"{\"time_s\":"<<s.preview_time_s<<",\"tick\":"<<s.preview_time_s*pitch_hz<<",\"distance_m\":"<<s.approach.distance_m<<",\"u\":"<<s.approach.u;
    o<<",\"ball\":";point(o,s.ball.position_m);o<<",\"ball_velocity\":";point(o,s.ball.velocity_mps);
    o<<",\"barrel\":";point(o,s.bat.barrel);o<<",\"tip\":";point(o,s.bat.tip);o<<",\"closest\":";point(o,s.approach.closest);
    o<<",\"screen_1920x1080\": [";unsigned n=0;
    for(const auto p:{s.ball.position_m,s.bat.barrel,s.bat.tip,s.approach.closest}) {
        const auto ndc=project_batting_point(staging,p,16.f/9);if(n++)o<<',';o<<'['<<(ndc.x+1)*960<<','<<(1-ndc.y)*540<<']';
    }
    o<<"]}";
}
}
int main(int argc,char** argv) {
 try {
    require(argc==3 || argc==4,"expected batting directory, staging, optional report JSON");
    const auto staging=load_batting_staging(argv[2]);BattingPreview preview(argv[1],staging);
    auto& batter=preview.batter;const auto initial=preview.delivery.pitch.initial;
    const double release=static_cast<double>(preview.delivery.motion.release_tick)/pitch_hz;
    ReferencePitch integrated(initial.position_m,initial.velocity_mps,staging.strike_zone_plane_z());integrated.release();
    double max_position=0,max_velocity=0;
    do {
        const auto analytic=sample_reference_pitch(initial,static_cast<double>(integrated.tick)/pitch_hz);
        max_position=std::max(max_position,distance(analytic.position_m,integrated.current.position_m));
        max_velocity=std::max(max_velocity,distance(analytic.velocity_mps,integrated.current.velocity_mps));
        if(integrated.phase==PitchPhase::Complete)break;
        integrated.step_tick();
    }while(true);
    require(max_position<.0002 && max_velocity<.0002,"analytic ball differs from fixed integration tolerance");
    require(same(sample_reference_pitch(initial,0).position_m,initial.position_m) && same(sample_reference_pitch(initial,0).velocity_mps,initial.velocity_mps),"t0 changed");
    const auto arrival=integrated.arrival_at_plane();const double arrival_time=release+arrival.time_s;
    require(distance(sample_reference_pitch(initial,arrival.time_s).position_m,arrival.state.position_m)<.0002,"arrival analytic mismatch");
    // Querying past the plane must not use or extend the mutable Complete state.
    const auto raw=integrated.current;sample_reference_pitch(integrated.initial,arrival.time_s+.01);
    require(integrated.phase==PitchPhase::Complete && integrated.tick==95 && same(raw.position_m,integrated.current.position_m),"query advanced completed pitch");
    engine::GlbPose scratch;double max_semantic=0;
    for(const auto tick:{std::uint64_t{0},batter.gather_tick,batter.plant_tick,std::uint64_t{479},batter.contact_area_tick,batter.end_tick}) {
        batter.evaluate_tick(tick);const auto q=batter.sample_barrel(static_cast<double>(tick)/pitch_hz,scratch);
        max_semantic=std::max({max_semantic,distance(q.barrel,{batter.barrel_world[12],batter.barrel_world[13],batter.barrel_world[14]}),distance(q.tip,{batter.tip_world[12],batter.tip_world[13],batter.tip_world[14]})});
    }
    require(max_semantic<.0001,"continuous semantics differ from authoritative pose");
    const BatBarrelSample line{{0,0,0},{2,0,0}};
    auto q=closest_barrel_point({1,3,0},line);require(q.u==.5 && q.distance_m==3,"interior closest point");
    q=closest_barrel_point({-1,0,0},line);require(q.u==0 && q.distance_m==1,"barrel endpoint");
    q=closest_barrel_point({3,0,0},line);require(q.u==1 && q.distance_m==1,"tip endpoint");
    q=closest_barrel_point({0,3,4},{{0,0,0},{0,0,0}});require(q.u==0 && q.distance_m==5,"degenerate guard");
    const auto tick=batter.tick,samples=batter.samples;const auto pose=batter.pose.world;const auto triangles=batter.triangles;
    const auto a=probe_contact(initial,release,batter,32),b=probe_contact(initial,release,batter,64),c=probe_contact(initial,release,batter,128);
    require(!c.boundary_minimum && c.window_start_s<arrival_time && arrival_time<c.window_end_s,"expand bounded window / arrival missing");
    require(c.minimum.approach.distance_m<=b.minimum.approach.distance_m && b.minimum.approach.distance_m<=a.minimum.approach.distance_m,"nested refinement worsened");
    require(std::abs(a.minimum.approach.distance_m-c.minimum.approach.distance_m)<1e-5
        && std::abs(a.minimum.preview_time_s-c.minimum.preview_time_s)<2e-5,"fixed refinement failed to converge");
    require(same(c.minimum,probe_contact(initial,release,batter,128).minimum),"probe repeat differs");
    const auto velocity=contact_velocity(batter,c.minimum,.0001);
    require(batter.tick==tick && batter.samples==samples && batter.pose.world==pose
        && std::memcmp(triangles.data(),batter.triangles.data(),triangles.size()*sizeof(engine::Vertex))==0,"study mutated authoritative animation");
    for(unsigned fps:{30u,60u,120u}) {
        preview.reset();preview.start();std::uint64_t prior=0;
        for(unsigned frame=1;frame<=fps*4;++frame) {
            const auto now=std::uint64_t{1'000'000'000}*frame/fps;preview.advance(now-prior);prior=now;
        }
        require(preview.phase==PreviewPhase::Complete,"preview did not complete");
        require(same(c.minimum,probe_contact(preview.delivery.pitch.initial,release,batter,128).minimum),"render chunks changed probe");
    }
    std::cout<<std::setprecision(12)<<"PASS contact probe: position_error_m="<<max_position<<" velocity_error_mps="<<max_velocity<<" semantic_error_m="<<max_semantic
        <<" minimum_m="<<c.minimum.approach.distance_m<<" tick="<<c.minimum.preview_time_s*pitch_hz<<" u="<<c.minimum.approach.u<<'\n';
    if(argc==4) {
        std::ofstream o(argv[3]);require(bool(o),"cannot open diagnostic report");o<<std::setprecision(15);
        o<<"{\"meaning\":\"ball center to barrel-tip centerline, no radius or hit rule\",\"glb_sha256\":\""<<batter.asset.sha256<<"\",\"window_ticks\":["<<c.window_start_s*pitch_hz<<','<<c.window_end_s*pitch_hz<<"],\"release_tick\":"<<release*pitch_hz
         <<",\"arrival_preview_time_s\":"<<arrival_time<<",\"arrival_complete_tick\":"<<release*pitch_hz+arrival.tick
         <<",\"offset_arrival_s\":"<<c.minimum.preview_time_s-arrival_time<<",\"offset_arrival_complete_s\":"<<c.minimum.preview_time_s-(release*pitch_hz+arrival.tick)/pitch_hz
         <<",\"offset_contact_area_s\":"<<c.minimum.preview_time_s-static_cast<double>(batter.contact_area_tick)/pitch_hz
         <<",\"ball_integer_max_error_m\":"<<max_position<<",\"ball_velocity_max_error_mps\":"<<max_velocity<<",\"semantic_max_error_m\":"<<max_semantic<<",\"refinements\":[";
        unsigned i=0;for(const auto& r:{a,b,c}) {if(i++)o<<',';o<<"{\"subdivisions\":"<<r.refinement<<",\"sample\":";sample_json(o,r.minimum,staging);o<<'}';}
        o<<"],\"minimum\":";sample_json(o,c.minimum,staging);o<<",\"nearby\":[";i=0;
        for(double offset:{-.002,-.001,0.,.001,.002}) {if(i++)o<<',';sample_json(o,sample_contact(initial,release,batter,c.minimum.preview_time_s+offset,scratch),staging);}
        o<<"],\"velocity_epsilon_s\":0.0001,\"bat_fixed_u_velocity\":";point(o,velocity.bat_point);o<<",\"relative_ball_minus_bat_velocity\":";point(o,velocity.relative);o<<"}\n";
    }
    return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}
}
