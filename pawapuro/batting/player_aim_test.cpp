#include "player_aim.hpp"
#include "reference_scene.hpp"
#include "batting_preview.hpp"
#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>
using namespace pawapuro;
namespace {
void require(bool ok,const char* why) {if(!ok)throw std::runtime_error(why);}
bool close_enough(float a,float b) {return std::abs(a-b)<1e-6f;}
bool same(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b) {return a.x==b.x&&a.y==b.y&&a.z==b.z;}
void corner(PlayerAim& a,float x,float y) {for(int i=0;i<100;++i)a.move(x,y,.05);}
}
int main(int argc,char** argv) {
 try {
    require(argc==3,"expected batting directory and staging");const auto s=load_batting_staging(argv[2]);PlayerAim aim(s);
    const auto center=aim.center();require(center.x==0&&center.y==(s.strike_zone_bottom_m+s.strike_zone_top_m)/2,"startup center");
    const float step=s.player_aim.cursor_speed_mps*.02f;
    for(auto d:{DirectX::XMFLOAT2{1,0},{-1,0},{0,1},{0,-1}}) {
        aim.recenter();aim.move(d.x,d.y,.02);require(close_enough(aim.center().x,center.x+d.x*step)&&close_enough(aim.center().y,center.y+d.y*step),"signed movement");
    }
    aim.recenter();aim.move(1,1,.02);require(close_enough(std::hypot(aim.center().x-center.x,aim.center().y-center.y),step),"diagonal normalization");
    for(float x:{-1.f,1.f})for(float y:{-1.f,1.f}) {
        corner(aim,x,y);require(aim.center().x==x*s.strike_zone_width_m/2&&aim.center().y==(y<0?s.strike_zone_bottom_m:s.strike_zone_top_m),"center clamp");
    }
    aim.recenter();require(aim.center().x==center.x&&aim.center().y==center.y,"recenter");
    aim.move(1,0,50);require(close_enough(aim.center().x,s.player_aim.cursor_speed_mps*.05f),"large frame gap cap");
    aim.recenter();aim.move(1,1,-1);aim.move(1,0,std::numeric_limits<double>::infinity());require(aim.center().x==center.x&&aim.center().y==center.y,"invalid dt moved aim");
    const auto d=aim.diagnostic({center.x+.13f,center.y-.13f,s.strike_zone_plane_z()});
    require(close_enough(d.dx,.13f)&&close_enough(d.dy,-.13f)&&close_enough(d.ex,.5f)&&close_enough(d.ey,-1)&&close_enough(d.q,1.25f),"signed diagnostic normalization/q");
    const auto project=[&](DirectX::XMFLOAT2 p){return project_batting_point(s,{p.x,p.y,s.strike_zone_plane_z()},16.f/9);};
    const auto pc=project(center);aim.move(1,0,.02);require(project(aim.center()).x>pc.x,"screen Right mapping");
    aim.recenter();aim.move(0,1,.02);require(1-project(aim.center()).y<1-pc.y,"screen Up pixel mapping");
    for(float bad:{0.f,-1.f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}) {
        for(int field=0;field<3;++field) {
            auto invalid=s;
            if(field==0)invalid.hit_authorization.normal_radius_x_m=bad;
            if(field==1)invalid.hit_authorization.normal_radius_y_m=bad;
            if(field==2)invalid.player_aim.cursor_speed_mps=bad;
            bool rejected=false;try{PlayerAim test(invalid);}catch(const std::runtime_error&){rejected=true;}
            require(rejected,"invalid tuning accepted");
        }
    }
    std::vector<engine::Vertex> vertices;vertices.reserve(PlayerAim::vertex_count);const auto capacity=vertices.capacity();
    for(int i=0;i<100;++i){vertices.clear();aim.append_triangles(vertices);require(vertices.size()==PlayerAim::vertex_count&&vertices.capacity()==capacity,"reticle allocation/count");}
    BattingPreview baseline(argv[1],s),moving(argv[1],s);
    for(int placement=0;placement<3;++placement) {
        aim.recenter();if(placement==1)corner(aim,-1,1);if(placement==2)corner(aim,1,-1);
        const auto persistent=aim.center();baseline.start();moving.start();baseline.toggle_pause();moving.toggle_pause();
        require(aim.center().x==persistent.x&&aim.center().y==persistent.y,"preview reset aim");
        for(unsigned tick=1;tick<=896;++tick) {
            // Ready/corner intent is entirely independent of both Native preview owners.
            (void)aim.diagnostic(predict_arrival(moving.delivery.pitch).state.position_m);
            baseline.single_step();moving.single_step();
            require(same(baseline.delivery.pitch.current.position_m,moving.delivery.pitch.current.position_m)&&same(baseline.delivery.pitch.current.velocity_mps,moving.delivery.pitch.current.velocity_mps)
                &&baseline.delivery.motion.pose.world==moving.delivery.motion.pose.world&&baseline.batter.pose.world==moving.batter.pose.world,"aim changed ball path/animation");
            require(bool(baseline.contact)==bool(moving.contact),"aim gated physical contact");
            if(baseline.contact)require(baseline.contact->sample.preview_time_s==moving.contact->sample.preview_time_s&&baseline.contact->sample.approach.u==moving.contact->sample.approach.u
                &&same(baseline.contact->normal,moving.contact->normal),"aim changed event time/u/normal");
        }
    }
    const auto prediction=predict_arrival(baseline.delivery.pitch).state.position_m;
    for(auto target:{DirectX::XMFLOAT2{0,0},{-1,1},{1,-1}}) {
        aim.recenter();corner(aim,target.x,target.y);const auto a=aim.center();const auto diagnostic=aim.diagnostic(prediction);
        std::printf("aim=(%.9g,%.9g) predicted=(%.9g,%.9g) error=(%.9g,%.9g) normalized=(%.9g,%.9g) q=%.9g\n",a.x,a.y,prediction.x,prediction.y,diagnostic.dx,diagnostic.dy,diagnostic.ex,diagnostic.ey,diagnostic.q);
    }
    std::puts("PASS PlayerAim: axes/projection, diagonal, clamps, recenter, dt cap, signed diagnostics, tuning, fixed-capacity geometry, center/corners unchanged full physical path/poses/contact.");return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}
}
