#include "manual_swing.hpp"
#include "player_aim.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
using namespace pawapuro;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
bool same(DirectX::XMFLOAT2 a,DirectX::XMFLOAT2 b){return a.x==b.x&&a.y==b.y;}
bool same(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b){return a.x==b.x&&a.y==b.y&&a.z==b.z;}
bool same(const HitAuthorizationDecision& a,const HitAuthorizationDecision& b){
    return a.authorized==b.authorized&&same(a.pitch_point,b.pitch_point)&&same(a.error,b.error)&&same(a.normalized_error,b.normalized_error)&&a.q==b.q;
}
void same_contact(const BatContact& a,const BatContact& b){
    require(a.sample.preview_time_s==b.sample.preview_time_s&&a.fractional_tick==b.fractional_tick&&a.sample.approach.u==b.sample.approach.u
        &&same(a.normal,b.normal)&&same(a.relative_velocity,b.relative_velocity)&&same(a.surface_point,b.surface_point)
        &&same(a.bat_point_velocity,b.bat_point_velocity)&&a.ball_radius_m==b.ball_radius_m&&a.bat_radius_m==b.bat_radius_m,"aim changed physical contact data");
}
void until(ManualSwingPreview& p,std::uint64_t tick){while(p.tick<tick)p.advance(4'166'667);}
int main(int argc,char** argv){try {
    require(argc==2,"expected batting directory");const std::filesystem::path d=argv[1];
    const auto s=load_batting_staging(d/"staging.toml");const auto t=s.hit_authorization;
    const auto center=authorize_normal_hit({0,0},{0,0},t);
    require(center.authorized&&same(center.error,{0,0})&&same(center.normalized_error,{0,0})&&center.q==0,"center");
    for(auto axis:{DirectX::XMFLOAT2{t.normal_radius_x_m,0},{0,t.normal_radius_y_m}}) {
        const auto edge=authorize_normal_hit({0,0},axis,t);
        require(edge.authorized&&edge.q==1,"closed exact boundary");
        const auto outside=authorize_normal_hit({0,0},{axis.x*1.001f,axis.y*1.001f},t);
        require(!outside.authorized&&outside.q>1,"outside");
        const auto negative=authorize_normal_hit({0,0},{-axis.x,-axis.y},t);
        require(negative.q==edge.q&&negative.error.x==-edge.error.x&&negative.error.y==-edge.error.y
            &&negative.normalized_error.x==-edge.normalized_error.x&&negative.normalized_error.y==-edge.normalized_error.y,"signed intent");
    }
    ManualSwingPreview a(d,s),b(d,s);PlayerAim live(s);
    const auto predicted=predict_arrival(a.delivery.pitch).state.position_m;
    const DirectX::XMFLOAT2 point{predicted.x,predicted.y},outside{point.x+t.normal_radius_x_m*1.01f,point.y};
    require(!a.authorization,"Ready decision");a.start();b.start();
    require(a.record_command(441,point)&&b.record_command(441,outside),"fixture commands");
    require(!a.authorization&&!b.authorization,"queued decision too early");
    until(a,440);until(b,440);require(!a.authorization&&!b.authorization,"pre-consumption decision");
    until(a,441);until(b,441);
    require(a.authorization->authorized&&!b.authorization->authorized&&a.authorization->q==0,"441 authorization pair");
    const auto saved_a=*a.authorization,saved_b=*b.authorization;
    for(auto* p:{&a,&b}) {
        p->toggle_pause();const auto saved=*p->authorization;const auto tick=p->tick;
        live.move(1,1,.05);p->input_boundary(false,false,true,live.center());p->advance(1'000'000'000);
        require(p->tick==tick&&same(*p->authorization,saved),"pause/live aim changed decision");
        p->single_step();require(p->tick==tick+1&&same(*p->authorization,saved),"step changed decision");p->toggle_pause();
    }
    while(a.phase==PreviewPhase::Playing) {
        a.advance(4'166'667);b.advance(4'166'667);
        require(a.tick==b.tick&&a.batter.pose.world==b.batter.pose.world&&a.batter.barrel_world==b.batter.barrel_world,"aim changed bat path");
        require(same(a.delivery.pitch.current.position_m,b.delivery.pitch.current.position_m)
            &&same(a.delivery.pitch.current.velocity_mps,b.delivery.pitch.current.velocity_mps),"aim changed ball path");
        require(a.geometry==b.geometry&&bool(a.contact)==bool(b.contact),"aim changed geometry");
        require(same(*a.authorization,saved_a)&&same(*b.authorization,saved_b),"decision mutated");
    }
    require(a.geometry==ManualGeometry::Contact&&b.geometry==ManualGeometry::Contact,"441 Compact must contact");
    same_contact(*a.contact,*b.contact);const auto event=*a.contact;
    std::cout<<std::setprecision(12)<<"Compact 441 Authorized + Contact and RejectedSpatial + Contact: P=("<<point.x<<','<<point.y
        <<") rejected aim=("<<outside.x<<','<<outside.y<<") q="<<saved_b.q<<" time="<<event.sample.preview_time_s
        <<" u="<<event.sample.approach.u<<" normal=("<<event.normal.x<<','<<event.normal.y<<','<<event.normal.z
        <<") relative_velocity=("<<event.relative_velocity.x<<','<<event.relative_velocity.y<<','<<event.relative_velocity.z<<"); exact equality\n";
    for(auto aim:{point,outside})for(std::uint64_t commit:{441ull,456ull})for(unsigned hz:{30u,60u,120u,0u}) {
        a.start();require(!a.authorization,"new ball retained decision");a.record_command(commit,aim);
        if(!hz){a.advance(3'000'000'000);while(a.pending_ticks)a.advance(0);}
        while(a.phase==PreviewPhase::Playing)a.advance(hz?1'000'000'000/hz:16'666'667);
        require(a.committed->consumed_tick==commit&&same(*a.authorization,authorize_normal_hit(aim,point,t)),"cadence changed consumed decision");
        require(a.geometry==(commit==441?ManualGeometry::Contact:ManualGeometry::NoContactInWindow),"fixture geometry");
        if(commit==441)same_contact(*a.contact,event);
    }
    // Live controller changes after scheduling and after consumption cannot alter the saved command.
    a.start();live.recenter();const auto snapshot=live.center();a.record_command(441,snapshot);
    live.move(1,0,.05);until(a,441);const auto decision=*a.authorization;
    live.move(0,-1,.05);a.input_boundary(false,false,true,live.center());until(a,442);
    require(same(a.committed->aim_center,snapshot)&&same(*a.authorization,decision),"live controller leaked into command");
    a.reset();require(!a.authorization,"reset retained decision");
    // Existing boundary/backlog scheduling: 420 + 24 elapsed ticks schedules 445, never 437.
    a.start();until(a,420);a.advance(100'000'000);
    require(a.tick==436&&a.pending_ticks==8,"backlog fixture");
    a.input_boundary(false,false,true,point);a.input_boundary(true,true,true,point);
    require(a.pending&&a.pending->target_tick==445&&!a.authorization,"backlog scheduling");
    a.advance(0);require(a.tick==444&&!a.authorization,"decision consumed old debt");until(a,445);
    require(a.committed->consumed_tick==445&&a.authorization->q==0,"backlog consumption");
    require(same(*a.authorization,authorize_normal_hit(point,point,t)),"backlog pitch truth");
    a.reset();a.start();while(a.phase==PreviewPhase::Playing)a.advance(16'666'667);
    require(!a.authorization&&a.geometry==ManualGeometry::NoSwing,"Take authorization");
    std::cout<<"PASS Normal S1: boundary/signs, immutable snapshot, pause/step/reset, 30/60/120 Hz/backlog, physical invariance; Compact 456 Authorized + NoContactInWindow\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
