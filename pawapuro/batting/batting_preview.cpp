#include "batting_preview.hpp"
#include <algorithm>
#include <cstdio>
#include <limits>

namespace pawapuro {
BattingPreview::BattingPreview(const std::filesystem::path& directory,const BattingStaging& staging)
    : delivery(directory/"pitcher/pitcher.glb",directory/"pitcher/pitcher.toml",staging),
      batter(directory/"batter/batter.glb",directory/"batter/batter.toml",staging)
{
    std::fprintf(stderr,"BattingPreview: one 240 Hz clock, authored choreography only; release=%llu gather=%llu plant=%llu contact_area=%llu pitcher_end=%llu batter_end=%llu\n",
        delivery.motion.release_tick,batter.gather_tick,batter.plant_tick,batter.contact_area_tick,delivery.motion.end_tick,batter.end_tick);
}
void BattingPreview::reset()
{
    delivery.reset();batter.evaluate_tick(0);tick=pending_ticks=fractional_credit=arrival_tick=0;
    paused=false;phase=PreviewPhase::Ready;
}
bool BattingPreview::start()
{
    if (phase==PreviewPhase::Playing) return false;
    reset();delivery.start();phase=PreviewPhase::Playing;return true;
}
void BattingPreview::toggle_pause() { if (phase==PreviewPhase::Playing) paused=!paused; }
bool BattingPreview::single_step() { return phase==PreviewPhase::Playing && paused?step_tick():false; }
bool BattingPreview::step_tick()
{
    ++tick;
    if (delivery.step_tick()) std::fprintf(stderr,"PitchDelivery Complete: preview_tick=%llu delivery_tick=%llu pitcher_tick=%llu\n",tick,delivery.tick,delivery.motion.tick);
    // Its own Complete remains final; never consume child wall time.
    if (!batter.complete()) batter.evaluate_tick(tick);
    if (!arrival_tick && delivery.pitch.phase==PitchPhase::Complete) {
        arrival_tick=tick;
        const auto delta=static_cast<std::int64_t>(batter.contact_area_tick)-static_cast<std::int64_t>(arrival_tick);
        std::fprintf(stderr,"Preview arrival: tick=%llu contact_area_delta_ticks=%lld delta_s=%.9f; diagnostic only\n",
            arrival_tick,delta,static_cast<double>(delta)/pitch_hz);
    }
    // Sample actual authoritative positions around the authoring marker. This
    // does not evaluate collision, snap the ball, or substitute the plane sample.
    if (tick+4>=batter.contact_area_tick && tick<=batter.contact_area_tick+4) {
        const auto p=delivery.ball_center();const auto& g=batter.grip_world;const auto& b=batter.barrel_world;const auto& t=batter.tip_world;
        std::fprintf(stderr,"Preview vicinity: tick=%llu ball=(%.9g,%.9g,%.9g) grip=(%.9g,%.9g,%.9g) barrel=(%.9g,%.9g,%.9g) tip=(%.9g,%.9g,%.9g) contact_area=%s; no hit/miss evaluation\n",
            tick,p.x,p.y,p.z,g[12],g[13],g[14],b[12],b[13],b[14],t[12],t[13],t[14],tick==batter.contact_area_tick?"yes":"no");
    }
    if (delivery.phase==DeliveryPhase::Complete && batter.complete()) {
        phase=PreviewPhase::Complete;paused=false;pending_ticks=fractional_credit=0;return true;
    }
    return false;
}
bool BattingPreview::advance(std::uint64_t elapsed_ns)
{
    if (phase!=PreviewPhase::Playing || paused) return false;
    constexpr std::uint64_t ns=1'000'000'000;
    fractional_credit+=(elapsed_ns%ns)*pitch_hz;
    const auto added=(elapsed_ns/ns)*pitch_hz+fractional_credit/ns;fractional_credit%=ns;
    pending_ticks+=std::min(added,std::numeric_limits<std::uint64_t>::max()-pending_ticks);
    for (unsigned n=0;n<max_pitch_ticks_per_frame && pending_ticks;++n) {
        --pending_ticks;if (step_tick()) return true;
    }
    return false;
}
const char* BattingPreview::state_name() const
{
    if (phase==PreviewPhase::Ready) return "Ready";
    if (phase==PreviewPhase::Complete) return "Complete";
    return paused?"Paused":"Playing";
}
}
