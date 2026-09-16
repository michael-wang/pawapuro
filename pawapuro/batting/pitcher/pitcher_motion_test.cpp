#include "pitcher_motion.hpp"
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
XMFLOAT3 read3(std::istream& in) { XMFLOAT3 p{}; in>>p.x>>p.y>>p.z; require(bool(in),"fixture XYZ"); return p; }
}
int main(int argc,char** argv)
{
    try {
        require(argc==5,"expected GLB, metadata, staging, expected samples");
        const auto staging=load_batting_staging(argv[3]);
        PitcherMotion motion(argv[1],argv[2],staging);
        require(motion.end_tick==816 && motion.release_tick==384 && motion.asset.times.size()==205,"timeline contract");
        const auto& asset=motion.asset;
        require(asset.primitives.front().bind_vertices.size()==2362 && asset.primitives.front().indices.size()==11808,"geometry contract");
        const std::pair<const char*,const char*> hierarchy[]{
            {"root","PitcherRig"},{"pelvis","root"},{"chest","pelvis"},{"head","chest"},
            {"arm_R","chest"},{"forearm_R","arm_R"},{"hand_R","forearm_R"},{"grip","hand_R"},
            {"arm_L","chest"},{"forearm_L","arm_L"},{"hand_L","forearm_L"},{"foot_R","root"},{"foot_L","root"}};
        for (const auto& [name,parent]:hierarchy) {
            const auto node=std::find_if(asset.nodes.begin(),asset.nodes.end(),[&](const auto& n){return n.name==name;});
            require(node!=asset.nodes.end() && node->parent_name==parent,"expected joint hierarchy");
            const auto i=static_cast<std::size_t>(node-asset.nodes.begin());
            require(std::count(asset.joints.begin(),asset.joints.end(),i)==1,"expected skin joint");
            for (const auto path:{engine::GlbPath::Translation,engine::GlbPath::Rotation,engine::GlbPath::Scale})
                require(std::count_if(asset.channels.begin(),asset.channels.end(),[&](const auto& c){return c.node==i && c.path==path;})==1,
                    "expected one TRS channel per joint");
        }
        engine::GlbPose bind;
        engine::evaluate_glb_pose(asset,std::nullopt,bind);
        std::vector<engine::Vertex> vertices;
        engine::skin_glb_vertices(asset.primitives.front(),bind,vertices);
        for (std::size_t i=0;i<vertices.size();++i)
            require(distance(vertices[i].position,asset.primitives.front().bind_vertices[i].position)<0.0001f,"bind skin differs from rest geometry");
        // Mesh-node transforms must not be applied twice to world-space skinning.
        auto moved=asset;
        for (auto& node:moved.nodes) if (node.name=="PitcherMesh") node.local.translation={7,8,9};
        engine::GlbPose moved_pose; engine::evaluate_glb_pose(moved,0.0f,moved_pose);
        engine::skin_glb_vertices(moved.primitives.front(),moved_pose,vertices);
        for (std::size_t i=0;i<vertices.size();++i)
            require(distance(vertices[i].position,motion.skinned[i].position)<1e-6f,"mesh node applied to skin");
        // Antipodal quaternion keys describe the same orientation and must use the shortest path.
        auto antipodal=asset;
        for (auto& ch:antipodal.channels) if (ch.path==engine::GlbPath::Rotation)
            for (std::size_t i=1;i<ch.values.size();i+=2) { auto& q=ch.values[i]; q={-q.x,-q.y,-q.z,-q.w}; }
        engine::GlbPose a,b;
        engine::evaluate_glb_pose(asset,0.004166667f,a); engine::evaluate_glb_pose(antipodal,0.004166667f,b);
        for (std::size_t i=0;i<a.world.size();++i) for (std::size_t j=0;j<16;++j)
            require(std::abs(a.world[i][j]-b.world[i][j])<1e-6f,"quaternion shortest-path regression");
        std::ifstream fixture(argv[4]); require(bool(fixture),"missing expected samples");
        std::string line; float max_error=0; unsigned checked=0;
        const auto placement=staging.pitcher_blockout_position_m;
        while (std::getline(fixture,line)) {
            if (line.empty() || line[0]=='#') continue;
            std::istringstream input(line); std::string kind; input>>kind;
            if (kind=="sample") {
                input>>motion.tick; motion.evaluate();
                const auto grip=read3(input), minimum=read3(input), maximum=read3(input);
                const float grip_error=distance({motion.grip_world[12]-placement.x,motion.grip_world[13]-placement.y,motion.grip_world[14]-placement.z},grip);
                max_error=std::max(max_error,grip_error);
                XMFLOAT3 low{1e9f,1e9f,1e9f}, high{-1e9f,-1e9f,-1e9f};
                for (const auto& v:motion.skinned) {
                    const XMFLOAT3 p{-v.position.x,v.position.y,v.position.z};
                    low={std::min(low.x,p.x),std::min(low.y,p.y),std::min(low.z,p.z)};
                    high={std::max(high.x,p.x),std::max(high.y,p.y),std::max(high.z,p.z)};
                }
                max_error=std::max({max_error,distance(low,minimum),distance(high,maximum)});
                std::cout<<"sample tick="<<motion.tick<<" time="<<motion.time_s()<<" grip_world="<<motion.grip_world[12]<<','
                    <<motion.grip_world[13]<<','<<motion.grip_world[14]<<" source_grip_error="<<grip_error<<'\n'; ++checked;
                const auto first=motion.triangles; const auto first_pose=motion.pose.world; motion.evaluate();
                require(std::memcmp(first.data(),motion.triangles.data(),first.size()*sizeof(engine::Vertex))==0
                    && first_pose==motion.pose.world,"same tick not repeatable in same build");
            } else {
                require(kind=="point","unknown fixture record"); const auto expected=read3(input);
                float closest=1e9f;
                for (const auto& v:motion.skinned) closest=std::min(closest,distance({-v.position.x,v.position.y,v.position.z},expected));
                max_error=std::max(max_error,closest);
            }
        }
        require(checked==8 && max_error<0.0001f,"runtime differs from accepted source by >=0.1 mm");
        // Entire 240 Hz clip: finite geometry, unchanged colors, rigid bone segments and contact intervals.
        const auto index=[&](const char* name) {
            for (std::size_t i=0;i<asset.nodes.size();++i) if (asset.nodes[i].name==name) return i;
            throw std::runtime_error("missing expected bone");
        };
        const auto world_position=[](const engine::GlbMatrix& m) { return XMFLOAT3{m[12],m[13],m[14]}; };
        struct Contact { std::size_t node; unsigned first,last; engine::GlbMatrix start{}; };
        Contact contacts[]{{index("foot_R"),0,336,{}},{index("foot_R"),680,816,{}},
            {index("foot_L"),0,64,{}},{index("foot_L"),312,816,{}}};
        const std::size_t limbs[]{index("forearm_R"),index("hand_R"),index("forearm_L"),index("hand_L")};
        for (motion.tick=0;motion.tick<=motion.end_tick;++motion.tick) {
            motion.evaluate();
            require(motion.triangles.size()==11808,"dynamic count changed");
            for (std::size_t i=0;i<motion.skinned.size();++i)
                require(std::memcmp(&motion.skinned[i].color,&asset.primitives.front().bind_vertices[i].color,sizeof(XMFLOAT3))==0,"color changed");
            for (const auto i:limbs) {
                const auto parent=static_cast<std::size_t>(asset.nodes[i].parent);
                const float expected=distance(world_position(bind.world[i]),world_position(bind.world[parent]));
                require(std::abs(distance(world_position(motion.pose.world[i]),world_position(motion.pose.world[parent]))-expected)<1e-5f,"fixed bone length changed");
            }
            for (auto& contact:contacts) {
                if (motion.tick==contact.first) contact.start=motion.pose.world[contact.node];
                if (motion.tick>=contact.first && motion.tick<=contact.last)
                    for (std::size_t j=0;j<16;++j) require(std::abs(contact.start[j]-motion.pose.world[contact.node][j])<1e-6f,"planted foot moved");
            }
        }
        for (unsigned fps:{30u,60u,120u}) {
            motion.reset(); require(motion.start(),"start failed");
            for (unsigned f=1;f<=fps*2;++f) {
                motion.advance(1'000'000'000ull*f/fps-1'000'000'000ull*(f-1)/fps);
                require(motion.tick==(1'000'000'000ull*f/fps)*240/1'000'000'000ull,"render chunking changed tick");
            }
            require(motion.tick==480,"two-second tick");
            motion.toggle_pause(); const auto tick=motion.tick; const auto paused=motion.triangles;
            motion.advance(5'000'000'000ull); require(motion.tick==tick && std::memcmp(paused.data(),motion.triangles.data(),paused.size()*sizeof(engine::Vertex))==0,"pause advanced");
            motion.single_step(); require(motion.tick==tick+1,"single-step");
            motion.toggle_pause(); motion.advance(10'000'000'000ull);
            while (motion.phase!=MotionPhase::Complete) motion.advance(0);
            require(motion.tick==816,"complete overshot end"); const auto final=motion.triangles;
            motion.advance(1'000'000'000ull); motion.single_step();
            require(motion.tick==816 && std::memcmp(final.data(),motion.triangles.data(),final.size()*sizeof(engine::Vertex))==0,"final pose changed");
            require(motion.start() && motion.tick==0,"replay failed");
        }
        motion.reset(); const auto initial=motion.triangles; motion.start(); require(!motion.start(),"restarted while playing");
        require(std::memcmp(initial.data(),motion.triangles.data(),initial.size()*sizeof(engine::Vertex))==0,"replay initial pose");
        std::cout<<"PASS accepted source max_error_m="<<max_error<<" all ticks, skin, contacts, fixed bones, chunking/pause/replay.\n";
        std::cout<<"CPU measurement samples="<<motion.samples<<" pose_mean_us="<<motion.pose_us/static_cast<double>(motion.samples)
            <<" skin_expand_basis_mean_us="<<motion.skin_us/static_cast<double>(motion.samples)<<'\n';
        return 0;
    } catch (const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
}
