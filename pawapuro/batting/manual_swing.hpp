#pragma once
#include "batting_preview.hpp" // Baseline remains an independent contact regression caller.
#include "batter/ingame_s0/ingame_motion.hpp"
#include "hit_authorization.hpp"
#include "timing_interaction.hpp"
#include "ball_response.hpp"
namespace pawapuro {
// Historical offline regression bound only; runtime searches the temporal intersection.
inline constexpr std::uint64_t manual_contact_window_ticks=64;
ContactSample sample_manual_contact(const BallState& initial,double release,const IngameMotion& motion,
    std::uint64_t commit,double time,engine::GlbPose& scratch,SwingTempoTiming tempo={});
std::optional<BatContact> first_manual_contact(const BallState& initial,double release,const IngameMotion& motion,
    std::uint64_t commit,BatContactEnvelope envelope,double start,double end,engine::GlbPose& scratch,unsigned substeps=64,SwingTempoTiming tempo={});
enum class ManualGeometry { Pending, NoSwing, Contact, NoContactInWindow };
enum class GameplayResult { Pending, NoSwing, Miss, Contact };
enum class SwingMode { Normal };
struct SwingCommand {
    std::uint64_t target_tick=0,consumed_tick=0,boundary_tick=0,backlog=0;
    DirectX::XMFLOAT2 aim_center{};
    SwingMode mode=SwingMode::Normal;
    SwingTempoTiming tempo;
    // S0 has one fixed Normal mode. Replay also requires motion asset SHA/recipe.
};
// One consumed intent owns its decisions and raw event for the whole pitch lifetime.
struct SwingAttempt {
    SwingCommand command;
    TimingInteraction timing;
    HitAuthorizationDecision authorization;
    ManualGeometry geometry=ManualGeometry::Pending;
    // Planned once on consumption; Contact is dispatched only when the world clock reaches its time.
    std::optional<BallResponse> response;
    GameplayResult gameplay=GameplayResult::Pending;
    std::uint64_t gameplay_dispatch_tick=0;
    std::optional<BatContact> contact;
    std::optional<TemporalInterval> searched_interval;
    unsigned contact_query_count=0,contact_count=0;
    std::uint64_t contact_dispatch_tick=0;
};
struct ManualSwingPreview {
    ManualSwingPreview(const std::filesystem::path& directory,const BattingStaging& staging);
    const BattingStaging tuning; // Immutable startup Data retained with this attempt owner for replay.
    PitchDelivery delivery;
    IngameMotion batter;
    PreviewPhase phase=PreviewPhase::Ready;
    bool paused=false;
    SwingTempo next_tempo=SwingTempo::Compact;
    SwingTempoTiming attempt_tempo;
    bool toggle_tempo();
    std::uint64_t tick=0,pending_ticks=0,arrival_tick=0;
    std::optional<SwingCommand> pending;
    std::vector<SwingAttempt> attempts;
    std::optional<std::size_t> active_attempt;
    const BattingInteractionPassage ball_passage;
    std::optional<BattedBallFlight> flight;
    // Flight exists only after Gameplay Contact dispatch, never for a planned response.
    bool can_restart_after_contact() const { return flight.has_value(); }
    GameplayResult gameplay_result() const { return latest()?latest()->gameplay:phase==PreviewPhase::Complete?GameplayResult::NoSwing:GameplayResult::Pending; }
    const char* gameplay_state() const;
    std::uint64_t completion_tick() const {
        const auto batter_end=committed()?committed()->tempo.end_tick(committed()->consumed_tick):batter.end_tick;
        const auto ground=flight?static_cast<std::uint64_t>(std::ceil(flight->ground_s*pitch_hz)):0;
        return std::max({delivery.motion.end_tick,batter_end,ground});
    }
    // Read-only latest-attempt views; no second copy of per-swing truth.
    const SwingAttempt* latest() const { return attempts.empty()?nullptr:&attempts.back(); }
    const SwingCommand* committed() const { return latest()?&latest()->command:nullptr; }
    const TimingInteraction* timing() const { return latest()?&latest()->timing:nullptr; }
    const HitAuthorizationDecision* authorization() const { return latest()?&latest()->authorization:nullptr; }
    const std::optional<BatContact>& contact() const { static const std::optional<BatContact> empty; return latest()?latest()->contact:empty; }
    ManualGeometry geometry() const { return latest()?latest()->geometry:phase==PreviewPhase::Complete?ManualGeometry::NoSwing:ManualGeometry::Pending; }
    unsigned contact_count() const { return latest()?latest()->contact_count:0; }
    unsigned contact_query_count() const { return latest()?latest()->contact_query_count:0; }
    std::uint64_t contact_dispatch_tick() const { return latest()?latest()->contact_dispatch_tick:0; }
    const std::optional<TemporalInterval>& searched_interval() const { static const std::optional<TemporalInterval> empty; return latest()?latest()->searched_interval:empty; }
    bool intent_live(std::uint64_t target) const { return phase==PreviewPhase::Playing&&!flight&&double(target)/pitch_hz<ball_passage.exit_s; }
    bool rearmed() const { return !attempts.empty()&&preparing&&!active_attempt&&!pending&&!paused&&intent_live(tick+pending_ticks+1); }
    const char* contact_state() const;
    const char* timing_state() const;
    void timing_diagnostic(char* buffer,std::size_t size) const;
    DirectX::XMFLOAT3 displayed_ball_center() const;
    DirectX::XMFLOAT3 displayed_ball_translation() const;
    void reset();
    bool start();
    void toggle_pause();
    bool single_step();
    bool step_ticks(unsigned count);
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
    bool preparing=true;
    const char* input_result="Waiting";
    engine::GlbPose contact_scratch;
    void query_contact();
    void dispatch_gameplay_contact();
    bool step_tick();
};
}
