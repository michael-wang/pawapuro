#pragma once
#include "engine/rendering/mesh_glb.hpp"
#include "pawapuro/batting/staging.hpp"

namespace pawapuro {
inline constexpr std::uint64_t animation_hz=240;
enum class MotionPhase { Ready, Playing, Complete };
// One concrete development delivery preview; ball ownership remains outside it.
struct PitcherMotion {
    PitcherMotion(const std::filesystem::path& glb, const std::filesystem::path& metadata, const BattingStaging& staging);
    const engine::MeshGlb asset;
    engine::GlbPose pose;
    std::vector<engine::Vertex> skinned, triangles;
    engine::GlbMatrix grip_world{};
    MotionPhase phase=MotionPhase::Ready;
    bool paused=false;
    std::uint64_t tick=0, pending_ticks=0, end_tick=0, release_tick=0;
    double pose_us=0, skin_us=0;
    std::uint64_t samples=0;
    bool start();
    void reset();
    void toggle_pause();
    bool single_step();
    bool step_tick(); // Concrete clock owner advances one tick; does not consume wall time.
    bool advance(std::uint64_t elapsed_ns);
    double time_s() const;
    const char* state_name() const;
    void evaluate(); // Deterministic current-tick diagnostics and geometry; no event dispatch.
private:
    DirectX::XMFLOAT3 placement;
    std::size_t grip=0;
    std::uint64_t fractional_credit=0;
};
}
