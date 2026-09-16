#pragma once
#include "engine/rendering/mesh_glb.hpp"
#include "pawapuro/batting/staging.hpp"

namespace pawapuro {
struct BatBarrelSample {
    DirectX::XMFLOAT3 barrel,tip;
    engine::GlbMatrix barrel_world{}; // Same game basis; anchors a capsule surface material point.
};
// Owns one accepted swing asset and reusable CPU pose/geometry. No wall clock.
struct BatterMotion {
    BatterMotion(const std::filesystem::path& glb,const std::filesystem::path& metadata,const BattingStaging& staging);
    const engine::MeshGlb asset;
    engine::GlbPose pose;
    std::array<std::vector<engine::Vertex>,2> skinned;
    std::vector<engine::Vertex> triangles;
    engine::GlbMatrix grip_world{},barrel_world{},tip_world{};
    std::uint64_t tick=0,end_tick=0,contact_area_tick=0,gather_tick=0,plant_tick=0;
    double pose_us=0,body_skin_us=0,bat_skin_us=0;
    std::uint64_t samples=0;
    void evaluate_tick(std::uint64_t authoritative_tick);
    // Caller-owned scratch: does not skin or modify authoritative pose, tick or triangles.
    BatBarrelSample sample_barrel(double clip_time_s,engine::GlbPose& scratch) const;
    bool complete() const { return tick==end_tick; }
private:
    DirectX::XMFLOAT3 placement;
    std::size_t grip=0,barrel=0,tip=0,body_primitive=0;
};
}
