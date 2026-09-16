#include "pitcher_motion.hpp"
#include <toml++/toml.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace pawapuro {
PitcherMotion::PitcherMotion(const std::filesystem::path& glb, const std::filesystem::path& metadata, const BattingStaging& staging)
    : asset(engine::read_mesh_glb(glb)), placement(staging.pitcher_blockout_position_m)
{
    const auto require=[&](bool ok,const char* reason) {
        if (!ok) throw std::runtime_error("PitcherMotion owner=pawapuro/batting/pitcher/pitcher_motion.cpp source='"
            +glb.string()+"' metadata='"+metadata.string()+"': "+reason);
    };
    const auto meta=toml::parse_file(metadata.string());
    require(asset.primitives.size()==1 && asset.primitives.front().mesh_name=="PitcherMesh" && asset.primitives.front().mesh_node_name=="PitcherMesh" && asset.skin_name=="PitcherRig"
        && asset.clip_name=="pitch_R" && asset.joints.size()==13 && asset.nodes.size()==15 && asset.channels.size()==39,
        "unexpected pitcher mesh/skin/clip contract");
    require(meta["schema_version"].value<int>()==1 && meta["glb_sha256"].value<std::string>()==asset.sha256,
        "metadata/GLB provenance mismatch");
    require(meta["clip"]["name"].value<std::string>()==asset.clip_name && meta["clip"]["fps"].value<int>()==60
        && meta["clip"]["fps_base"].value<double>()==1 && meta["clip"]["start_frame"].value<int>()==1
        && meta["clip"]["time_origin_s"].value<double>()==0 && meta["clip"]["playback_speed"].value<double>()==1
        && meta["clip"]["loop"].value<bool>()==false,"unsupported authoring clip metadata");
    const auto duration=meta["clip"]["duration_s"].value_or(-1.0);
    const auto end=meta["clip"]["end_frame"].value_or(-1);
    require(std::isfinite(duration) && duration>0 && duration<60 && std::abs(duration-asset.times.back())<1e-6
        && std::abs(duration-(end-1)/60.0)<1e-6,"clip duration mismatch");
    end_tick=static_cast<std::uint64_t>(std::llround(duration*animation_hz));
    const auto release_time=meta["release"]["time_s"].value_or(-1.0);
    const auto marker_frame=meta["release"]["authoring_frame"].value_or(-1);
    const auto marker_tick=meta["release"]["tick_at_240_hz"].value_or(-1);
    require(meta["release"]["marker"].value<std::string>()=="release"
        && meta["release"]["grip_node"].value<std::string>()=="grip"
        && meta["release"]["grip_parent"].value<std::string>()=="hand_R"
        && std::isfinite(release_time) && release_time>=0 && release_time<=duration
        && std::abs(release_time-(marker_frame-1)/60.0)<1e-8
        && std::abs(release_time*animation_hz-marker_tick)<1e-8,"release diagnostic metadata mismatch");
    release_tick=static_cast<std::uint64_t>(marker_tick);
    unsigned matches=0;
    for (std::size_t i=0;i<asset.nodes.size();++i) if (asset.nodes[i].name=="grip") {
        grip=i; ++matches; require(asset.nodes[i].parent_name=="hand_R","grip hierarchy mismatch");
    }
    require(matches==1,"expected unique grip");
    // Check release alignment without moving Data, snapping the hand, or firing a ball event.
    tick=release_tick; evaluate();
    const auto r=staging.release_position_m;
    const float dx=grip_world[12]-r.x,dy=grip_world[13]-r.y,dz=grip_world[14]-r.z;
    const float error=std::sqrt(dx*dx+dy*dy+dz*dz);
    require(error<0.0001f,"release-tick grip differs from staging reference (>0.1 mm)");
    std::fprintf(stderr,"PitcherMotion: source=%s clip=pitch_R scale=1 joints=%zu vertices=%zu triangles=%zu end_tick=%llu "
        "release_tick=%llu error_m=%.9g; sequencing owned by caller\n",
        glb.string().c_str(),asset.joints.size(),skinned.size(),triangles.size()/3,end_tick,release_tick,error);
    reset(); pose_us=skin_us=0; samples=0;
}
double PitcherMotion::time_s() const { return std::min(static_cast<double>(tick)/animation_hz,static_cast<double>(asset.times.back())); }
void PitcherMotion::evaluate()
{
    const auto before=std::chrono::steady_clock::now();
    engine::evaluate_glb_pose(asset,static_cast<float>(time_s()),pose);
    const auto posed=std::chrono::steady_clock::now();
    engine::skin_glb_vertices(asset.primitives.front(),pose,skinned);
    triangles.resize(asset.primitives.front().indices.size());
    for (std::size_t i=0;i<triangles.size();++i) {
        const auto reversed=i/3*3+(i%3==0 ? 0 : 3-i%3);
        auto v=skinned[asset.primitives.front().indices[reversed]];
        v.position={-v.position.x+placement.x,v.position.y+placement.y,v.position.z+placement.z};
        triangles[i]=v;
    }
    // Change basis for the full node transform: B * M * B, then world placement.
    const auto& g=pose.world[grip];
    for (std::size_t col=0;col<4;++col) for (std::size_t row=0;row<4;++row)
        grip_world[col*4+row]=g[col*4+row]*(col==0 ? -1.0f : 1.0f)*(row==0 ? -1.0f : 1.0f);
    grip_world[12]+=placement.x; grip_world[13]+=placement.y; grip_world[14]+=placement.z;
    const auto finished=std::chrono::steady_clock::now();
    pose_us+=std::chrono::duration<double,std::micro>(posed-before).count();
    skin_us+=std::chrono::duration<double,std::micro>(finished-posed).count(); ++samples;
}
void PitcherMotion::reset() { tick=pending_ticks=fractional_credit=0; paused=false; phase=MotionPhase::Ready; evaluate(); }
bool PitcherMotion::start()
{
    if (phase==MotionPhase::Playing) return false;
    reset(); phase=MotionPhase::Playing; return true;
}
void PitcherMotion::toggle_pause() { if (phase==MotionPhase::Playing) paused=!paused; }
bool PitcherMotion::step_tick()
{
    if (phase!=MotionPhase::Playing) return false;
    if (tick<end_tick) { ++tick; evaluate(); }
    if (tick==end_tick) { phase=MotionPhase::Complete; paused=false; pending_ticks=fractional_credit=0; return true; }
    return false;
}
bool PitcherMotion::single_step() { return phase==MotionPhase::Playing && paused ? step_tick() : false; }
bool PitcherMotion::advance(std::uint64_t elapsed_ns)
{
    if (phase!=MotionPhase::Playing || paused) return false;
    constexpr std::uint64_t ns=1'000'000'000;
    // Bound accumulated work by remaining clip ticks; never accumulate time in floats.
    const auto remaining=end_tick-tick;
    const auto whole=std::min(elapsed_ns/ns,remaining/animation_hz+1)*animation_hz;
    fractional_credit+=(elapsed_ns%ns)*animation_hz;
    pending_ticks=std::min(remaining,pending_ticks+whole+fractional_credit/ns); fractional_credit%=ns;
    for (unsigned n=0;n<16 && pending_ticks;++n) { --pending_ticks; if (step_tick()) return true; }
    return false;
}
const char* PitcherMotion::state_name() const
{
    if (phase==MotionPhase::Ready) return "Ready";
    if (phase==MotionPhase::Complete) return "Complete";
    return paused ? "Paused" : "Playing";
}
}
