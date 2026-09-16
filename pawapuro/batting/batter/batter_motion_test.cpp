#include "batter_motion.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace pawapuro;
using DirectX::XMFLOAT3;
namespace {
void require(bool ok,const char* why) { if (!ok) throw std::runtime_error(why); }
float distance(XMFLOAT3 a,XMFLOAT3 b) { return std::sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)+(a.z-b.z)*(a.z-b.z)); }
XMFLOAT3 xyz(std::istream& in) { XMFLOAT3 p;in>>p.x>>p.y>>p.z;require(bool(in),"fixture XYZ");return p; }
XMFLOAT3 center(const engine::GlbMatrix& m) { return {m[12],m[13],m[14]}; }
}
int main(int argc,char** argv) {
 try {
    require(argc==5,"expected GLB, metadata, staging and saved source samples");
    const auto staging=load_batting_staging(argv[3]);BatterMotion m(argv[1],argv[2],staging);
    require(m.asset.primitives.size()==2 && m.asset.joints.size()==17 && m.asset.clip_name=="swing_L","asset contract");
    require(m.end_tick==896 && m.gather_tick==284 && m.plant_tick==424 && m.contact_area_tick==480,"accepted metadata mapping");
    require(m.triangles.size()==14304 && m.asset.times.size()==225,"sample count");
    const auto initial=m.triangles;const auto capacity=m.triangles.capacity();
    const auto* pose_data=m.pose.world.data();const auto* body_data=m.skinned[0].data();const auto* bat_data=m.skinned[1].data();
    const auto binding_distance=distance({-m.skinned[1][0].position.x+staging.batter_blockout_position_m.x,
        m.skinned[1][0].position.y+staging.batter_blockout_position_m.y,m.skinned[1][0].position.z+staging.batter_blockout_position_m.z},center(m.grip_world));
    std::size_t hand=0;unsigned hands=0;
    for (std::size_t i=0;i<m.asset.nodes.size();++i) if (m.asset.nodes[i].name=="hand_R") {hand=i;++hands;}
    require(hands==1,"unique authoritative hand");
    for (std::uint64_t tick=0;tick<=m.end_tick;++tick) {
        m.evaluate_tick(tick);require(m.tick==tick && m.complete()==(tick==m.end_tick),"tick clamp/completion");
        require(m.triangles.capacity()==capacity && m.triangles.size()==14304 && m.pose.world.data()==pose_data
            && m.skinned[0].data()==body_data && m.skinned[1].data()==bat_data,"steady evaluation reallocated");
        for (std::size_t p=0;p<2;++p) for (std::size_t i=0;i<m.skinned[p].size();++i) {
            const auto& v=m.skinned[p][i];require(std::isfinite(v.position.x)&&std::isfinite(v.position.y)&&std::isfinite(v.position.z),"nonfinite skin");
            require(std::memcmp(&v.color,&m.asset.primitives[p].bind_vertices[i].color,sizeof(XMFLOAT3))==0,"color changed");
        }
        const auto origin=staging.batter_blockout_position_m;const auto& hw=m.pose.world[hand];
        require(distance(center(m.grip_world),{-hw[12]+origin.x,hw[13]+origin.y,hw[14]+origin.z})<.0001f,"grip left hand_R");
        require(std::abs(distance(center(m.grip_world),center(m.barrel_world))-.86f)<.0001f
            && std::abs(distance(center(m.grip_world),center(m.tip_world))-1.2f)<.0001f,"bat semantic length changed");
        const auto v=m.skinned[1][0].position;
        require(std::abs(distance({-v.x+origin.x,v.y+origin.y,v.z+origin.z},center(m.grip_world))-binding_distance)<.0001f,"rigid Bat attachment drift");
    }
    const auto final=m.triangles;m.evaluate_tick(m.end_tick+1000);
    require(std::memcmp(final.data(),m.triangles.data(),final.size()*sizeof(engine::Vertex))==0,"end clamp changed final pose");
    std::ifstream file(argv[4]);require(bool(file),"missing source fixture");std::string line;float max_error=0;unsigned samples=0,points=0;
    while (std::getline(file,line)) {
        if (line.starts_with("# glb_sha256 ")) require(line.substr(13)==m.asset.sha256,"fixture GLB provenance changed");
        if (line.empty()||line[0]=='#') continue;
        std::istringstream in(line);std::string kind;in>>kind;
        if (kind=="sample") {
            std::uint64_t tick;in>>tick;m.evaluate_tick(tick);++samples;
            const auto origin=staging.batter_blockout_position_m;
            for (const auto* mat:{&m.grip_world,&m.barrel_world,&m.tip_world}) {
                const auto expected=xyz(in);max_error=std::max(max_error,distance({(*mat)[12]-origin.x,(*mat)[13]-origin.y,(*mat)[14]-origin.z},expected));
            }
            const auto first=m.triangles;const auto pose=m.pose.world;m.evaluate_tick(tick);
            require(pose==m.pose.world && std::memcmp(first.data(),m.triangles.data(),first.size()*sizeof(engine::Vertex))==0,"same tick nondeterministic");
        } else {
            std::string name;in>>name;std::size_t p=0;while(p<2 && m.asset.primitives[p].mesh_name!=name) ++p;require(p<2,"fixture mesh");
            if (kind=="bounds") {
                XMFLOAT3 low{1e9f,1e9f,1e9f},high{-1e9f,-1e9f,-1e9f};
                for (const auto& v:m.skinned[p]) {
                    const XMFLOAT3 q{-v.position.x,v.position.y,v.position.z};
                    low={std::min(low.x,q.x),std::min(low.y,q.y),std::min(low.z,q.z)};
                    high={std::max(high.x,q.x),std::max(high.y,q.y),std::max(high.z,q.z)};
                }
                max_error=std::max({max_error,distance(low,xyz(in)),distance(high,xyz(in))});
            } else {
                require(kind=="point","fixture record");const auto expected=xyz(in);float nearest=1e9f;
                for (const auto& v:m.skinned[p]) nearest=std::min(nearest,distance({-v.position.x,v.position.y,v.position.z},expected));
                max_error=std::max(max_error,nearest);++points;
            }
        }
    }
    require(samples>=15 && points==samples*48 && max_error<.0001f,"saved-source / runtime error exceeds 0.1 mm");
    for (unsigned run=0;run<20;++run) {
        m.evaluate_tick(0);require(std::memcmp(initial.data(),m.triangles.data(),initial.size()*sizeof(engine::Vertex))==0,"replay initial differs");
        m.evaluate_tick(m.end_tick);require(std::memcmp(final.data(),m.triangles.data(),final.size()*sizeof(engine::Vertex))==0,"replay final differs");
    }
    // Runtime metadata must not silently select a different time base or marker.
    std::ifstream metadata_input(argv[2]);const std::string metadata{std::istreambuf_iterator<char>(metadata_input),{}};
    const auto temporary=std::filesystem::current_path()/"batter-motion-test-data";std::filesystem::create_directories(temporary);
    const auto fails=[&](const char* name,const std::string& before,const std::string& after,const char* reason) {
        auto text=metadata;const auto at=text.find(before);require(at!=std::string::npos,"metadata fixture target");text.replace(at,before.size(),after);
        const auto path=temporary/name;{std::ofstream out(path);out<<text;}
        try {BatterMotion bad(argv[1],path,staging);}
        catch(const std::exception& error) {
            require(std::string(error.what()).find(reason)!=std::string::npos,"wrong metadata failure");return;
        }
        throw std::runtime_error("bad metadata accepted");
    };
    fails("hash.toml",m.asset.sha256,std::string(64,'0'),"provenance mismatch");
    fails("fps.toml","fps = 60","fps = 30","unsupported swing");
    fails("marker.toml","time_s = 2.0","time_s = 2.1","marker mismatch");
    fails("loop.toml","loop = false","loop = true","unsupported swing");
    std::cout<<"PASS BatterMotion: "<<samples<<" Blender poses, "<<points<<" surface points; max source error_m="<<max_error
        <<"; all 897 ticks finite/rigid attachment/colors/stable storage; 20 repeat initial/final poses.\n";
    return 0;
 } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
