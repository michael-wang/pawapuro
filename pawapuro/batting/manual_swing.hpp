#pragma once
#include "batting_preview.hpp" // Baseline remains an independent contact regression caller.
#include "batter/ingame_s0/ingame_motion.hpp"
namespace pawapuro {
enum class SwingMode { Normal };
struct SwingCommand {
    std::uint64_t target_tick=0,consumed_tick=0,boundary_tick=0,backlog=0;
    DirectX::XMFLOAT2 aim_center{};
    SwingMode mode=SwingMode::Normal;
    // S0 has one fixed Normal mode. Replay also requires motion asset SHA/recipe.
};
struct ManualSwingPreview {
    ManualSwingPreview(const std::filesystem::path& directory,const BattingStaging& staging);
    const BattingStaging tuning; // Immutable startup Data retained with this attempt owner for replay.
    PitchDelivery delivery;
    IngameMotion batter;
    PreviewPhase phase=PreviewPhase::Ready;
    bool paused=false;
    std::uint64_t tick=0,pending_ticks=0,arrival_tick=0;
    std::optional<SwingCommand> pending,committed;
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
    bool step_tick();
};
}
