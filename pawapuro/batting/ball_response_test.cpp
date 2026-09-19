#include "manual_swing.hpp"
#include "player_aim.hpp"
#include "contact_review.hpp"
#include <fstream>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace pawapuro;
void require(bool b,const char* m){if(!b)throw std::runtime_error(m);}
bool close_enough(double a,double b,double e=1e-6){return std::abs(a-b)<=e;}
bool same(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b){return a.x==b.x&&a.y==b.y&&a.z==b.z;}
void finish(ManualSwingPreview& p){while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);}
// Diagnostic of today's model, not accepted future vertical-contact balance.
std::string vertical_contact_study(const ManualSwingPreview& p) {
 const auto& s=p.tuning;const auto& tuning=s.ball_response;const auto& timing=p.latest()->timing;
 const auto region=normal_authorization_region(s);
 require(p.latest()->command.consumed_tick==448&&s.batter_profile.trajectory==3,"study fixture changed");
 std::ostringstream out;out<<std::setprecision(12);
 out<<"# Current production diagnostic only; no future angle/family contract\n"
    <<"# Michael Contact="<<s.batter_profile.contact<<" Power="<<s.batter_profile.power<<" Trajectory="<<s.batter_profile.trajectory
    <<" commit=448 efficiency="<<timing.efficiency<<" offset_ms="<<timing.offset_ms<<'\n'
    <<"# rx="<<region.normal_radius_x_m<<" ry="<<region.normal_radius_y_m
    <<" max_spray_deg="<<tuning.max_spray_degrees<<" full_spray_offset_ms="<<tuning.full_spray_offset_ms<<'\n'
    <<"# Authorization evaluated in pitch-relative XY coordinates to represent +/-ry exactly at q=1\n"
    <<"requested_ey,ey,q,spatial_transfer,exit_speed_mps,longitudinal_deg,spray_deg,vx_mps,vy_mps,vz_mps,longitudinal,vertical\n";
 for(float ey:{-1.f,-.95f,-.90f,-.75f,-.50f,-.10f,0.f,.10f,.50f,.75f,.90f,.95f,1.f}) {
  const DirectX::XMFLOAT2 aim{0,-ey*region.normal_radius_y_m};
  const auto auth=authorize_normal_hit(aim,{0,0},region);
  require(auth.authorized&&auth.error.y==-aim.y&&auth.normalized_error.x==0&&close_enough(auth.normalized_error.y,ey),"study signed ey semantics");
  require((ey==0&&auth.error.y==0)||(ey<0&&aim.y>0&&auth.error.y<0)||(ey>0&&aim.y<0&&auth.error.y>0),"study upper/lower sign inverted");
  const auto response=ball_response(timing,auth,s.batter_profile,tuning,p.delivery.pitch.initial,double(p.delivery.motion.release_tick)/240);
  require(response.has_value(),"study authorized overlap did not launch");const auto& r=*response;
  const float elevation=DirectX::XMConvertToRadians(r.longitudinal_angle_deg),spray=DirectX::XMConvertToRadians(r.spray_angle_deg);
  require(same(r.launch_velocity_mps,{r.exit_speed_mps*std::abs(std::cos(elevation))*std::sin(spray),r.exit_speed_mps*std::sin(elevation),r.exit_speed_mps*std::cos(elevation)*std::cos(spray)}),"current velocity decomposition changed; revisit study");
  // Report direction; never assert forward-only as a desired gameplay invariant.
  const auto v=r.launch_velocity_mps;
  out<<ey<<','<<auth.normalized_error.y<<','<<auth.q<<','<<r.spatial_transfer<<','<<r.exit_speed_mps<<','<<r.longitudinal_angle_deg<<','<<r.spray_angle_deg<<','<<v.x<<','<<v.y<<','<<v.z<<','
     <<(v.z>0?"Forward field (+Z)":v.z<0?"Back home/catcher (-Z)":"Zero longitudinal")<<','<<(v.y>0?"Up":v.y<0?"Down":"Flat")<<'\n';
 }
 return out.str();
}
int main(int argc,char** argv){try{
 require(argc==3,"expected batting/fixture directories");const std::filesystem::path d=argv[1],temp=argv[2];
 const auto s=load_batting_staging(d/"staging.toml");
 require(s.batter_profile.contact==75&&s.batter_profile.power==85&&s.batter_profile.trajectory==3,"Michael baseline");
 auto region=normal_authorization_region(s);require(region.normal_radius_x_m==.26f&&region.normal_radius_y_m==.13f,"Contact75 anchor changed");
 float prior_q=100;for(int contact:{0,75,120}){
  auto other=s;other.batter_profile.contact=contact;const auto r=normal_authorization_region(other);
  require(close_enough(r.normal_radius_x_m,.26*(.5+contact/150.))&&close_enough(r.normal_radius_y_m,.13*(.5+contact/150.)),"Contact scale");
  const auto normalized=authorize_normal_hit({0,0},{r.normal_radius_x_m*.5f,r.normal_radius_y_m*.5f},r);
  require(close_enough(normalized.q,.5)&&close_enough(spatial_transfer(normalized.q,s.ball_response),spatial_transfer(.5f,s.ball_response)),"relative quality depends on metres");
  PlayerAim visual(other);const auto vc=visual.center();const auto diagnostic=visual.diagnostic({vc.x+r.normal_radius_x_m*.5f,vc.y+r.normal_radius_y_m*.5f,0});
  require(close_enough(diagnostic.q,.5),"visual aim uses a different authorization region");
  const auto absolute=authorize_normal_hit({0,0},{.10f,.04f},r);require(absolute.q<prior_q,"Contact did not reduce same-metre q");prior_q=absolute.q;
 }
 float last=1;for(int i=0;i<=100;++i){const float transfer=spatial_transfer(float(i)/100,s.ball_response);
  require(std::isfinite(transfer)&&transfer>=0&&transfer<=last,"nonmonotonic spatial transfer");last=transfer;}
 require(spatial_transfer(0,s.ball_response)==1&&close_enough(spatial_transfer(1,s.ball_response),.35),"spatial endpoints");
 ManualSwingPreview p(d,s);const auto pp=predict_arrival(p.delivery.pitch).state.position_m;const DirectX::XMFLOAT2 center{pp.x,pp.y};
 const auto run=[&](std::uint64_t c,float ex=0,float ey=0){p.reset();p.start();require(p.record_command(c,{center.x-ex*region.normal_radius_x_m,center.y-ey*region.normal_radius_y_m}),"fixture rejected");finish(p);return p.attempts.back();};
 std::cout<<std::setprecision(12);
 BallResponse peak{};
 for(auto c:{80ull,433ull,448ull,456ull,460ull,480ull}){
  const auto a=run(c);if(c==80||c==480){require(!a.response&&!p.flight&&a.gameplay==GameplayResult::Miss,"no-overlap launched");continue;}
  require(a.response&&p.flight&&a.gameplay==GameplayResult::Contact,"authorized overlap failed to launch");const auto& r=*a.response;
  if(c==456)require(a.geometry==ManualGeometry::NoContactInWindow&&!a.contact,"456 raw-NoContact premise changed");
  if(c==448){require(a.contact&&r.temporal_transfer>.97f&&r.spatial_transfer==1&&close_enough(r.longitudinal_angle_deg,0)&&std::abs(r.spray_angle_deg)<1,"peak fixture");peak=r;}
  if(c==433)require(r.spray_angle_deg>0&&r.launch_velocity_mps.x>0,"left-handed early pull sign");
  if(c==460)require(r.spray_angle_deg<0&&r.launch_velocity_mps.x<0,"late opposite sign");
  require(r.launch_velocity_mps.z>0&&r.contact_time_s==std::clamp(a.timing.peak_s,a.timing.overlap->start_s,a.timing.overlap->end_s),"effective time/center field");
  require(same(r.launch_position_m,sample_reference_pitch(p.delivery.pitch.initial,r.contact_time_s-double(p.delivery.motion.release_tick)/240).position_m),"launch origin not pitch truth");
  require(a.gameplay_dispatch_tick==static_cast<std::uint64_t>(std::ceil(r.contact_time_s*240)),"gameplay dispatched at wrong tick");
  require(p.tick==p.completion_tick(),"completion did not wait for ground/animations");
  const auto ground=p.flight->sample(p.flight->ground_s);require(ground.position_m.y==.037f&&same(ground.velocity_mps,{})&&same(ground.position_m,p.flight->sample(p.flight->ground_s+10).position_m),"ground hold/bounce");
  require(p.flight->sample(p.flight->ground_s-1e-5).position_m.y>.037f,"wrong ground root");
  std::cout<<"RESPONSE commit="<<c<<" raw="<<p.contact_state()<<" efficiency="<<a.timing.efficiency<<" offset_ms="<<a.timing.offset_ms<<" effective_s="<<r.contact_time_s<<" speed="<<r.exit_speed_mps<<" kmh="<<r.exit_speed_mps*3.6<<" longitudinal="<<r.longitudinal_angle_deg<<" spray="<<r.spray_angle_deg<<" ground_s="<<p.flight->ground_s<<'\n';
 }
 const float ideal=20+(55-20)*(85.f/120);require(close_enough(peak.exit_speed_mps,ideal*(.2f+.8f*std::sqrt(peak.energy_transfer)),1e-5),"response speed curve");
 const auto rejected=run(448,1.01f);require(rejected.contact&&!rejected.response&&!p.flight&&rejected.gameplay==GameplayResult::Miss,"raw contact overrode spatial rejection");
 const auto up=run(448,0,.8f),down=run(448,0,-.8f);require(up.response&&down.response&&up.response->longitudinal_angle_deg>peak.longitudinal_angle_deg&&down.response->longitudinal_angle_deg<peak.longitudinal_angle_deg&&close_enough(up.response->exit_speed_mps,down.response->exit_speed_mps,1e-5),"vertical bias/q sign");
 std::cout<<"VERTICAL ey=+0.8 longitudinal="<<up.response->longitudinal_angle_deg<<" ey=0 longitudinal="<<peak.longitudinal_angle_deg<<" ey=-0.8 longitudinal="<<down.response->longitudinal_angle_deg<<" paired_speed="<<up.response->exit_speed_mps<<'\n';
 const auto left=run(448,.8f),right=run(448,-.8f);require(left.response->spray_angle_deg==right.response->spray_angle_deg&&close_enough(left.response->exit_speed_mps,right.response->exit_speed_mps),"ex steered spray");
 auto a=run(448);
 const auto vertical_response=[&](float ey,double offset,int trajectory_value=3,int power_value=85) {
  auto t=a.timing;t.offset_ms=offset;
  auto b=s.batter_profile;b.trajectory=trajectory_value;b.power=power_value;
  const auto auth=authorize_normal_hit({0,-ey*region.normal_radius_y_m},{0,0},region);
  return *ball_response(t,auth,b,s.ball_response,p.delivery.pitch.initial,1.6);
 };
 const std::array<float,7> anchors{-1,-.75f,-.5f,0,.5f,.75f,1},angles{-150,-35,-15,0,25,45,130};
 require(s.ball_response.vertical_contact_longitudinal_degrees==angles,"authored S0 anchor candidate");
 for(std::size_t i=0;i<anchors.size();++i){
  require(vertical_contact_longitudinal_angle(anchors[i],s.ball_response)==angles[i],"exact vertical anchor");
  if(i)require(vertical_contact_longitudinal_angle((anchors[i]+anchors[i-1])/2,s.ball_response)==(angles[i]+angles[i-1])/2,"linear midpoint");
 }
 require(vertical_contact_longitudinal_angle(-2,s.ball_response)==-150&&vertical_contact_longitudinal_angle(2,s.ball_response)==130,"defensive ey clamp");
 require(close_enough(vertical_contact_longitudinal_angle(-.1f,s.ball_response),-3)&&vertical_contact_longitudinal_angle(0,s.ball_response)==0&&close_enough(vertical_contact_longitudinal_angle(.1f,s.ball_response),5),"line-drive neighborhood");
 // Human-found upper-half contact previously became an airborne +10.112 degrees.
 const auto upper_half_bug=vertical_response(-.412f,0);
 require(close_enough(upper_half_bug.longitudinal_angle_deg,-12.36)&&upper_half_bug.launch_velocity_mps.y<0&&upper_half_bug.launch_velocity_mps.z>0,"human ey=-0.412 grounder regression");
 for(float ey:{-1.f,-.9f,-.75f,-.5f,-.1f,0.f,.1f,.5f,.75f,.9f,1.f}) {
  const auto response=vertical_response(ey,0),paired=vertical_response(-ey,0);
  require(response.exit_speed_mps==paired.exit_speed_mps&&response.spatial_transfer==paired.spatial_transfer,"equal-q speed changed");
  for(double offset:{-30.,0.,30.}) {
   const auto r=vertical_response(ey,offset);const auto v=r.launch_velocity_mps;
   require((std::abs(ey)>=.9f?v.z<0:v.z>0)&&(ey<0?v.y<0:ey>0?v.y>0:v.y==0),"vertical direction topology");
   require(offset<0?v.x>0:offset>0?v.x<0:v.x==0,"backward topology reversed timing lateral sign");
   require(close_enough(std::sqrt(double(v.x)*v.x+double(v.y)*v.y+double(v.z)*v.z),r.exit_speed_mps,1e-5),"direction not unit length");
   require(r.exit_speed_mps==response.exit_speed_mps&&r.longitudinal_angle_deg==response.longitudinal_angle_deg,"spray altered vertical/speed");
   for(int tr=1;tr<=4;++tr){const auto neutral=vertical_response(ey,offset,tr);
    require(same(neutral.launch_velocity_mps,v)&&neutral.exit_speed_mps==r.exit_speed_mps,"temporary S0 Trajectory neutrality");}
   const auto stronger=vertical_response(ey,offset,3,120);
   require(stronger.exit_speed_mps>r.exit_speed_mps&&stronger.longitudinal_angle_deg==r.longitudinal_angle_deg&&stronger.spray_angle_deg==r.spray_angle_deg,"Power changed direction");
  }
  const auto& r=response;const auto& tuning=s.ball_response;
  const float expected_ideal=std::lerp(tuning.ideal_exit_speed_min_mps,tuning.ideal_exit_speed_max_mps,85.f/120);
  require(r.temporal_transfer==static_cast<float>(a.timing.efficiency)&&r.energy_transfer==r.temporal_transfer*r.spatial_transfer&&r.exit_speed_mps==expected_ideal*(tuning.minimum_exit_speed_factor+(1-tuning.minimum_exit_speed_factor)*std::sqrt(r.energy_transfer))&&r.contact_time_s==a.response->contact_time_s&&same(r.launch_position_m,a.response->launch_position_m),"direction changed quality/speed/effective time/origin");
 }
 // Backward flight uses the same analytic sampler and render/backlog clock contract.
 const DirectX::XMFLOAT2 backward_aim{center.x,center.y-.95f*region.normal_radius_y_m};
 p.reset();p.start();p.record_command(448,backward_aim);std::vector<DirectX::XMFLOAT3> backward_history{p.displayed_ball_center()};
 while(p.phase==PreviewPhase::Playing){p.advance(4'166'667);backward_history.push_back(p.displayed_ball_center());}
 require(p.flight&&p.latest()->response->launch_velocity_mps.z<0,"backward replay fixture");
 for(unsigned hz:{30u,60u,120u,0u}){p.start();p.record_command(448,backward_aim);
  while(p.phase==PreviewPhase::Playing){p.advance(hz?1'000'000'000/hz:100'000'000);require(same(p.displayed_ball_center(),backward_history.at(static_cast<std::size_t>(p.tick))),"backward flight cadence");}}
 a=run(448);
 const auto study_tick=p.tick;const auto study_ball=p.displayed_ball_center();
 std::vector<unsigned char> study_attempt(sizeof(SwingAttempt));std::memcpy(study_attempt.data(),p.latest(),study_attempt.size());
 const auto table=vertical_contact_study(p);
 require(table==vertical_contact_study(p),"vertical study not deterministic");
 require(p.tick==study_tick&&same(study_ball,p.displayed_ball_center())&&std::memcmp(study_attempt.data(),p.latest(),study_attempt.size())==0,"study mutated production state");
 std::filesystem::create_directories(temp);
 {std::ofstream output(temp/"current-response-table.csv");output<<table;require(bool(output),"study artifact write failed");}
 auto profile=s.batter_profile;profile.power=120;
 auto power=ball_response(a.timing,a.authorization,profile,s.ball_response,p.delivery.pitch.initial,1.6);
 require(power->exit_speed_mps>peak.exit_speed_mps&&power->longitudinal_angle_deg==peak.longitudinal_angle_deg&&power->spray_angle_deg==peak.spray_angle_deg,"Power independence");
 profile=s.batter_profile;profile.trajectory=4;auto trajectory=ball_response(a.timing,a.authorization,profile,s.ball_response,p.delivery.pitch.initial,1.6);
 require(trajectory->longitudinal_angle_deg==0&&trajectory->exit_speed_mps==peak.exit_speed_mps&&trajectory->spray_angle_deg==peak.spray_angle_deg,"Trajectory independence");
 // Any strictly positive overlap qualifies; there is no hidden efficiency threshold.
 auto tiny=a.timing;tiny.efficiency=1e-12;require(ball_response(tiny,a.authorization,profile,s.ball_response,p.delivery.pitch.initial,1.6).has_value(),"tiny match rejected");
 p.reset();p.start();p.record_command(448,center);while(p.tick<477)p.advance(4'166'667);
 require(p.latest()->response&&!p.flight&&p.gameplay_result()==GameplayResult::Pending,"launch dispatched before effective time");
 const auto before=p.displayed_ball_center();p.toggle_pause();p.advance(1'000'000'000);
 require(p.tick==477&&!p.flight&&same(before,p.displayed_ball_center()),"pause advanced planned response");
 p.single_step();require(p.tick==478,"single step failed");
 require(p.flight&&p.gameplay_result()==GameplayResult::Contact,"single step did not dispatch response");
 p.reset();require(!p.flight&&p.attempts.empty()&&p.gameplay_result()==GameplayResult::Pending,"reset retained response");
 // Accepted early miss -> rearm -> second contact, then close all new intent.
 p.reset();p.start();p.record_command(80,center);std::vector<DirectX::XMFLOAT3> history{p.displayed_ball_center()};
 while(p.phase==PreviewPhase::Playing){p.advance(4'166'667);history.push_back(p.displayed_ball_center());
  if(p.tick==320){require(p.rearmed(),"early miss did not rearm");p.record_command(448,center);}
  if(p.tick==478){require(p.flight&&p.attempts[1].gameplay==GameplayResult::Contact&&!p.intent_live(479),"contact did not close input before exit");require(!p.record_command(479,center),"third attempt queued");}
 }
 const auto final_tick=p.tick;const auto ground=p.displayed_ball_center();
 for(unsigned hz:{30u,60u,120u,0u}){
  p.reset();p.start();p.record_command(80,center);bool second=false;
  while(p.phase==PreviewPhase::Playing){p.advance(hz?1'000'000'000/hz:100'000'000);
   require(same(p.displayed_ball_center(),history.at(static_cast<std::size_t>(p.tick))),"render cadence changed sampled flight");
   if(!second&&p.tick>=320){require(!p.active_attempt&&p.record_command(448,center),"replay second command");second=true;}
  }
  require(p.tick==final_tick&&same(p.displayed_ball_center(),ground)&&p.attempts.size()==2&&p.latest()->response->exit_speed_mps==peak.exit_speed_mps,"cadence changed launch/ground/completion");
 }
 // Result review has no timer: all completed attempt bytes and final poses remain owned until Space/start.
 const auto held_tick=p.tick,held_count=p.attempts.size();const auto held_batter=p.batter.pose.world,held_pitcher=p.delivery.motion.pose.world;
 const auto held_ball=p.displayed_ball_center();const auto held_flight=*p.flight;
 std::vector<unsigned char> held_attempts(sizeof(SwingAttempt)*held_count);
 std::memcpy(held_attempts.data(),p.attempts.data(),held_attempts.size());
 for(unsigned n=0;n<20;++n){p.advance(60'000'000'000);p.single_step();p.input_boundary(true,true,true,{.4f,1.f});
  require(p.phase==PreviewPhase::Complete&&p.tick==held_tick&&p.attempts.size()==held_count&&p.gameplay_result()==GameplayResult::Contact,"result hold advanced/reset");
  require(p.batter.pose.world==held_batter&&p.delivery.motion.pose.world==held_pitcher&&same(p.displayed_ball_center(),held_ball),"final poses/ball changed");
  require(std::memcmp(held_attempts.data(),p.attempts.data(),held_attempts.size())==0&&p.flight->start_s==held_flight.start_s&&p.flight->ground_s==held_flight.ground_s&&same(p.flight->initial.position_m,held_flight.initial.position_m)&&same(p.flight->initial.velocity_mps,held_flight.initial.velocity_mps),"review truth changed during hold");
 }
 ManualSwingPreview fresh(d,s);fresh.start();
 const auto clean_restart=[&]{require(p.can_restart_after_contact()&&p.start(),"Space did not restart contacted pitch");
  require(p.phase==PreviewPhase::Playing&&p.tick==0&&p.delivery.tick==0&&p.delivery.pitch.tick==0&&p.batter.tick==0&&!p.paused,"restart did not start at tick zero");
  require(p.attempts.empty()&&!p.flight&&!p.pending&&!p.active_attempt&&!p.rearmed()&&!p.swing_available()&&p.pending_ticks==0&&p.arrival_tick==0&&p.gameplay_result()==GameplayResult::Pending&&!p.can_restart_after_contact(),"restart retained old result/input");
  require(p.batter.pose.world==fresh.batter.pose.world&&p.delivery.motion.pose.world==fresh.delivery.motion.pose.world&&same(p.displayed_ball_center(),fresh.displayed_ball_center())&&same(p.delivery.pitch.current.position_m,fresh.delivery.pitch.current.position_m)&&same(p.delivery.pitch.current.velocity_mps,fresh.delivery.pitch.current.velocity_mps),"restart pose/pitch differs from fresh start");
  p.advance(4'166'666);require(p.tick==0,"restart retained fractional debt");p.advance(1);require(p.tick==1,"fresh clock failed");
 };
 clean_restart(); // Complete after the accepted two-swing sequence.
 const auto until=[&](std::uint64_t tick){while(p.tick<tick)p.advance(4'166'667);};
 for(auto target:{478ull,520ull}){p.reset();p.start();p.record_command(448,center);until(target);
  require(p.flight&&!p.flight->complete(double(p.tick)/240)&&p.active_attempt,"airborne/follow-through fixture");
  const auto saved_aim=p.committed()->aim_center;const auto saved_response=*p.latest()->response;
  p.input_boundary(false,false,true,{});p.input_boundary(true,true,true,{.4f,1.f});require(p.attempts.size()==1&&!p.pending&&!p.toggle_tempo(),"contact allowed new intent/tempo");
  require(p.committed()->aim_center.x==saved_aim.x&&p.committed()->aim_center.y==saved_aim.y&&p.latest()->response->exit_speed_mps==saved_response.exit_speed_mps,"live input changed response snapshot");
  if(target==478){p.toggle_pause();p.advance(1'000'000'000);require(p.tick==478,"post-contact pause failed");p.single_step();require(p.tick==479,"post-contact step failed");}
  else p.advance(100'000'001); // Also reset nonzero backlog and fractional credit.
  clean_restart();
 }
 p.reset();p.start();p.record_command(433,center);until(477);require(p.flight.has_value(),"433 contact fixture");
 until(static_cast<std::uint64_t>(std::ceil(p.flight->ground_s*240)));
 require(p.phase==PreviewPhase::Playing&&p.flight->complete(double(p.tick)/240)&&p.delivery.phase!=DeliveryPhase::Complete,"ground-before-pitcher-completion fixture");clean_restart();
 p.reset();p.start();until(100);require(!p.start()&&p.tick==100,"ordinary Playing restarted");
 p.record_command(448,center);until(477);require(p.latest()->response&&!p.flight&&!p.start()&&p.tick==477,"planned response enabled early restart");
 p.toggle_pause();require(!p.start()&&p.paused&&p.tick==477,"pre-contact pause enabled restart");
 p.reset();p.start();p.record_command(80,center);until(90);require(p.gameplay_result()==GameplayResult::Miss&&!p.start()&&p.tick==90,"miss enabled restart");
 until(320);require(p.rearmed()&&!p.start()&&p.tick==320,"rearmed miss enabled restart");
 p.record_command(448,center);until(478);require(p.attempts.size()==2&&p.flight&&!p.intent_live(479),"second contact did not close intent");clean_restart();
 p.reset();p.start();p.record_command(448,{center.x+region.normal_radius_x_m*1.01f,center.y});until(481);
 require(p.contact()&&!p.flight&&!p.start()&&p.tick==481,"raw overlap authorized restart");
 std::cout<<"PASS result hold, pre-contact rejection, airborne/follow-through/ground/Complete restart and clean tick-zero lifecycle\n";
 // The same presentation append used by main always renders live next-pitch intent.
 PlayerAim live(s);std::vector<engine::Vertex> review;review.reserve(contact_review_vertex_count);
 static_assert(contact_review_vertex_count==PlayerAim::vertex_count);
 const auto cue_prediction=predict_arrival(p.delivery.pitch);
 const auto scene=make_batting_reference(s,cue_prediction.state.position_m,16.f/9,{});
 std::vector<engine::Vertex> cue;cue.reserve(arrival_cue_vertex_count);
 const auto* cue_storage=cue.data();const auto cue_capacity=cue.capacity();
 const auto check_cue=[&]{
  const auto phase=p.delivery.pitch.phase;
  const bool visible=contact_review_arrival_visible(p);
  require(visible==(arrival_cue_visible(phase)||p.gameplay_result()==GameplayResult::Contact),"cue lifecycle changed");
  for(auto style:{ArrivalCueStyle::Baseball,ArrivalCueStyle::Ring}) {
   cue.clear();append_arrival_cue(cue,scene,visible,style);
   require(cue.size()==arrival_cue_vertex_count&&cue.data()==cue_storage&&cue.capacity()==cue_capacity,"cue count/allocation changed");
   if(visible){const auto& expected=style==ArrivalCueStyle::Baseball?scene.arrival_baseball:scene.arrival_ring;
    require(std::memcmp(cue.data(),expected.data(),cue.size()*sizeof(engine::Vertex))==0,"persisted cue moved/restyled");
   }else for(const auto& v:cue)require(same(v.position,{}),"hidden cue remained visible");
  }
  require(p.delivery.pitch.phase==phase,"presentation changed pitch phase");
 };
 const auto* storage=review.data();const auto capacity=review.capacity();
 const auto render_review=[&]{review.clear();append_contact_review(review,live);
  require(review.size()==contact_review_vertex_count&&review.data()==storage&&review.capacity()==capacity,"review count/allocation changed");check_cue();};
 const auto check_live=[&]{render_review();std::vector<engine::Vertex> expected;live.append_triangles(expected);
  require(review.size()==expected.size()&&std::memcmp(review.data(),expected.data(),expected.size()*sizeof(engine::Vertex))==0,"reticle did not follow live aim");};
 check_live(); // Spatial miss, despite raw Contact.
 p.reset();check_live();p.start();check_live();until(384);check_live();
 require(contact_review_arrival_visible(p),"normal release cue hidden");
 finish(p);require(p.gameplay_result()==GameplayResult::NoSwing&&!contact_review_arrival_visible(p),"take persisted cue");check_live();
 p.start();p.record_command(80,{center.x+.3f,center.y});until(80);check_live();until(320);check_live();
 const DirectX::XMFLOAT2 review_aim{center.x-.13f,center.y-.065f};
 p.record_command(448,review_aim);until(477);require(p.latest()->response&&!p.flight,"planned review fixture");check_live();
 until(478);require(p.attempts.size()==2&&p.gameplay_result()==GameplayResult::Contact,"second contact fixture");render_review();
 std::vector<engine::Vertex> committed_geometry;live.append_triangles_at(committed_geometry,p.latest()->command.aim_center);
 check_live();
 require(std::memcmp(review.data(),committed_geometry.data(),review.size()*sizeof(engine::Vertex))!=0,"contact substituted historical aim for live reticle");
 const auto point=p.latest()->authorization.pitch_point;
 require(point.x==cue_prediction.state.position_m.x&&point.y==cue_prediction.state.position_m.y,"fixed-pitch cue prediction differs from authorization");
 const auto projected=project_batting_point(s,{point.x,point.y,s.strike_zone_plane_z()},16.f/9);
 require(scene.prediction_ndc.x==projected.x&&scene.prediction_ndc.y==projected.y,"scene cue not at authorization point");
 require(review.size()==committed_geometry.size(),"extra contact marker geometry");
 require(close_enough(p.latest()->authorization.q,.5)&&p.latest()->command.aim_center.x==review_aim.x&&p.latest()->command.aim_center.y==review_aim.y,"second attempt intent fixture");
 std::vector<unsigned char> immutable(sizeof(SwingAttempt));std::memcpy(immutable.data(),p.latest(),immutable.size());
 const auto control_flight=*p.flight;
 const auto check_history=[&]{check_live();
  require(contact_review_arrival_visible(p),"dispatched contact lost cue");
  require(std::memcmp(immutable.data(),p.latest(),immutable.size())==0,"live aim mutated committed truth");
  for(double time:{double(p.tick)/240,2.2,3.,4.,control_flight.ground_s,control_flight.ground_s+10}) {
   const auto expected=control_flight.sample(time),observed=p.flight->sample(time);
   require(same(expected.position_m,observed.position_m)&&same(expected.velocity_mps,observed.velocity_mps),"live aim changed outgoing flight");
  }
  require(same(p.displayed_ball_center(),control_flight.sample(double(p.tick)/240).position_m),"displayed flight changed with aim");
 };
 for(unsigned n=0;n<1000;++n){live.move(n%2?1.f:-1.f,1,.05);check_history();}
 require(!p.record_command(479,live.center()),"post-contact movement reopened third swing");
 live.recenter();require(live.center().x==0&&live.center().y==(s.strike_zone_bottom_m+s.strike_zone_top_m)/2,"post-contact R changed");check_history();
 p.toggle_pause();p.advance(5'000'000'000);live.move(1,0,.05);check_history();p.single_step();check_history();p.toggle_pause();
 while(p.phase==PreviewPhase::Playing){live.move(p.tick%2?1.f:-1.f,-1,.01);p.advance(16'666'667);check_history();}
 require(p.flight->complete(double(p.tick)/240),"review ground hold fixture");
 const auto actual=p.delivery.pitch.arrival_at_plane();
 require(actual.tick==cue_prediction.tick&&actual.time_s==cue_prediction.time_s&&same(actual.state.position_m,cue_prediction.state.position_m)&&same(actual.state.velocity_mps,cue_prediction.state.velocity_mps),"fixed-pitch actual arrival differs from prediction");
 for(unsigned n=0;n<10;++n){live.move(1,1,.02);p.advance(60'000'000'000);check_history();}
 const auto adjusted=live.center();
 require(p.start(),"review Space restart");check_live();
 require(live.center().x==adjusted.x&&live.center().y==adjusted.y,"Space reset adjusted live aim");
 require(!p.latest()&&!p.flight&&!contact_review_arrival_visible(p),"old review retained after Space");
 p.input_boundary(false,false,true,live.center());until(447);p.input_boundary(true,true,true,live.center());until(448);
 require(p.attempts.size()==1&&p.latest()->command.aim_center.x==adjusted.x&&p.latest()->command.aim_center.y==adjusted.y,"next input did not snapshot adjusted aim");
 finish(p);p.start();
 // First-ground hold can precede overall completion; both selected cue styles still persist.
 p.record_command(433,center);until(735);
 require(p.phase==PreviewPhase::Playing&&p.flight&&p.flight->complete(double(p.tick)/240)&&contact_review_arrival_visible(p),"ground-before-completion cue fixture");check_cue();
 live.move(-1,-1,.05);live.recenter();const auto recentered=live.center();p.start();check_live();
 require(live.center().x==recentered.x&&live.center().y==recentered.y,"Space undid post-contact R");
 p.record_command(80,live.center());finish(p);
 require(p.gameplay_result()==GameplayResult::Miss&&!contact_review_arrival_visible(p),"miss persisted cue");check_live();
 std::cout<<"PASS live aim: immutable second attempt and flight control; Space preserves adjusted/R aim; next command snapshots it; exact cue prediction/authorization/actual arrival; both cue styles through flight/follow-through/ground/Complete; reset/miss/take; fixed 210 reticle + 636 cue vertices and stable allocation\n";
 // Finite, bounded startup response Data; no silent clamping or unknown keys.
 std::filesystem::create_directories(temp);const auto path=temp/"candidate.toml";
 const std::string profile_text="[batter_profile]\ndisplay_name='Michael'\ncontact=75\npower=85\ntrajectory=3\n";
 for(const char* bad:{"ideal_exit_speed_min_mps=0","ideal_exit_speed_max_mps=101","ideal_exit_speed_min_mps=60","spatial_edge_transfer=-0.1","spatial_edge_transfer=1.1","minimum_exit_speed_factor=nan","minimum_exit_speed_factor=2","vertical_aim_bias_degrees=inf","vertical_aim_bias_degrees=-1","max_spray_degrees=81","full_spray_offset_ms=0","full_spray_offset_ms='65'","vertical_contact_longitudinal_degrees=[8,14,20]","vertical_contact_longitudinal_degrees=[8,14,20,26,30]","vertical_contact_longitudinal_degrees=[8,14,nan,26]","vertical_contact_longitudinal_degrees=[8,14,20,51]","vertical_contact_longitudinal_degrees=[8,14,20,'26']","vertical_contact_longitudinal_degrees=1","vertical_contact_longitudinal_degrees=[-181,-35,-15,0,25,45,130]","vertical_contact_longitudinal_degrees=[-150,-35,-15,0,25,45,181]","vertical_contact_longitudinal_degrees=[-150,-35,-15,0,25,45,nan]","vertical_contact_longitudinal_degrees=[-150,-35,-15,0,25,45,inf]","vertical_contact_longitudinal_degrees=[-150,-35,-15,0,25,45,'130']","vertical_contact_longitudinal_degrees=[-150,-35,-15,0,25,45,130,150]","trajectory_launch_degrees=[8,14,20,26]","unknown=1"}){
  {std::ofstream out(path);out<<profile_text<<"[ball_response]\n"<<bad<<'\n';}
  bool failed=false;try{(void)load_batting_staging(path);}catch(const std::runtime_error& e){failed=std::string(e.what()).find("ball_response")!=std::string::npos;}require(failed,"bad response Data accepted");
 }
 {std::ofstream out(path);out<<profile_text<<"[ball_response]\nideal_exit_speed_min_mps=25\nideal_exit_speed_max_mps=60\nvertical_contact_longitudinal_degrees=[-180,-40,-10,0,20,50,180]\n";}
 const auto tuned=load_batting_staging(path);require(tuned.ball_response.ideal_exit_speed_min_mps==25&&tuned.ball_response.vertical_contact_longitudinal_degrees[2]==-10,"valid response override ignored");std::filesystem::remove(path);
 std::cout<<"PASS gameplay authority, normalized response, handed spray, deterministic launch/ground and two-swing lifecycle\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
