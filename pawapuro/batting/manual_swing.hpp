#pragma once
#include "batting_preview.hpp" // Baseline remains an independent contact regression caller.
#include "batter/ingame_s0/ingame_motion.hpp"
namespace pawapuro {
// Fixed-pitch diagnostic: commit through commit+64 ticks, including analytic flight past the plane.
inline constexpr std::uint64_t manual_contact_window_ticks=64;
ContactSample sample_manual_contact(const BallState& initial,double release,const IngameMotion& motion,
    std::uint64_t commit,double time,engine::GlbPose& scratch,SwingTempoTiming tempo={});
std::optional<BatContact> first_manual_contact(const BallState& initial,double release,const IngameMotion& motion,
    std::uint64_t commit,BatContactEnvelope envelope,double start,double end,engine::GlbPose& scratch,unsigned substeps=64,SwingTempoTiming tempo={});
enum class ManualGeometry { Pending, NoSwing, Contact, NoContactInWindow };
enum class SwingMode { Normal };
struct SwingCommand {
    std::uint64_t target_tick=0,consumed_tick=0,boundary_tick=0,backlog=0;
    DirectX::XMFLOAT2 aim_center{};
    SwingMode mode=SwingMode::Normal;
    SwingTempoTiming tempo;
    // S0 has one fixed Normal mode. Replay also requires motion asset SHA/recipe.
};
struct ManualSwingPreview {
    ManualSwingPreview(const std::filesystem::path& directory,const BattingStaging& staging);
    const BattingStaging tuning; // Immutable startup Data retained with this attempt owner for replay.
    PitchDelivery delivery;
    IngameMotion batter;
    PreviewPhase phase=PreviewPhase::Ready;
    bool paused=false;
    SwingTempo next_tempo=SwingTempo::Original;
    SwingTempoTiming attempt_tempo;
    bool domain_rejected=false;
    bool toggle_tempo();
    std::uint64_t tick=0,pending_ticks=0,arrival_tick=0;
    std::optional<SwingCommand> pending,committed;
    ManualGeometry geometry=ManualGeometry::Pending;
    std::optional<BatContact> contact;
    unsigned contact_count=0;
    std::uint64_t contact_dispatch_tick=0;
    const char* contact_state() const;
    void reset();
    bool start();
    void toggle_pause();
    bool single_step();
    bool advance(std::uint64_t elapsed_ns);
    void lose_input();
    void input_boundary(bool held,bool edge,bool eligible,DirectX::XMFLOAT2 aim);
    // Authoritative replay uses recorded target/snapshot, never raw OS event timestamps.
    bool record_command(std::uint64_t target,DirectX::XMFLOAT2 aim);
    const char* state_name() const;
    const char* swing_state() const;
    bool swing_available() const;
private:
    std::uint64_t fractional_credit=0;
    bool armed=false;
    const char* input_result="Waiting";
    engine::GlbPose contact_scratch;
    void query_contact();
    bool step_tick();
};
}
