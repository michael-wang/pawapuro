#include "mesh_glb.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

using namespace DirectX;
namespace engine {
namespace {
// glTF column-major bytes equal the transposed row-vector DirectX matrix storage.
XMMATRIX load(const GlbMatrix& values)
{
    XMFLOAT4X4 m; std::memcpy(&m,values.data(),sizeof(m)); return XMLoadFloat4x4(&m);
}
void save(GlbMatrix& values, FXMMATRIX m)
{
    XMFLOAT4X4 out; XMStoreFloat4x4(&out,m); std::memcpy(values.data(),&out,sizeof(out));
    if (!std::all_of(values.begin(),values.end(),[](float x){return std::isfinite(x);}))
        throw std::runtime_error("GLB pose produced a non-finite matrix");
}
}
void evaluate_glb_pose(const MeshGlb& mesh, std::optional<float> time_s, GlbPose& pose)
{
    pose.local.resize(mesh.nodes.size()); pose.world.resize(mesh.nodes.size()); pose.skin.resize(mesh.joints.size());
    for (std::size_t i=0;i<mesh.nodes.size();++i) pose.local[i]=mesh.nodes[i].local;
    if (time_s) {
        if (!std::isfinite(*time_s)) throw std::runtime_error("GLB sample time must be finite");
        const float time=std::clamp(*time_s,mesh.times.front(),mesh.times.back());
        const auto upper=std::upper_bound(mesh.times.begin(),mesh.times.end(),time);
        const auto hi=std::min(static_cast<std::size_t>(upper-mesh.times.begin()),mesh.times.size()-1);
        const auto lo=hi ? hi-1 : 0;
        const float t=hi==lo ? 0 : (time-mesh.times[lo])/(mesh.times[hi]-mesh.times[lo]);
        for (const auto& channel:mesh.channels) {
            auto& local=pose.local[channel.node];
            auto a=XMLoadFloat4(&channel.values[lo]), b=XMLoadFloat4(&channel.values[hi]);
            if (channel.path==GlbPath::Rotation) {
                a=XMQuaternionNormalize(a); b=XMQuaternionNormalize(b);
                if (XMVectorGetX(XMVector4Dot(a,b))<0) b=XMVectorNegate(b);
                XMStoreFloat4(&local.rotation,XMQuaternionNormalize(XMQuaternionSlerp(a,b,t)));
            } else {
                auto& value=channel.path==GlbPath::Translation ? local.translation : local.scale;
                XMStoreFloat3(&value,XMVectorLerp(a,b,t));
            }
        }
    }
    for (const auto i:mesh.hierarchy_order) {
        const auto& local=pose.local[i];
        auto world=XMMatrixScaling(local.scale.x,local.scale.y,local.scale.z)
            *XMMatrixRotationQuaternion(XMQuaternionNormalize(XMLoadFloat4(&local.rotation)))
            *XMMatrixTranslation(local.translation.x,local.translation.y,local.translation.z);
        if (mesh.nodes[i].parent>=0) world=world*load(pose.world[mesh.nodes[i].parent]);
        save(pose.world[i],world);
    }
    // World-space LBS: IBM first, then joint world. No extra mesh-node transform.
    for (std::size_t i=0;i<mesh.joints.size();++i)
        save(pose.skin[i],load(mesh.inverse_binds[i])*load(pose.world[mesh.joints[i]]));
}
void skin_glb_vertices(const MeshGlb& mesh, const GlbPose& pose, std::vector<Vertex>& vertices)
{
    vertices.resize(mesh.bind_vertices.size());
    for (std::size_t i=0;i<vertices.size();++i) {
        const auto& input=mesh.bind_vertices[i]; const auto& influence=mesh.influences[i];
        auto result=XMVectorZero();
        for (std::size_t j=0;j<4;++j) if (influence.weights[j]>0)
            result=XMVectorMultiplyAdd(XMVector3TransformCoord(XMLoadFloat3(&input.position),load(pose.skin[influence.joints[j]])),
                XMVectorReplicate(influence.weights[j]),result);
        vertices[i].color=input.color; XMStoreFloat3(&vertices[i].position,result);
        const auto p=vertices[i].position;
        if (!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z))
            throw std::runtime_error("GLB skinning produced a non-finite vertex");
    }
}
}
