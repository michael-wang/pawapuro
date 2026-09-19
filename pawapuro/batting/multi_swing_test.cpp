#include "manual_swing.hpp"
#include "player_aim.hpp"
#include <iostream>
#include <stdexcept>
using namespace pawapuro;
void require(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
void until(ManualSwingPreview& p,std::uint64_t t){while(p.tick<t){p.advance(4'166'667);require(p.phase==PreviewPhase::Playing||p.tick==t,"unexpected completion");}}
bool same(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b){return a.x==b.x&&a.y==b.y&&a.z==b.z;}
void compare(const SwingAttempt& a,const SwingAttempt& b){
 require(a.command.consumed_tick==b.command.consumed_tick&&a.command.aim_center.x==b.command.aim_center.x&&a.command.aim_center.y==b.command.aim_center.y,"command/snapshot changed");
 require(a.timing.state==b.timing.state&&a.timing.efficiency==b.timing.efficiency&&a.timing.offset_ms==b.timing.offset_ms&&a.timing.start_s==b.timing.start_s&&a.timing.end_s==b.timing.end_s,"timing changed");
 require(a.authorization.authorized==b.authorization.authorized&&a.authorization.q==b.authorization.q&&a.authorization.error.x==b.authorization.error.x&&a.authorization.error.y==b.authorization.error.y&&a.authorization.normalized_error.x==b.authorization.normalized_error.x&&a.authorization.normalized_error.y==b.authorization.normalized_error.y,"spatial changed");
 require(a.geometry==b.geometry&&bool(a.contact)==bool(b.contact)&&a.contact_count==b.contact_count&&a.contact_dispatch_tick==b.contact_dispatch_tick&&a.contact_query_count==b.contact_query_count,"geometry changed");
 if(a.contact){const auto& x=*a.contact;const auto& y=*b.contact;
 require(x.sample.preview_time_s==y.sample.preview_time_s&&x.sample.approach.u==y.sample.approach.u&&same(x.normal,y.normal)&&same(x.relative_velocity,y.relative_velocity)&&x.ball_radius_m==y.ball_radius_m&&x.bat_radius_m==y.bat_radius_m,"physical event changed");}
}
int main(int argc,char** argv){try{
 require(argc==2,"expected batting directory");const std::filesystem::path d=argv[1];const auto s=load_batting_staging(d/"staging.toml");
 ManualSwingPreview p(d,s),fresh(d,s);PlayerAim aim(s);for(int n=0;n<10;++n)aim.move(-1,1,.05);const auto first_aim=aim.center();
 p.start();fresh.start();require(p.record_command(80,first_aim),"early command");aim.recenter();const auto center=aim.center();fresh.record_command(448,center);
 until(p,80);const auto saved=p.attempts[0];require(saved.timing.state==SwingTimingState::Early&&saved.geometry==ManualGeometry::NoContactInWindow&&!saved.authorization.authorized,"early rejected-spatial fixture");
 until(p,319);until(fresh,319);p.input_boundary(false,false,true,center);p.input_boundary(true,true,true,center);require(!p.pending&&p.attempts.size()==1,"active press buffered");
 until(p,320);until(fresh,320);require(!p.active_attempt&&p.rearmed()&&p.batter.pose.world==fresh.batter.pose.world,"hard reset not same-world preparation");
 require(p.delivery.motion.pose.world==fresh.delivery.motion.pose.world&&same(p.delivery.ball_center(),fresh.delivery.ball_center()),"rearm reset pitch");
 p.input_boundary(true,false,true,center);p.input_boundary(true,true,true,center);require(!p.pending,"held repeat across rearm");
 until(p,447);until(fresh,447);p.input_boundary(false,false,true,center);p.input_boundary(true,true,true,center);
 require(p.pending&&p.pending->target_tick==448,"fresh second input rejected");aim.move(1,0,.05);
 for(std::uint64_t t=448;t<=494;++t){until(p,t);until(fresh,t);
 require(p.batter.pose.world==fresh.batter.pose.world&&p.batter.grip_world==fresh.batter.grip_world&&p.batter.barrel_world==fresh.batter.barrel_world&&p.batter.tip_world==fresh.batter.tip_world,"first swing contaminated second pose");
 require(same(p.delivery.ball_center(),fresh.delivery.ball_center()),"second swing changed ball");}
 require(p.attempts.size()==2&&p.attempts[1].contact,"second contact missing");compare(p.attempts[0],saved);compare(p.attempts[1],fresh.attempts[0]);
 const auto expected=p.attempts[1];require(expected.contact->ball_radius_m==.037f&&expected.contact->bat_radius_m==.033f,"radii changed");
 require(double(expected.command.tempo.end_tick(448))/240>=p.ball_passage.exit_s,"STOP: Authorized Contact can rearm before passage exit");
 while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);compare(p.attempts[0],saved);compare(p.attempts[1],expected);
 for(auto mode:{SwingTempo::Original,SwingTempo::Compact})for(unsigned hz:{30u,60u,120u,0u}){
 p.reset();fresh.reset();p.next_tempo=fresh.next_tempo=mode;p.start();fresh.start();p.record_command(80,first_aim);fresh.record_command(448,center);
 while(p.tick<320){p.advance(hz?1'000'000'000/hz:100'000'000);if(!hz)while(p.pending_ticks)p.advance(0);}
 require(p.rearmed()&&!p.toggle_tempo()&&p.attempt_tempo.mode==mode,"tempo unlocked at rearm");p.record_command(448,center);
 while(p.phase==PreviewPhase::Playing)p.advance(hz?1'000'000'000/hz:100'000'000);
 while(fresh.phase==PreviewPhase::Playing)fresh.advance(16'666'667);
 require(p.attempts.size()==2&&p.tick==fresh.tick&&p.batter.pose.world==fresh.batter.pose.world,"replay changed completion");compare(p.attempts[1],fresh.attempts[0]);
 if(mode==SwingTempo::Compact)compare(p.attempts[1],expected);
 }
 p.reset();p.start();p.record_command(80,first_aim);until(p,420);p.advance(100'000'000);
 p.input_boundary(false,false,true,center);p.input_boundary(true,true,true,center);require(p.pending&&p.pending->target_tick==445&&p.pending->backlog==8,"second scheduling lost debt");p.advance(0);until(p,445);require(p.attempts[1].command.consumed_tick==445,"second consumed wrong target");
 p.reset();p.start();until(p,479);p.input_boundary(false,false,true,center);p.input_boundary(true,true,true,center);until(p,480);
 require(p.attempts.size()==1&&p.timing()->state==SwingTimingState::Late,"last live target rejected or clamped");
 until(p,720);require(p.batter.complete()&&!p.active_attempt&&p.geometry()==ManualGeometry::NoContactInWindow,"last live swing truncated after exit");
p.reset();p.start();until(p,480);require(!p.record_command(481,center),"post-exit target accepted");
 p.reset();p.start();until(p,450);p.advance(150'000'000);require(p.tick==466&&p.pending_ticks==20,"debt fixture");p.input_boundary(false,false,true,center);p.input_boundary(true,true,true,center);require(!p.pending,"backlogged target beyond exit accepted");
 p.reset();p.start();p.record_command(80,center);until(p,320);p.input_boundary(false,false,true,center);p.input_boundary(true,true,true,center);p.lose_input();require(!p.pending,"focus loss retained second command");p.input_boundary(true,true,true,center);require(!p.pending,"focus rearmed held key");p.toggle_pause();p.input_boundary(false,false,true,center);p.input_boundary(true,true,true,center);require(!p.pending,"paused second input accepted");
 // No maximum of two: a slower test pitch permits four complete independent intents.
 auto slow=s;slow.reference_velocity_mps.z=-10;ManualSwingPreview longer(d,slow);longer.start();
 for(std::uint64_t c:{1ull,242ull,483ull,724ull}){until(longer,c-1);require(longer.record_command(c,center),"arbitrary attempt limit");until(longer,c);}
 require(longer.attempts.size()==4,"not 0..N attempts");
 p.reset();p.start();while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);require(p.attempts.empty()&&p.geometry()==ManualGeometry::NoSwing,"zero-swing result");
 std::cout<<"PASS early80 -> rearm320 -> Compact448: exact fresh-single pose/event; independent aims/decisions; held/pause/focus/debt/cutoff; 30/60/120/backlog; four attempts on slow test pitch\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
