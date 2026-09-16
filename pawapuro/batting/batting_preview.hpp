#pragma once
#include "pitcher/pitch_delivery.hpp"
#include "batter/batter_motion.hpp"

namespace pawapuro {
enum class PreviewPhase { Ready, Playing, Complete };
// One concrete development choreography. No gameplay swing input/commit semantics.
// Owns both actors and the sole preview accumulator; children receive fixed ticks.
struct BattingPreview {
    BattingPreview(const std::filesystem::path& batting_directory,const BattingStaging& staging);
    PitchDelivery delivery;
    BatterMotion batter;
    PreviewPhase phase=PreviewPhase::Ready;
    bool paused=false;
    std::uint64_t tick=0,pending_ticks=0,arrival_tick=0;
    void reset();
    bool start();
    void toggle_pause();
    bool single_step();
    bool advance(std::uint64_t elapsed_ns);
    const char* state_name() const;
private:
    std::uint64_t fractional_credit=0;
    bool step_tick();
};
}
