#include "ingame_motion.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
DirectX::XMMATRIX matrix(const engine::GlbMatrix& v){DirectX::XMFLOAT4X4 m;std::memcpy(&m,v.data(),sizeof(m));return DirectX::XMLoadFloat4x4(&m);}
int main(int argc,char** argv) {
    try {
        if(argc!=2)throw std::runtime_error("expected batting directory");
        const std::filesystem::path d=argv[1];pawapuro::IngameMotion motion(d/"batter/ingame_s0",pawapuro::load_batting_staging(d/"staging.toml"));
        const auto baseline=engine::read_mesh_glb(d/"batter/batter.glb");
        for(const auto& primitive:motion.asset.primitives){
            const auto old=std::find_if(baseline.primitives.begin(),baseline.primitives.end(),[&](const auto& x){return x.mesh_name==primitive.mesh_name;});
            if(old==baseline.primitives.end()||old->indices!=primitive.indices||old->bind_vertices.size()!=primitive.bind_vertices.size())throw std::runtime_error("protected mesh topology changed");
            for(std::size_t i=0;i<primitive.bind_vertices.size();++i){
                if(std::memcmp(&primitive.bind_vertices[i],&old->bind_vertices[i],sizeof(engine::Vertex))!=0)throw std::runtime_error("protected bind geometry/color changed");
                const auto& a=primitive.influences[i];const auto& b=old->influences[i];
                for(unsigned k=0;k<4;++k)if(a.weights[k]!=b.weights[k] || (a.weights[k]>0&&motion.asset.nodes[motion.asset.joints[a.joints[k]]].name!=baseline.nodes[baseline.joints[b.joints[k]]].name))throw std::runtime_error("protected skin weights changed");
            }
        }
        std::ifstream f(d/"batter/ingame_s0/motion_expected.txt");if(!f)throw std::runtime_error("missing saved-source fixtures");
        engine::GlbPose scratch;std::string line,worst;std::string last_case,integer_worst;double case_max=0,integer_max=0;double max_error=0;unsigned count=0;
        while(std::getline(f,line)) {if(line.empty()||line[0]=='#')continue;std::istringstream row(line);std::string name,bone;std::uint64_t c;double tick;row>>name>>c>>tick>>bone;
            if(name!=last_case){std::cout<<last_case<<" max="<<case_max<<"\n";last_case=name;case_max=0;}
            motion.sample(tick,c?std::optional(c):std::nullopt,scratch);
            auto n=std::find_if(motion.asset.nodes.begin(),motion.asset.nodes.end(),[&](const auto& x){return x.name==bone;});
            if(n==motion.asset.nodes.end())throw std::runtime_error("fixture bone missing");
            const auto& m=scratch.world[static_cast<std::size_t>(n-motion.asset.nodes.begin())];
            for(unsigned i=0;i<16;++i){double expected;row>>expected;const auto e=std::abs(expected-m[i]);case_max=std::max(case_max,e);if(tick==std::floor(tick)&&e>integer_max){integer_max=e;integer_worst=name+" "+std::to_string(tick)+" "+bone;}if(e>max_error){max_error=e;worst=name+" "+std::to_string(tick)+" "+bone+" component "+std::to_string(i);}}
            if(!row)throw std::runtime_error("invalid fixture");++count;
        }
        std::cout<<"integer_max="<<integer_max<<" at "<<integer_worst<<"\n";std::cout<<"source matrices="<<count<<" max_error="<<max_error<<" at "<<worst<<'\n';
        if(max_error>0.0001)throw std::runtime_error("saved-source pose mismatch (>1e-4)");
        const auto node=[&](const char* name){for(std::size_t i=0;i<motion.asset.nodes.size();++i)if(motion.asset.nodes[i].name==name)return i;throw std::runtime_error("missing node");};
        const auto front=node("foot_R"),rear=node("foot_L"),hand=node("hand_R"),left=node("hand_L"),grip=node("bat_grip");
        const auto joint=[&](std::size_t n){return static_cast<std::size_t>(std::find(motion.asset.joints.begin(),motion.asset.joints.end(),n)-motion.asset.joints.begin());};
        const auto front_joint=joint(front),rear_joint=joint(rear);
        std::array<std::vector<DirectX::XMFLOAT3>,2> foot_points;
        for(const auto& primitive:motion.asset.primitives)for(std::size_t i=0;i<primitive.bind_vertices.size();++i){const auto& inf=primitive.influences[i];
            for(unsigned k=0;k<4;++k)if(inf.weights[k]>.999f){if(inf.joints[k]==front_joint)foot_points[0].push_back(primitive.bind_vertices[i].position);if(inf.joints[k]==rear_joint)foot_points[1].push_back(primitive.bind_vertices[i].position);}}
        if(foot_points[0].empty()||foot_points[1].empty())throw std::runtime_error("missing support geometry");
        float min_sole=1,max_slide=0,max_hand=0,max_grip=0,max_prefix=0;engine::GlbPose take;
        const auto rest=[&](std::size_t n){return DirectX::XMMatrixInverse(nullptr,matrix(motion.asset.inverse_binds[joint(n)]));};
        for(std::uint64_t c=432;c<=496;++c){
            engine::GlbMatrix planted{};bool has_plant=false;
            for(unsigned t=0;t<=c+456;++t){
                motion.sample(t,c,scratch);for(const auto& m:scratch.world)for(float v:m)if(!std::isfinite(v))throw std::runtime_error("nonfinite pose");
                if(t<=c){motion.sample(t,std::nullopt,take);for(std::size_t n=0;n<scratch.world.size();++n)for(unsigned k=0;k<16;++k)max_prefix=std::max(max_prefix,std::abs(scratch.world[n][k]-take.world[n][k]));}
                if(1+t/4.>=motion.plant_frame(1+c/4.)){
                    if(!has_plant){planted=scratch.world[front];has_plant=true;}
                    for(unsigned k=0;k<16;++k)max_slide=std::max(max_slide,std::abs(planted[k]-scratch.world[front][k]));
                }
                for(unsigned side=0;side<2;++side)for(const auto& point:foot_points[side]){
                    const auto v=DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&point),matrix(scratch.skin[side?rear_joint:front_joint]));min_sole=std::min(min_sole,DirectX::XMVectorGetY(v));}
                for(const auto n:{left,grip}){
                    if(n==left&&t<=c)continue;
                    const auto expected=rest(n)*DirectX::XMMatrixInverse(nullptr,rest(hand))*matrix(scratch.world[hand]);
                    const auto actual=matrix(scratch.world[n]);const auto error=DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(expected.r[3],actual.r[3])));
                    (n==left?max_hand:max_grip)=std::max(n==left?max_hand:max_grip,error);
                }
            }
            motion.evaluate_tick(c+456,c);if(!motion.complete())throw std::runtime_error("incomplete finish");
            const auto before=motion.barrel_world;motion.sample(c+39.5,c,scratch);if(before!=motion.barrel_world)throw std::runtime_error("read-only sample mutates authoritative semantics");
        }
        std::cout<<"prefix="<<max_prefix<<" sole_min="<<min_sole<<" planted_drift="<<max_slide<<" hand="<<max_hand<<" grip="<<max_grip<<'\n';
        if(max_prefix!=0||min_sole< -1e-6f||max_slide>2e-6f||max_hand>1e-4f||max_grip>2e-6f)throw std::runtime_error("support/attachment/prefix invariant failed");
        std::cout<<"65 continuous commit ticks finite and complete\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
