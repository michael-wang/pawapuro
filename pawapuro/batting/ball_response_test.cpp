#include "manual_swing.hpp"
#include "player_aim.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace pawapuro;
void require(bool b,const char* m){if(!b)throw std::runtime_error(m);}
bool close_enough(double a,double b,double e=1e-6){return std::abs(a-b)<=e;}
bool same(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b){return a.x==b.x&&a.y==b.y&&a.z==b.z;}
void finish(ManualSwingPreview& p){while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);}
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
  if(c==448){require(a.contact&&r.temporal_transfer>.97f&&r.spatial_transfer==1&&close_enough(r.launch_angle_deg,20)&&std::abs(r.spray_angle_deg)<1,"peak fixture");peak=r;}
  if(c==433)require(r.spray_angle_deg>0&&r.launch_velocity_mps.x>0,"left-handed early pull sign");
  if(c==460)require(r.spray_angle_deg<0&&r.launch_velocity_mps.x<0,"late opposite sign");
  require(r.launch_velocity_mps.z>0&&r.contact_time_s==std::clamp(a.timing.peak_s,a.timing.overlap->start_s,a.timing.overlap->end_s),"effective time/center field");
  require(same(r.launch_position_m,sample_reference_pitch(p.delivery.pitch.initial,r.contact_time_s-double(p.delivery.motion.release_tick)/240).position_m),"launch origin not pitch truth");
  require(a.gameplay_dispatch_tick==static_cast<std::uint64_t>(std::ceil(r.contact_time_s*240)),"gameplay dispatched at wrong tick");
  require(p.tick==p.completion_tick(),"completion did not wait for ground/animations");
  const auto ground=p.flight->sample(p.flight->ground_s);require(ground.position_m.y==.037f&&same(ground.velocity_mps,{})&&same(ground.position_m,p.flight->sample(p.flight->ground_s+10).position_m),"ground hold/bounce");
  require(p.flight->sample(p.flight->ground_s-1e-5).position_m.y>.037f,"wrong ground root");
  std::cout<<"RESPONSE commit="<<c<<" raw="<<p.contact_state()<<" efficiency="<<a.timing.efficiency<<" offset_ms="<<a.timing.offset_ms<<" effective_s="<<r.contact_time_s<<" speed="<<r.exit_speed_mps<<" kmh="<<r.exit_speed_mps*3.6<<" launch="<<r.launch_angle_deg<<" spray="<<r.spray_angle_deg<<" ground_s="<<p.flight->ground_s<<'\n';
 }
 const float ideal=20+(55-20)*(85.f/120);require(close_enough(peak.exit_speed_mps,ideal*(.2f+.8f*std::sqrt(peak.energy_transfer)),1e-5),"response speed curve");
 const auto rejected=run(448,1.01f);require(rejected.contact&&!rejected.response&&!p.flight&&rejected.gameplay==GameplayResult::Miss,"raw contact overrode spatial rejection");
 const auto up=run(448,0,.8f),down=run(448,0,-.8f);require(up.response&&down.response&&up.response->launch_angle_deg>peak.launch_angle_deg&&down.response->launch_angle_deg<peak.launch_angle_deg&&close_enough(up.response->exit_speed_mps,down.response->exit_speed_mps,1e-5),"vertical bias/q sign");
 std::cout<<"VERTICAL ey=+0.8 launch="<<up.response->launch_angle_deg<<" ey=0 launch="<<peak.launch_angle_deg<<" ey=-0.8 launch="<<down.response->launch_angle_deg<<" paired_speed="<<up.response->exit_speed_mps<<'\n';
 const auto left=run(448,.8f),right=run(448,-.8f);require(left.response->spray_angle_deg==right.response->spray_angle_deg&&close_enough(left.response->exit_speed_mps,right.response->exit_speed_mps),"ex steered spray");
 auto a=run(448);auto profile=s.batter_profile;profile.power=120;
 auto power=ball_response(a.timing,a.authorization,profile,s.ball_response,p.delivery.pitch.initial,1.6);
 require(power->exit_speed_mps>peak.exit_speed_mps&&power->launch_angle_deg==peak.launch_angle_deg&&power->spray_angle_deg==peak.spray_angle_deg,"Power independence");
 profile=s.batter_profile;profile.trajectory=4;auto trajectory=ball_response(a.timing,a.authorization,profile,s.ball_response,p.delivery.pitch.initial,1.6);
 require(trajectory->launch_angle_deg==26&&trajectory->exit_speed_mps==peak.exit_speed_mps&&trajectory->spray_angle_deg==peak.spray_angle_deg,"Trajectory independence");
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
 // Finite, bounded startup response Data; no silent clamping or unknown keys.
 std::filesystem::create_directories(temp);const auto path=temp/"candidate.toml";
 const std::string profile_text="[batter_profile]\ndisplay_name='Michael'\ncontact=75\npower=85\ntrajectory=3\n";
 for(const char* bad:{"ideal_exit_speed_min_mps=0","ideal_exit_speed_max_mps=101","ideal_exit_speed_min_mps=60","spatial_edge_transfer=-0.1","spatial_edge_transfer=1.1","minimum_exit_speed_factor=nan","minimum_exit_speed_factor=2","vertical_aim_bias_degrees=inf","vertical_aim_bias_degrees=-1","max_spray_degrees=81","full_spray_offset_ms=0","full_spray_offset_ms='65'","trajectory_launch_degrees=[8,14,20]","trajectory_launch_degrees=[8,14,20,26,30]","trajectory_launch_degrees=[8,14,nan,26]","trajectory_launch_degrees=[8,14,20,51]","trajectory_launch_degrees=[8,14,20,'26']","unknown=1"}){
  {std::ofstream out(path);out<<profile_text<<"[ball_response]\n"<<bad<<'\n';}
  bool failed=false;try{(void)load_batting_staging(path);}catch(const std::runtime_error& e){failed=std::string(e.what()).find("ball_response")!=std::string::npos;}require(failed,"bad response Data accepted");
 }
 {std::ofstream out(path);out<<profile_text<<"[ball_response]\nideal_exit_speed_min_mps=25\nideal_exit_speed_max_mps=60\ntrajectory_launch_degrees=[1,2,3,4]\n";}
 const auto tuned=load_batting_staging(path);require(tuned.ball_response.ideal_exit_speed_min_mps==25&&tuned.ball_response.trajectory_launch_degrees[2]==3,"valid response override ignored");std::filesystem::remove(path);
 std::cout<<"PASS gameplay authority, normalized response, handed spray, deterministic launch/ground and two-swing lifecycle\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
