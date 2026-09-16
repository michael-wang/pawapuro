#include "bat_contact.hpp"
#include "batting_preview.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
using namespace pawapuro;
namespace {
void require(bool ok,const char* why) {if(!ok)throw std::runtime_error(why);}
double length(DirectX::XMFLOAT3 p) {return std::sqrt(double(p.x)*p.x+double(p.y)*p.y+double(p.z)*p.z);}
std::vector<double> signature(const BatContact& c) {
    const auto& s=c.sample;std::vector<double> v{s.preview_time_s,c.fractional_tick,s.approach.u,s.approach.distance_m,c.ball_radius_m,c.bat_radius_m};
    for(auto p:{s.ball.position_m,s.ball.velocity_mps,s.bat.barrel,s.bat.tip,s.approach.closest,c.normal,c.surface_point,c.bat_point_velocity,c.relative_velocity})
        v.insert(v.end(),{p.x,p.y,p.z});return v;
}
std::optional<BatContact> search(const BattingPreview& p,BatContactEnvelope e,unsigned substeps) {
    for(auto tick=p.batter.contact_area_tick-8+1;tick<=p.batter.contact_area_tick+8;++tick) {
        const auto c=first_bat_contact(p.delivery.pitch.initial,double(p.delivery.motion.release_tick)/pitch_hz,p.batter,e,double(tick-1)/pitch_hz,double(tick)/pitch_hz,substeps);
        if(c)return c;
    }return std::nullopt;
}
void point(const char* label,DirectX::XMFLOAT3 p) {std::cout<<label<<"=("<<p.x<<','<<p.y<<','<<p.z<<")\n";}
}
int main(int argc,char** argv) {
 try {
    require(argc==3,"expected batting directory and staging");const auto staging=load_batting_staging(argv[2]);BattingPreview p(argv[1],staging);
    const auto a=search(p,staging.bat_contact,32),b=search(p,staging.bat_contact,64),c=search(p,staging.bat_contact,128);
    require(a && b && c,"current accepted choreography has no contact");
    std::cout<<std::setprecision(15);
    for(const auto& value:{a,b,c})std::cout<<"refinement time="<<value->sample.preview_time_s<<" tick="<<value->fractional_tick<<" separation="<<value->sample.approach.distance_m<<'\n';
    require(std::abs(a->sample.preview_time_s-c->sample.preview_time_s)<1e-7 && std::abs(b->sample.preview_time_s-c->sample.preview_time_s)<1e-7,"entry refinement not stable");
    const double radius=double(staging.bat_contact.ball_radius_m)+staging.bat_contact.bat_radius_m;
    require(c->sample.approach.distance_m<=radius && radius-c->sample.approach.distance_m<1e-6,"not on inside entry boundary");
    require(std::abs(length(c->normal)-1)<1e-6,"contact normal not unit");
    require(length(c->relative_velocity)>0 && std::isfinite(length(c->bat_point_velocity)),"invalid contact velocities");
    engine::GlbPose scratch;const double release=double(p.delivery.motion.release_tick)/pitch_hz;
    require(sample_contact(p.delivery.pitch.initial,release,p.batter,c->sample.preview_time_s-1e-6,scratch).approach.distance_m>radius,"returned minimum/late contact rather than entry");
    require(sample_contact(p.delivery.pitch.initial,release,p.batter,c->sample.preview_time_s+1e-6,scratch).approach.distance_m<radius,"entry not followed by intersection");
    // Measure this accepted envelope's dwell time; exit is diagnostic only.
    double inside=c->sample.preview_time_s,outside=inside;bool found_exit=false;
    for(unsigned i=1;i<=256;++i) {
        outside=c->sample.preview_time_s+double(i)/(pitch_hz*128);
        if(sample_contact(p.delivery.pitch.initial,release,p.batter,outside,scratch).approach.distance_m>radius) {found_exit=true;break;}
        inside=outside;
    }
    require(found_exit,"contact dwell did not exit bounded diagnostic interval");
    for(unsigned i=0;i<24;++i) {
        const double mid=(inside+outside)*.5;
        if(sample_contact(p.delivery.pitch.initial,release,p.batter,mid,scratch).approach.distance_m<=radius)inside=mid;else outside=mid;
    }
    require(outside-c->sample.preview_time_s<1.0/pitch_hz,"accepted contact window no longer sub-tick");
    std::cout<<"dwell exit_time="<<outside<<" duration_ms="<<(outside-c->sample.preview_time_s)*1000<<'\n';
    const auto minimum=probe_contact(p.delivery.pitch.initial,release,p.batter,128);
    require(c->sample.preview_time_s<minimum.minimum.preview_time_s && c->sample.approach.u==0 && minimum.minimum.approach.u==0,"endpoint / minimum relationship changed");
    require(!search(p,{.020f,.020f},64),"smaller artificial envelope should be NoContact");
    const auto larger=search(p,{.040f,.040f},64);require(larger && larger->sample.preview_time_s<c->sample.preview_time_s,"larger envelope did not enter earlier");
    auto visual=staging;visual.ball_marker_radius_m=.15f;BattingPreview different_visual(argv[1],visual);
    require(signature(*search(different_visual,visual.bat_contact,64))==signature(*b),"visual marker affected gameplay contact");
    const auto entry_tick=static_cast<std::uint64_t>(std::ceil(b->fractional_tick));
    // Compare every mutable physics sample and both animation poses to callers
    // with no contact event owner. No response may leak into accepted playback.
    PitchDelivery baseline(std::filesystem::path(argv[1])/"pitcher/pitcher.glb",std::filesystem::path(argv[1])/"pitcher/pitcher.toml",staging);
    BatterMotion baseline_batter(std::filesystem::path(argv[1])/"batter/batter.glb",std::filesystem::path(argv[1])/"batter/batter.toml",staging);
    p.start();p.toggle_pause();baseline.start();
    for(std::uint64_t tick=1;tick<=p.batter.end_tick;++tick) {
        p.advance(1'000'000'000);require(p.tick==tick-1,"pause advanced contact clock");p.single_step();baseline.step_tick();baseline_batter.evaluate_tick(tick);
        require(bool(p.contact)==(tick>=entry_tick) && p.contact_count==(tick>=entry_tick?1u:0u),"event missing/duplicate/early");
        require(p.delivery.pitch.current.position_m.x==baseline.pitch.current.position_m.x
            && p.delivery.pitch.current.position_m.y==baseline.pitch.current.position_m.y && p.delivery.pitch.current.position_m.z==baseline.pitch.current.position_m.z
            && p.delivery.pitch.current.velocity_mps.y==baseline.pitch.current.velocity_mps.y && p.delivery.ball_owner==baseline.ball_owner
            && p.delivery.motion.pose.world==baseline.motion.pose.world && p.batter.pose.world==baseline_batter.pose.world,"contact changed physics/animation");
        if(p.contact) require(signature(*p.contact)==signature(*b),"stored event changed");
    }
    require(p.arrival_tick==479 && p.delivery.tick==816 && p.tick==896,"completion semantics changed");
    for(unsigned fps:{30u,60u,120u}) {
        p.start();require(!p.contact && !p.contact_count,"replay retained contact");std::uint64_t prior=0;
        for(unsigned frame=1;frame<=fps*4;++frame) {const auto now=std::uint64_t{1'000'000'000}*frame/fps;p.advance(now-prior);prior=now;}
        require(p.contact_count==1 && signature(*p.contact)==signature(*b),"render chunking changed event");
    }
    for(unsigned replay=0;replay<20;++replay) {
        p.start();p.toggle_pause();require(!p.contact && !p.contact_count,"replay not NoContact");
        while(p.phase!=PreviewPhase::Complete)p.single_step();
        require(p.contact_count==1 && signature(*p.contact)==signature(*b),"20 replays changed event");
    }
    auto reduced_staging=staging;reduced_staging.bat_contact={.020f,.020f};BattingPreview no_contact(argv[1],reduced_staging);no_contact.start();no_contact.toggle_pause();
    while(no_contact.phase!=PreviewPhase::Complete)no_contact.single_step();
    require(!no_contact.contact && !no_contact.contact_count && no_contact.arrival_tick==479,"NoContact preview changed completion");
    const auto& s=b->sample;std::cout<<"FIRST time_s="<<s.preview_time_s<<" tick="<<b->fractional_tick<<" dispatch_tick="<<entry_tick<<" separation="<<s.approach.distance_m<<" u="<<s.approach.u
        <<" marker_delta_ms="<<(s.preview_time_s-double(p.batter.contact_area_tick)/pitch_hz)*1000<<" minimum_u="<<minimum.minimum.approach.u<<'\n';
    point("ball",s.ball.position_m);point("barrel",s.bat.barrel);point("tip",s.bat.tip);point("closest",s.approach.closest);point("surface",b->surface_point);point("normal",b->normal);
    point("ball_velocity",s.ball.velocity_mps);point("bat_surface_velocity",b->bat_point_velocity);point("relative_velocity",b->relative_velocity);
    std::cout<<"speeds ball="<<length(s.ball.velocity_mps)<<" bat="<<length(b->bat_point_velocity)<<" relative="<<length(b->relative_velocity)<<'\n';
    std::cout<<"PASS first contact: refinement32/64/128, one shot, pause/step, 20 replays, 30/60/120 chunks, smaller NoContact, larger earlier; unchanged physics/animation.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}
}
