#pragma once
#include "pitcher_motion.hpp"
#include "pawapuro/batting/reference_pitch.hpp"

namespace pawapuro {
enum class DeliveryPhase { Ready, Delivering, Complete };
enum class BallOwner { Hand, Simulation };
// Owns this one pitch's asset, evaluated geometry and baseball state for the
// app lifetime. Rendering borrows triangles only for the synchronous upload.
struct PitchDelivery {
    PitchDelivery(const std::filesystem::path& glb, const std::filesystem::path& metadata, const BattingStaging& staging);
    PitcherMotion motion;
    ReferencePitch pitch;
    DeliveryPhase phase=DeliveryPhase::Ready;
    BallOwner ball_owner=BallOwner::Hand;
    bool paused=false;
    std::uint64_t tick=0, pending_ticks=0;
    unsigned release_count=0;
    float release_alignment_error_m=0;
    void reset();
    bool start();
    void toggle_pause();
    bool single_step();
    bool advance(std::uint64_t elapsed_ns);
    DirectX::XMFLOAT3 ball_center() const;
    DirectX::XMFLOAT3 ball_translation() const;
    const char* state_name() const;
    const char* owner_name() const;
private:
    std::uint64_t fractional_credit=0;
    bool step_tick();
};
}
