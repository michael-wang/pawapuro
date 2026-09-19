#include "review_fixture.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
using namespace pawapuro;
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F> void rejects(F action){bool rejected=false;try{action();}catch(const std::runtime_error&){rejected=true;}require(rejected,"invalid fixture accepted");}
BattingStartupOptions options(std::initializer_list<const char*> args){return parse_batting_options(static_cast<int>(args.size()),args.begin());}
std::string truth(const ManualSwingPreview& p) {
    std::ostringstream s;s<<std::hexfloat;
    const auto pair=[&](DirectX::XMFLOAT2 v){s<<v.x<<','<<v.y<<';';};
    const auto triple=[&](DirectX::XMFLOAT3 v){s<<v.x<<','<<v.y<<','<<v.z<<';';};
    const auto& a=*p.latest();const auto& t=a.timing;const auto& h=a.authorization;
    s<<p.tick<<','<<p.pending_ticks<<','<<p.attempts.size()<<','<<a.command.consumed_tick<<','<<a.command.target_tick<<';';pair(a.command.aim_center);
    s<<int(a.command.tempo.mode)<<','<<int(t.state)<<','<<t.start_s<<','<<t.peak_s<<','<<t.end_s<<','<<t.efficiency<<','<<t.offset_ms<<';';
    if(t.overlap)s<<t.overlap->start_s<<','<<t.overlap->end_s<<';';
    pair(h.pitch_point);pair(h.error);pair(h.normalized_error);s<<h.authorized<<','<<h.q<<','<<int(a.gameplay)<<','<<int(a.geometry)<<','<<a.gameplay_dispatch_tick<<';';
    s<<a.contact_query_count<<','<<a.contact_count<<','<<a.contact_dispatch_tick<<';';
    if(a.searched_interval)s<<a.searched_interval->start_s<<','<<a.searched_interval->end_s<<';';
    if(a.contact){const auto& c=*a.contact;s<<c.fractional_tick<<','<<c.sample.preview_time_s<<','<<c.sample.approach.u<<','<<c.ball_radius_m<<','<<c.bat_radius_m<<';';triple(c.normal);triple(c.relative_velocity);}
    if(a.response){const auto& r=*a.response;s<<r.contact_time_s<<','<<r.exit_speed_mps<<','<<r.longitudinal_angle_deg<<','<<r.spray_angle_deg<<','<<r.temporal_transfer<<','<<r.spatial_transfer<<','<<r.energy_transfer<<';';triple(r.launch_position_m);triple(r.launch_velocity_mps);}
    if(p.flight){const auto& f=*p.flight;triple(f.initial.position_m);triple(f.initial.velocity_mps);s<<f.start_s<<','<<f.ground_s<<','<<f.ground_height_m<<';';
        for(double time:{2.,2.1,2.5,3.,5.,10.}){const auto b=f.sample(time);triple(b.position_m);triple(b.velocity_mps);}}
    return s.str();
}
int main(int argc,char** argv){try{
    require(argc==2,"expected batting directory");const std::filesystem::path directory=argv[1];
    const auto staging=load_batting_staging(directory/"staging.toml");
    require(!options({"pawapuro"}).fixture,"normal mode changed");
    const auto parsed=options({"pawapuro","--staging","custom path.toml","--batting-fixture","448","0","+0.9"});
    require(parsed.staging_path=="custom path.toml"&&parsed.fixture->commit_tick==448&&parsed.fixture->ey==.9f,"combined options");
    require(options({"p","--batting-fixture","433","-0.1","0","--staging","s"}).staging_path=="s","option order");
    require(options({"p","--batting-fixture","18446744073709551615","0","0"}).fixture->commit_tick==UINT64_MAX,"uint64 parsing");
    for(const char* tick:{"-1","1.5","448x","18446744073709551616",""})rejects([&]{options({"p","--batting-fixture",tick,"0","0"});});
    for(const char* value:{"nan","inf","-inf","1e50","0x1",".9x","+-1",""}){rejects([&]{options({"p","--batting-fixture","448",value,"0"});});rejects([&]{options({"p","--batting-fixture","448","0",value});});}
    rejects([]{options({"p","--batting-fixture","448","0"});});
    rejects([]{options({"p","--unknown"});});rejects([]{options({"p","--staging"});});
    rejects([]{options({"p","--staging","a","--staging","b"});});
    rejects([]{options({"p","--batting-fixture","448","0","0","--batting-fixture","448","0","0"});});
    PlayerAim aim(staging);const auto original=aim.center();
    require(!aim.set_center({INFINITY,0})&&!aim.set_center({0,NAN})&&!aim.set_center({1,original.y})&&!aim.set_center({0,0}),"setter accepted invalid aim");
    require(aim.center().x==original.x&&aim.center().y==original.y,"failed setter mutated aim");
    ManualSwingPreview preview(directory,staging);
    for(const BattingReviewFixture fixture: {BattingReviewFixture{448,0,0},{448,0,.9f},{448,0,-.9f},{433,0,0},{456,0,0},{448,.6f,1}}) {
        std::string baseline;
        for(const std::uint64_t cadence:{33'333'333ULL,16'666'667ULL,8'333'333ULL,250'000'000ULL}) {
            require(fixture.start(preview,aim),"fixture start/replay rejected");
            require(preview.tick==0&&preview.pending_ticks==0&&preview.attempts.empty()&&!preview.flight&&!preview.active_attempt,"replay retained old state/debt");
            require(preview.pending&&preview.pending->target_tick==fixture.commit_tick&&preview.pending->aim_center.x==aim.center().x&&preview.pending->aim_center.y==aim.center().y,"render/command aim mismatch");
            require(!fixture.start(preview,aim),"pre-contact Space cancelled fixture");
            BattingReviewFixture::toggle_pause(preview);preview.advance(1'000'000'000);
            require(preview.tick==0&&preview.pending.has_value(),"pause lost exact fixture intent");
            preview.single_step();require(preview.tick==1&&preview.pending.has_value(),"fixture single step");
            BattingReviewFixture::toggle_pause(preview);
            while(preview.phase==PreviewPhase::Playing)preview.advance(cadence);
            require(preview.attempts.size()==1,"extra fixture attempt");fixture.verify(*preview.latest());
            require(preview.committed()->tempo.mode==SwingTempo::Compact,"fixture tempo changed");
            require(preview.latest()->authorization.authorized==(fixture.ex==0),"fixture forced authorization");
            require(preview.gameplay_result()==(fixture.ex==0?GameplayResult::Contact:GameplayResult::Miss),"fixture forced result");
            if(fixture.ey!=0&&fixture.ex==0){const auto v=preview.latest()->response->launch_velocity_mps;require(v.z<0&&(fixture.ey>0?v.y>0:v.y<0),"backward fixture direction");}
            const auto actual=truth(preview);if(baseline.empty())baseline=actual;else require(actual==baseline,"replay/cadence changed authoritative output");
            const auto held_tick=preview.tick;preview.advance(1'000'000'001);require(preview.tick==held_tick&&truth(preview)==baseline,"result hold changed");
        }
        std::cout<<std::setprecision(12)<<"fixture "<<fixture.commit_tick<<" requested="<<fixture.ex<<','<<fixture.ey<<" consumed="<<preview.committed()->consumed_tick<<" actual="<<preview.authorization()->normalized_error.x<<','<<preview.authorization()->normalized_error.y<<" gameplay="<<preview.gameplay_state()<<" replay exact across 30/60/120/backlog\n";
    }
    // Invalid intent is rejected, not clamped into a reachable/successful fixture.
    rejects([&]{BattingReviewFixture{448,100,0}.start(preview,aim);});preview.reset();
    rejects([&]{BattingReviewFixture{0,0,0}.start(preview,aim);});preview.reset();
    rejects([&]{BattingReviewFixture{481,0,0}.start(preview,aim);});preview.reset();
    // Replay is also allowed while a contacted ball is still airborne.
    const BattingReviewFixture pop{448,0,.9f};require(pop.start(preview,aim),"pop start");
    while(!preview.flight)preview.advance(4'166'667);
    require(!preview.flight->complete(double(preview.tick)/pitch_hz),"pop already grounded");
    preview.advance(1'000'000'001);require(pop.start(preview,aim),"airborne replay");
    require(preview.tick==0&&preview.pending_ticks==0&&!preview.flight&&preview.attempts.empty(),"airborne replay retained truth/debt");
    preview.advance(4'166'666);require(preview.tick==0,"fractional debt survived replay");
    preview.advance(1);require(preview.tick==1,"replay clock not clean");
    // Review batches use the same intermediate ticks as repeated single steps.
    ManualSwingPreview singles(directory,staging);PlayerAim singles_aim(staging);
    preview.reset();require(!preview.step_ticks(10)&&preview.tick==0,"Ready stepped");
    const BattingReviewFixture center{448,0,0};center.start(preview,aim);center.start(singles,singles_aim);
    require(!preview.step_ticks(10)&&preview.tick==0,"unpaused stepped");
    BattingReviewFixture::toggle_pause(preview);BattingReviewFixture::toggle_pause(singles);
    preview.step_ticks(1);singles.single_step();require(preview.tick==1&&singles.tick==1,"one-step changed");
    bool release_crossed=false,commit_crossed=false,contact_crossed=false,stopped_short=false;
    while(preview.phase==PreviewPhase::Playing){
        // Exercise a short final batch independently of tuned flight duration.
        if(preview.flight&&preview.tick+10>=preview.completion_tick())
            while(preview.tick+3<preview.completion_tick()){preview.single_step();singles.single_step();}
        const auto before=preview.tick;
        const bool completed=preview.step_ticks(10);bool single_completed=false;
        for(unsigned i=0;i<10;++i)single_completed=singles.single_step()||single_completed;
        require(completed==single_completed&&preview.tick==singles.tick&&preview.phase==singles.phase&&preview.paused==singles.paused,"batch clock/completion differs");
        require(preview.pending_ticks==0&&singles.pending_ticks==0,"step introduced backlog");
        require(preview.batter.pose.world==singles.batter.pose.world&&preview.delivery.motion.pose.world==singles.delivery.motion.pose.world,"batch pose differs");
        const auto a=preview.displayed_ball_center(),b=singles.displayed_ball_center();
        require(a.x==b.x&&a.y==b.y&&a.z==b.z&&preview.delivery.pitch.phase==singles.delivery.pitch.phase&&preview.delivery.tick==singles.delivery.tick,"batch pitch/flight differs");
        require(preview.attempts.size()==singles.attempts.size()&&bool(preview.pending)==bool(singles.pending),"batch command differs");
        if(preview.latest())require(truth(preview)==truth(singles),"batch authoritative result differs");
        if(before<384&&preview.tick>=384){require(preview.delivery.release_count==1,"batch skipped release");release_crossed=true;}
        if(before<448&&preview.tick>=448){require(preview.committed()->consumed_tick==448,"batch skipped exact commit");center.verify(*preview.latest());commit_crossed=true;}
        if(before<478&&preview.tick>=478){require(preview.latest()->gameplay_dispatch_tick==478&&preview.flight.has_value(),"batch skipped Contact dispatch");contact_crossed=true;}
        if(!completed)require(preview.paused&&preview.tick==before+10,"step unpaused or wrong count");
        else stopped_short=preview.tick<before+10&&preview.tick==preview.completion_tick();
    }
    require(release_crossed&&commit_crossed&&contact_crossed&&stopped_short,"missing step boundary coverage");
    const auto held=truth(preview);preview.step_ticks(10);preview.advance(1'000'000'000);require(truth(preview)==held,"Complete step advanced");
    // Existing backlog and fractional credit are discarded, never replayed on resume.
    preview.reset();preview.start();preview.advance(150'000'001);require(preview.pending_ticks>0,"debt fixture has no backlog");
    preview.toggle_pause();const auto debt_tick=preview.tick;preview.step_ticks(10);
    require(preview.tick==debt_tick+10&&preview.paused&&preview.pending_ticks==0,"review retained backlog");
    preview.toggle_pause();const auto resumed=preview.tick;preview.advance(0);preview.advance(4'166'666);
    require(preview.tick==resumed,"review retained fractional debt");preview.advance(1);require(preview.tick==resumed+1,"resume clock changed");
    std::cout<<"Review steps: 1/10, release 384, commit 448, Contact 478, Complete early-stop and zero debt passed\n";
    std::cout<<"Batting review fixture passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
