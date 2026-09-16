#include "batter_motion.hpp"
#include "pawapuro/batting/pitcher/pitcher_motion.hpp"
#include <toml++/toml.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <regex>
#include <stdexcept>

namespace pawapuro {
BatterMotion::BatterMotion(const std::filesystem::path& glb,const std::filesystem::path& metadata,const BattingStaging& staging)
    : asset(engine::read_mesh_glb(glb)),placement(staging.batter_blockout_position_m)
{
    const auto require=[&](bool ok,const char* reason) {
        if (!ok) throw std::runtime_error("BatterMotion source='"+glb.string()+"' metadata='"+metadata.string()+"': "+reason);
    };
    const auto meta=toml::parse_file(metadata.string());
    require(asset.primitives.size()==2 && asset.skin_name=="BatterRig" && asset.joints.size()==17
        && asset.clip_name=="swing_L","expected two primitives, one BatterRig skin and swing_L");
    require(meta["schema_version"].value<int>()==1 && meta["glb_sha256"].value<std::string>()==asset.sha256,
        "metadata/GLB provenance mismatch");
    require(meta["clip"]["name"].value<std::string>()==asset.clip_name && meta["clip"]["fps"].value<int>()==60
        && meta["clip"]["fps_base"].value<double>()==1 && meta["clip"]["start_frame"].value<int>()==1
        && meta["clip"]["time_origin_s"].value<double>()==0 && meta["clip"]["loop"].value<bool>()==false,
        "unsupported swing authoring metadata");
    const auto end=meta["clip"]["end_frame"].value_or(-1);
    const auto duration=meta["clip"]["duration_s"].value_or(-1.0);
    require(end>1 && std::isfinite(duration) && duration>0 && duration<60
        && std::abs(duration-(end-1)/60.0)<1e-6 && std::abs(duration-asset.times.back())<1e-6,"clip duration mismatch");
    end_tick=static_cast<std::uint64_t>(std::llround(duration*animation_hz));
    const auto marker_frame=meta["contact_area"]["authoring_frame"].value_or(-1);
    const auto marker_time=meta["contact_area"]["time_s"].value_or(-1.0);
    require(meta["contact_area"]["marker"].value<std::string>()=="contact_area"
        && marker_frame>=1 && marker_frame<=end && std::isfinite(marker_time)
        && std::abs(marker_time-(marker_frame-1)/60.0)<1e-8,"contact_area marker mismatch");
    contact_area_tick=static_cast<std::uint64_t>(std::llround(marker_time*animation_hz));
    // Existing review.key_poses is a flat JSON string. Read only these two named
    // authoring diagnostics; it is neither a runtime event DSL nor playback control.
    const auto poses=meta["review"]["key_poses"].value_or(std::string{});
    const auto pose_tick=[&](const char* name) {
        const std::regex pattern(std::string("\"([0-9]+)\"\\s*:\\s*\"")+name+"\"");
        auto it=std::sregex_iterator(poses.begin(),poses.end(),pattern), stop=std::sregex_iterator();
        require(it!=stop,"missing named diagnostic pose");const auto frame=std::stoll((*it)[1]);
        require(++it==stop && frame>=1 && frame<=end,"invalid/duplicate named diagnostic pose");
        return static_cast<std::uint64_t>((frame-1)*animation_hz/60);
    };
    gather_tick=pose_tick("Gather");plant_tick=pose_tick("Plant");
    const auto node_index=[&](const char* name) {
        std::size_t found=0,count=0;
        for (std::size_t i=0;i<asset.nodes.size();++i) if (asset.nodes[i].name==name) {found=i;++count;}
        require(count==1,"expected unique bat semantic/hand node");return found;
    };
    grip=node_index("bat_grip");barrel=node_index("bat_barrel");tip=node_index("bat_tip");
    require(meta["bat"]["mesh"].value<std::string>()=="Bat" && meta["bat"]["grip"].value<std::string>()=="bat_grip"
        && meta["bat"]["parent"].value<std::string>()=="hand_R" && meta["bat"]["barrel"].value<std::string>()=="bat_barrel"
        && meta["bat"]["tip"].value<std::string>()=="bat_tip","bat semantic metadata mismatch");
    require(asset.nodes[grip].parent==static_cast<int>(node_index("hand_R"))
        && asset.nodes[barrel].parent==static_cast<int>(grip) && asset.nodes[tip].parent==static_cast<int>(grip),"bat hierarchy mismatch");
    require(std::isfinite(placement.x) && std::isfinite(placement.y) && std::isfinite(placement.z),"invalid staging placement");
    bool body=false,bat=false;std::size_t expanded=0;
    for (std::size_t i=0;i<asset.primitives.size();++i) {
        const auto& p=asset.primitives[i];require(p.mesh_name==p.mesh_node_name,"mesh/node identity mismatch");
        if (p.mesh_name=="BatterMesh") { require(!body,"duplicate body");body=true;body_primitive=i; }
        else {
            require(p.mesh_name=="Bat" && !bat,"expected unique Bat");bat=true;
            for (const auto& influence:p.influences) for (std::size_t j=0;j<4;++j)
                if (influence.weights[j]>0) require(asset.joints[influence.joints[j]]==grip,"Bat attachment differs from bat_grip");
        }
        skinned[i].resize(p.bind_vertices.size());expanded+=p.indices.size();
    }
    require(body && bat,"BatterMesh/Bat missing");triangles.resize(expanded);
    evaluate_tick(0);pose_us=body_skin_us=bat_skin_us=0;samples=0;
    std::fprintf(stderr,"BatterMotion: source=%s clip=swing_L scale=1 gather_tick=%llu plant_tick=%llu contact_area_tick=%llu end_tick=%llu vertices=%zu; no wall clock\n",
        glb.string().c_str(),gather_tick,plant_tick,contact_area_tick,end_tick,triangles.size());
}
BatBarrelSample BatterMotion::sample_barrel(double time,engine::GlbPose& scratch) const
{
    if (!std::isfinite(time) || time<0 || time>static_cast<double>(end_tick)/animation_hz)
        throw std::runtime_error("Bat semantic study time outside accepted clip.");
    engine::evaluate_glb_pose(asset,static_cast<float>(time),scratch);
    const auto point=[&](std::size_t node) {
        const auto& m=scratch.world[node];
        return DirectX::XMFLOAT3{-m[12]+placement.x,m[13]+placement.y,m[14]+placement.z};
    };
    return {point(barrel),point(tip)};
}
void BatterMotion::evaluate_tick(std::uint64_t authoritative_tick)
{
    tick=std::min(authoritative_tick,end_tick);
    const auto before=std::chrono::steady_clock::now();
    engine::evaluate_glb_pose(asset,static_cast<float>(static_cast<double>(tick)/animation_hz),pose);
    const auto posed=std::chrono::steady_clock::now();
    pose_us+=std::chrono::duration<double,std::micro>(posed-before).count();
    std::size_t offset=0;
    for (std::size_t primitive=0;primitive<asset.primitives.size();++primitive) {
        const auto& p=asset.primitives[primitive];auto& vertices=skinned[primitive];
        const auto start=std::chrono::steady_clock::now();engine::skin_glb_vertices(p,pose,vertices);
        const auto stop=std::chrono::steady_clock::now();
        (primitive==body_primitive?body_skin_us:bat_skin_us)+=std::chrono::duration<double,std::micro>(stop-start).count();
        for (std::size_t i=0;i<p.indices.size();++i) {
            const auto reverse=i/3*3+(i%3==0?0:3-i%3);auto v=vertices[p.indices[reverse]];
            v.position={-v.position.x+placement.x,v.position.y+placement.y,v.position.z+placement.z};triangles[offset+i]=v;
        }
        offset+=p.indices.size();
    }
    const auto world=[&](std::size_t node,engine::GlbMatrix& out) {
        const auto& m=pose.world[node];
        for (std::size_t col=0;col<4;++col) for (std::size_t row=0;row<4;++row)
            out[col*4+row]=m[col*4+row]*(col==0?-1.f:1.f)*(row==0?-1.f:1.f);
        out[12]+=placement.x;out[13]+=placement.y;out[14]+=placement.z;
    };
    world(grip,grip_world);world(barrel,barrel_world);world(tip,tip_world);++samples;
}
}
