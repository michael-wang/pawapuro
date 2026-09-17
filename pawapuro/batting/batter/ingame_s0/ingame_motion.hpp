#pragma once
#include "engine/rendering/mesh_glb.hpp"
#include "pawapuro/batting/staging.hpp"
#include <optional>
#include "../batter_motion.hpp"

namespace pawapuro {
// Owns immutable source samples, reusable pose/skin scratch and expanded geometry.
// All times are preview ticks (including fractional read-only samples), never wall time.
struct IngameMotion {
    static constexpr std::size_t bone_count=17;
    using Bones=std::array<engine::GlbTrs,bone_count>;
    IngameMotion(const std::filesystem::path& directory,const BattingStaging& staging);
    const engine::MeshGlb asset;
    engine::GlbPose pose;
    std::array<std::vector<engine::Vertex>,2> skinned;
    std::vector<engine::Vertex> triangles;
    engine::GlbMatrix grip_world{},barrel_world{},tip_world{};
    std::uint64_t tick=0,end_tick=800,samples=0;
    double pose_us=0,body_skin_us=0,bat_skin_us=0;
    void evaluate_tick(std::uint64_t tick,std::optional<std::uint64_t> commit);
    void sample(double tick,std::optional<std::uint64_t> commit,engine::GlbPose& scratch) const;
    BatBarrelSample sample_barrel(double preview_time_s,std::uint64_t commit,engine::GlbPose& scratch) const;
    double plant_frame(double commit_frame) const;
    bool complete() const { return tick==end_tick; }
private:
    std::array<std::size_t,bone_count> nodes{};
    std::array<engine::GlbMatrix,bone_count> rest{};
    std::array<Bones,225> source{};
    DirectX::XMFLOAT3 placement;
    void sample_key(double tick,std::optional<std::uint64_t> commit,engine::GlbPose& scratch) const;
    Bones source_pose(double frame) const;
    Bones preparation(double frame) const;
    engine::GlbTrs front_foot(double frame) const;
    void semantics(Bones& bones) const;
};
}
