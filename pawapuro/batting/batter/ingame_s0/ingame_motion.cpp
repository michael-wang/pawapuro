#include "ingame_motion.hpp"
#include <toml++/toml.hpp>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <chrono>
#include <stdexcept>
using namespace DirectX;
namespace pawapuro {
namespace {
constexpr const char* names[]={"root","pelvis","chest","head","arm_R_0","arm_R_1","arm_R_2","hand_R","bat_grip","bat_barrel","bat_tip","arm_L_0","arm_L_1","arm_L_2","hand_L","foot_R","foot_L"};
XMMATRIX load(const engine::GlbMatrix& a) { XMFLOAT4X4 m;std::memcpy(&m,a.data(),sizeof(m));return XMLoadFloat4x4(&m); }
engine::GlbMatrix save(FXMMATRIX m) { XMFLOAT4X4 a;XMStoreFloat4x4(&a,m);engine::GlbMatrix b;std::memcpy(b.data(),&a,sizeof(a));return b; }
XMMATRIX matrix(const engine::GlbTrs& t) { return XMMatrixRotationQuaternion(XMLoadFloat4(&t.rotation))*XMMatrixTranslation(t.translation.x,t.translation.y,t.translation.z); }
engine::GlbTrs trs(FXMMATRIX m) { XMVECTOR s,q,p;if(!XMMatrixDecompose(&s,&q,&p,m))throw std::runtime_error("Ingame pose decomposition failed");engine::GlbTrs t;XMStoreFloat3(&t.translation,p);XMStoreFloat4(&t.rotation,XMQuaternionNormalize(q));return t; }
XMVECTOR mul(FXMVECTOR a,FXMVECTOR b) { return XMQuaternionMultiply(b,a); }
XMVECTOR log_rotation(FXMVECTOR q0) {
    auto q=XMQuaternionNormalize(q0);if(XMVectorGetW(q)<0)q=XMVectorNegate(q);
    const float w=std::clamp(XMVectorGetW(q),-1.f,1.f),s=std::sqrt(std::max(0.f,1-w*w));
    // Preserve the saved Blender 4.5 tiny-angle convention used by Gate B.
    return XMVectorScale(XMVectorSetW(q,0),2*std::acos(w)/(s<0.0005f?1.f:s));
}
XMVECTOR exp_rotation(FXMVECTOR v) {
    const float a=XMVectorGetX(XMVector3Length(v));
    if(a<1e-7f)return XMQuaternionNormalize(XMVectorSetW(XMVectorScale(v,.5f),1));
    return XMVectorSetW(XMVectorScale(v,std::sin(a/2)/a),std::cos(a/2));
}
double hermite(double a,double b,double va,double vb,double u,double d) {return (2*u*u*u-3*u*u+1)*a+(u*u*u-2*u*u+u)*d*va+(-2*u*u*u+3*u*u)*b+(u*u*u-u*u)*d*vb;}
double curve(std::initializer_list<std::pair<double,double>> keys,double f) {
    auto a=keys.begin();if(f<=a->first)return a->second;
    for(auto b=a+1;b!=keys.end();a=b++)if(f<=b->first){auto u=(f-a->first)/(b->first-a->first);return a->second+(b->second-a->second)*u*u*(3-2*u);}
    return a->second;
}
double swing_frame(double x) {
    constexpr double knots[][3]={{0,99,0},{4,113,1.6},{6,116,1.5},{10,121,1},{114,225,1}};
    if(x<=0)return 99;if(x>=114)return 225;
    for(unsigned i=1;i<5;++i)if(x<=knots[i][0]){auto a=knots[i-1],b=knots[i];return hermite(a[1],b[1],a[2],b[2],(x-a[0])/(b[0]-a[0]),b[0]-a[0]);}
    return 225;
}
}
IngameMotion::IngameMotion(const std::filesystem::path& d,const BattingStaging& staging)
    :asset(engine::read_mesh_glb(d/"motion.glb")),placement(staging.batter_blockout_position_m)
{
    const auto meta=toml::parse_file((d/"motion.toml").string());
    if(meta["schema_version"].value<int>()!=1 || meta["glb_sha256"].value<std::string>()!=asset.sha256
        || meta["source_sha256"].value<std::string>()!="9e37b73e8419e721b885326e06ba3b185fbd11168da77184a13e2b07d6961ea6"
        || meta["entry_frames"].value<int>()!=6 || meta["velocity_epsilon_frames"].value<double>()!=.01
        || meta["source_frames"].value<int>()!=225 || meta["source_fps"].value<int>()!=60
        || meta["commit_first_tick"].value<int>()!=432 || meta["commit_last_tick"].value<int>()!=496
        || asset.clip_name!="ingame_source" || asset.times.size()!=225
        || meta["recipe"].value<std::string>()!="gate-b-world-residual-v1" || asset.joints.size()!=bone_count || asset.primitives.size()!=2)
        throw std::runtime_error("Ingame S0 asset/recipe mismatch");
    const auto* knots=meta["plant_duration_knots"].as_array();
    constexpr double expected[][2]={{109,6},{115,3},{117,0}};
    if(!knots||knots->size()!=3)throw std::runtime_error("Ingame support recipe mismatch");
    for(std::size_t i=0;i<3;++i){const auto* row=(*knots)[i].as_array();
        if(!row||row->size()!=2||(*row)[0].value<double>()!=expected[i][0]||(*row)[1].value<double>()!=expected[i][1])throw std::runtime_error("Ingame support recipe mismatch");}
    for(std::size_t b=0;b<bone_count;++b) {
        auto it=std::find_if(asset.nodes.begin(),asset.nodes.end(),[&](const auto& n){return n.name==names[b];});
        if(it==asset.nodes.end())throw std::runtime_error("Ingame S0 missing bone");
        nodes[b]=static_cast<std::size_t>(it-asset.nodes.begin());
        auto joint=std::find(asset.joints.begin(),asset.joints.end(),nodes[b]);
        if(joint==asset.joints.end())throw std::runtime_error("Ingame S0 missing joint");
        rest[b]=save(XMMatrixInverse(nullptr,load(asset.inverse_binds[static_cast<std::size_t>(joint-asset.joints.begin())])));
    }
    engine::GlbPose scratch;
    for(unsigned f=0;f<225;++f) {
        engine::evaluate_glb_pose(asset,static_cast<float>(f)/60,scratch);
        for(std::size_t b=0;b<bone_count;++b)source[f][b]=trs(load(scratch.world[nodes[b]]));
    }
    std::size_t count=0;
    for(std::size_t i=0;i<2;++i){skinned[i].resize(asset.primitives[i].bind_vertices.size());count+=asset.primitives[i].indices.size();}
    triangles.resize(count);evaluate_tick(0,std::nullopt);
}
IngameMotion::Bones IngameMotion::source_pose(double frame) const {
    const double f=std::clamp(frame,1.,225.)-1;const auto lo=static_cast<std::size_t>(f),hi=std::min(lo+1,std::size_t{224});
    Bones result;const float u=static_cast<float>(f-static_cast<double>(lo));
    for(std::size_t b=0;b<bone_count;++b){const auto& a=source[lo][b];const auto& c=source[hi][b];
        XMStoreFloat3(&result[b].translation,XMVectorLerp(XMLoadFloat3(&a.translation),XMLoadFloat3(&c.translation),u));
        XMStoreFloat4(&result[b].rotation,XMQuaternionNormalize(XMQuaternionSlerp(XMLoadFloat4(&a.rotation),XMLoadFloat4(&c.rotation),u)));}
    return result;
}
engine::GlbTrs IngameMotion::front_foot(double f) const {
    const float lift=static_cast<float>(curve({{1,0},{91,0},{103,.035},{117,0},{241,0}},f));
    const float angle=static_cast<float>(curve({{1,0},{91,0},{101,8},{117,0},{241,0}},f))*XM_PI/180;
    const float dz=static_cast<float>(curve({{1,0},{95,0},{117,.15},{241,.15}},f));
    auto rotation=XMMatrixRotationZ(-angle);
    auto center=XMVectorSubtract(XMVectorSet(.464f,lift,.34f+dz,1),XMVector3TransformNormal(XMVectorSet(.384f,-.06885f,0,0),rotation));
    auto orientation=load(rest[15]);orientation.r[3]=XMVectorSet(0,0,0,1);orientation=orientation*rotation;orientation.r[3]=center;
    return trs(orientation);
}
IngameMotion::Bones IngameMotion::preparation(double f) const {
    auto m=source_pose(curve({{1,1},{77,65},{103,88},{117,99},{125,99},{169,1},{241,1}},f));
    m[15]=front_foot(f);m[16]=source[0][16];return m;
}
double IngameMotion::plant_frame(double c) const {
    if(c>=117)return 117;
    // Continuous remaining descent through the two accepted incomplete-plant fixtures.
    const double duration=c<=115?6-(c-109)*.5:3-(c-115)*1.5;
    return c+duration;
}
void IngameMotion::semantics(Bones& m) const {
    auto rear=matrix(m[16]);const auto toe=XMVectorSet(.464f,0,-.34f,1);
    const auto local=XMVector3TransformCoord(toe,XMMatrixInverse(nullptr,load(rest[16])));
    rear.r[3]=XMVectorSetW(XMVectorSubtract(toe,XMVector3TransformNormal(local,rear)),1);m[16]=trs(rear);
    m[14]=trs(load(rest[14])*XMMatrixInverse(nullptr,load(rest[7]))*matrix(m[7]));
}
void IngameMotion::sample_key(double tick_value,std::optional<std::uint64_t> commit,engine::GlbPose& out) const {
    if(!std::isfinite(tick_value)||tick_value<0 || (commit&&(*commit<432||*commit>496)))throw std::runtime_error("Ingame sample outside domain");
    const double f=1+tick_value/4;auto m=preparation(f);
    if(commit&&tick_value>static_cast<double>(*commit)) {
        const double c=1+static_cast<double>(*commit)/4,x=f-c; m=source_pose(swing_frame(x));
        if(x<6) {
            const auto p=preparation(c),pm=preparation(c-.01),pp=preparation(c+.01);const double u=x/6;
            const float h=static_cast<float>(2*u*u*u-3*u*u+1),g=static_cast<float>((u*u*u-2*u*u+u)*6);
            for(std::size_t b=0;b<15;++b)if(b!=8&&b!=9&&b!=10&&b!=14) {
                const auto v=XMVectorScale(XMVectorSubtract(XMLoadFloat3(&pp[b].translation),XMLoadFloat3(&pm[b].translation)),50);
                const auto w=XMVectorScale(log_rotation(mul(XMLoadFloat4(&pp[b].rotation),XMQuaternionInverse(XMLoadFloat4(&pm[b].rotation)))),50);
                const auto delta=XMVectorSubtract(XMLoadFloat3(&p[b].translation),XMLoadFloat3(&source[98][b].translation));
                XMStoreFloat3(&m[b].translation,XMVectorAdd(XMLoadFloat3(&m[b].translation),XMVectorAdd(XMVectorScale(delta,h),XMVectorScale(v,g))));
                const auto q=log_rotation(mul(XMLoadFloat4(&p[b].rotation),XMQuaternionInverse(XMLoadFloat4(&source[98][b].rotation))));
                XMStoreFloat4(&m[b].rotation,XMQuaternionNormalize(mul(exp_rotation(XMVectorAdd(XMVectorScale(q,h),XMVectorScale(w,g))),XMLoadFloat4(&m[b].rotation))));
            }
        }
        const double plant=plant_frame(c);
        m[15]=c>=117||f>=plant?front_foot(117):front_foot(hermite(c,117,1,1,(f-c)/(plant-c),plant-c));
        semantics(m);
    }
    m[0]=trs(load(rest[0]));
    // Gate B key_pose fixes the equipment local channels even during Take.
    for(const auto b:{8,9,10}){const int parent=b==8?7:8;
        m[b]=trs(load(rest[b])*XMMatrixInverse(nullptr,load(rest[parent]))*matrix(m[parent]));}

    engine::evaluate_glb_pose(asset,std::nullopt,out);
    for(std::size_t b=0;b<bone_count;++b)out.world[nodes[b]]=save(matrix(m[b]));
    // Convert the completed world pose to local TRS once, then use the common hierarchy/skinning primitive.
    for(std::size_t b=0;b<bone_count;++b) {
        const auto n=nodes[b];auto local=load(out.world[n]);const int parent=asset.nodes[n].parent;
        if(parent>=0)local=local*XMMatrixInverse(nullptr,load(out.world[static_cast<std::size_t>(parent)]));
        out.local[n]=trs(local);
    }
    engine::rebuild_glb_pose(asset,out);
}
void IngameMotion::sample(double value,std::optional<std::uint64_t> commit,engine::GlbPose& out) const {
    if(!std::isfinite(value)||value<0)throw std::runtime_error("Invalid fractional preview tick");
    const double lo=std::floor(value);sample_key(lo,commit,out);
    if(value==lo)return;
    Bones lower;for(std::size_t b=0;b<bone_count;++b)lower[b]=out.local[nodes[b]];
    sample_key(lo+1,commit,out);const float u=static_cast<float>(value-lo);
    // Saved quarter-frame keys interpolate local quaternion components, then normalize.
    for(std::size_t b=0;b<bone_count;++b){auto& t=out.local[nodes[b]];
        XMStoreFloat3(&t.translation,XMVectorLerp(XMLoadFloat3(&lower[b].translation),XMLoadFloat3(&t.translation),u));
        auto a=XMLoadFloat4(&lower[b].rotation),q=XMLoadFloat4(&t.rotation);
        if(XMVectorGetX(XMVector4Dot(a,q))<0)q=XMVectorNegate(q);
        XMStoreFloat4(&t.rotation,XMQuaternionNormalize(XMVectorLerp(a,q,u)));
    }
    engine::rebuild_glb_pose(asset,out);
}
BatBarrelSample IngameMotion::sample_barrel(double time,std::uint64_t commit,engine::GlbPose& scratch) const {
    sample(time*240,commit,scratch);
    const auto world=[&](std::size_t bone) {
        auto out=scratch.world[nodes[bone]];
        for(std::size_t col=0;col<4;++col)for(std::size_t row=0;row<4;++row)
            out[col*4+row]*=(col==0?-1.f:1.f)*(row==0?-1.f:1.f);
        out[12]+=placement.x;out[13]+=placement.y;out[14]+=placement.z;return out;
    };
    const auto b=world(9),t=world(10);
    return {{b[12],b[13],b[14]},{t[12],t[13],t[14]},b};
}
void IngameMotion::evaluate_tick(std::uint64_t value,std::optional<std::uint64_t> commit) {
    end_tick=commit?*commit+456:800;tick=std::min(value,end_tick);const auto begin=std::chrono::steady_clock::now();sample(static_cast<double>(tick),commit,pose);
    pose_us+=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-begin).count();
    std::size_t offset=0;
    for(std::size_t p=0;p<2;++p) {
        const auto& primitive=asset.primitives[p];const auto before=std::chrono::steady_clock::now();engine::skin_glb_vertices(primitive,pose,skinned[p]);
        for(std::size_t i=0;i<primitive.indices.size();++i){const auto reverse=i/3*3+(i%3==0?0:3-i%3);auto v=skinned[p][primitive.indices[reverse]];
            v.position={-v.position.x+placement.x,v.position.y+placement.y,v.position.z+placement.z};triangles[offset+i]=v;}
        (primitive.mesh_name=="BatterMesh"?body_skin_us:bat_skin_us)+=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-before).count();
        offset+=primitive.indices.size();
    }
    for(const auto b:{8,9,10}) {
        auto& out=b==8?grip_world:b==9?barrel_world:tip_world;const auto& mat=pose.world[nodes[b]];
        for(std::size_t col=0;col<4;++col)for(std::size_t row=0;row<4;++row)out[col*4+row]=mat[col*4+row]*(col==0?-1.f:1.f)*(row==0?-1.f:1.f);
        out[12]+=placement.x;out[13]+=placement.y;out[14]+=placement.z;
    }
    ++samples;
}
}
