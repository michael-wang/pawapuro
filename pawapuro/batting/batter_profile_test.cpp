#include "batter_card.hpp"
#include "manual_swing.hpp"
#include "player_aim.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace pawapuro;
void require(bool b,const char* message){if(!b)throw std::runtime_error(message);}
bool same(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b){return a.x==b.x&&a.y==b.y&&a.z==b.z;}
int main(int argc,char** argv){try {
    require(argc==3,"expected batting and fixture directories");const std::filesystem::path d=argv[1],temp=argv[2];
    std::filesystem::create_directories(temp);const auto path=temp/"profile.toml";
    const auto s=load_batting_staging(d/"staging.toml");const auto& p=s.batter_profile;
    require(p.display_name=="Michael"&&p.contact==75&&p.power==85&&p.trajectory==3,"authored profile differs");
    for(const auto [value,grade]:{std::pair{0,'F'},{49,'F'},{50,'E'},{59,'E'},{60,'D'},{69,'D'},
        {70,'C'},{79,'C'},{80,'B'},{89,'B'},{90,'A'},{99,'A'},{100,'S'},{120,'S'}})
        require(ability_grade(value)==grade,"grade boundary differs");
    const auto write=[&](std::string name,std::string contact,std::string power,std::string trajectory,std::string extra="") {
        std::ofstream(path)<<"[batter_profile]\n"<<"display_name="<<name<<"\ncontact="<<contact<<"\npower="<<power<<"\ntrajectory="<<trajectory<<'\n'<<extra;
    };
    const auto reject=[&](const char* key){
        try{(void)load_batting_staging(path);}catch(const std::runtime_error& e){
            require(std::string(e.what()).find(key)!=std::string::npos,"wrong validation diagnostic");return;}
        throw std::runtime_error("invalid profile accepted");
    };
    for(const char* bad:{"-1","121","75.0","75.5","true","'75'","nan"}){
        write("'Michael'",bad,"85","3");reject("batter_profile.contact");
        write("'Michael'","75",bad,"3");reject("batter_profile.power");
    }
    for(const char* bad:{"0","5","3.0","'3'","false"}){write("'Michael'","75","85",bad);reject("batter_profile.trajectory");}
    for(const char* bad:{"''","'   '","75","true"}){write(bad,"75","85","3");reject("batter_profile.display_name");}
    write("'Michael'","75","85","3","handedness='left'\n");reject("batter_profile.handedness");
    for(const char* missing:{"display_name","contact","power","trajectory"}){
        {std::ofstream out(path);out<<"[batter_profile]\n";
        for(const auto& field:{std::pair{"display_name","'Michael'"},{"contact","75"},{"power","85"},{"trajectory","3"}})
            if(std::string(missing)!=field.first)out<<field.first<<'='<<field.second<<'\n';}
        reject((std::string("batter_profile.")+missing).c_str());
    }
    {std::ofstream out(path);out<<"batter_profile=7\n";}reject("batter_profile must be a table");
    {std::ofstream out(path);}reject("batter_profile.display_name");
    for(const char* value:{"0","120"})for(const char* trajectory:{"1","2","3","4"}){
        write("'Michael'",value,value,trajectory);(void)load_batting_staging(path);
    }
    for(unsigned h:{450u,1080u,1620u}){
        const BatterCard card(p,h*16/9,h);
        require(card.lines==std::array<std::wstring,4>{L"Michael",L"CONTACT   C",L"POWER     B",L"彈道        3"},"card is not profile-derived");
        require(card.vertex_count()>18,"card text missing");
        std::vector<engine::Vertex> stream;stream.reserve(card.vertex_count());const auto* data=stream.data();const auto cap=stream.capacity();
        for(unsigned n=0;n<1000;++n){stream.clear();card.append(stream);
            require(stream.data()==data&&stream.capacity()==cap&&stream.size()==card.vertex_count(),"append allocation/capacity regression");
            require(std::memcmp(stream.data(),card.triangles.data(),stream.size()*sizeof(engine::Vertex))==0,"cached geometry changed");}
        for(const auto& v:stream)require(v.position.x>=-1&&v.position.x<-.6f&&v.position.y>=-1&&v.position.y<-.5f,"card escaped bottom-left bounds");
        const BatterCard other({"Julia",100,49,4},h*16/9,h);
        require(other.lines[0]==L"Julia"&&other.lines[1]==L"CONTACT   S"&&other.lines[2]==L"POWER     F"&&other.lines[3]==L"彈道        4","card duplicated authored values");
    }
    // Profile-only changes cannot affect any tick of the accepted two-swing sequence.
    auto changed=s;changed.batter_profile={"Other",0,120,1};
    ManualSwingPreview a(d,s),b(d,changed);PlayerAim aim_a(s),aim_b(changed);
    std::vector<engine::Vertex> ra,rb;aim_a.append_triangles(ra);aim_b.append_triangles(rb);
    require(ra.size()==rb.size()&&std::memcmp(ra.data(),rb.data(),ra.size()*sizeof(engine::Vertex))==0,"profile changed reticle");
    require(s.hit_authorization.normal_radius_x_m==.26f&&s.hit_authorization.normal_radius_y_m==.13f,"ellipse changed");
    a.start();b.start();a.record_command(80,aim_a.center());b.record_command(80,aim_b.center());
    for(unsigned t=1;t<=816;++t){a.advance(4'166'667);b.advance(4'166'667);
        require(a.tick==b.tick&&a.phase==b.phase&&a.rearmed()==b.rearmed()&&a.active_attempt==b.active_attempt,"profile changed multi-swing lifecycle");
        require(a.batter.pose.world==b.batter.pose.world&&a.batter.barrel_world==b.batter.barrel_world&&same(a.displayed_ball_center(),b.displayed_ball_center()),"profile changed motion/pitch");
        if(t==320){require(a.rearmed(),"early rearm lost");a.record_command(448,aim_a.center());b.record_command(448,aim_b.center());}
    }
    require(a.attempts.size()==2&&b.attempts.size()==2&&a.attempts[1].contact,"two-swing contact regression");
    for(unsigned i=0;i<2;++i){const auto& x=a.attempts[i];const auto& y=b.attempts[i];
        require(x.timing.state==y.timing.state&&x.timing.efficiency==y.timing.efficiency&&x.timing.offset_ms==y.timing.offset_ms&&x.authorization.q==y.authorization.q&&x.authorization.authorized==y.authorization.authorized&&x.geometry==y.geometry,"profile changed decisions");
        if(x.contact){require(y.contact.has_value(),"profile changed contact");const auto& c=*x.contact;const auto& e=*y.contact;
            require(c.sample.preview_time_s==e.sample.preview_time_s&&c.sample.approach.u==e.sample.approach.u&&same(c.normal,e.normal)&&same(c.relative_velocity,e.relative_velocity)&&c.ball_radius_m==.037f&&c.bat_radius_m==.033f,"profile changed physical event");}
    }
    std::filesystem::remove(path);
    std::cout<<"PASS profile validation/grade boundaries; cached bottom-left card; profile-only two-swing gameplay invariance\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
