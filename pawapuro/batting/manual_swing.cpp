#include "manual_swing.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
namespace pawapuro {
ManualSwingPreview::ManualSwingPreview(const std::filesystem::path& d,const BattingStaging& s)
    :tuning(s),delivery(d/"pitcher/pitcher.glb",d/"pitcher/pitcher.toml",s),batter(d/"batter/ingame_s0",s) {
    std::fprintf(stderr,"Manual Swing S0: source=%s recipe=gate-b-world-residual-v1 mode=Normal domain=[432,496]; Contact: NotEvaluated; ball freezes after arrival\n",batter.asset.sha256.c_str());
}
void ManualSwingPreview::reset() {
    delivery.reset();batter.evaluate_tick(0,std::nullopt);tick=pending_ticks=fractional_credit=arrival_tick=0;
    pending.reset();committed.reset();paused=false;armed=false;input_result="Waiting";phase=PreviewPhase::Ready;
}
bool ManualSwingPreview::start(){if(phase==PreviewPhase::Playing)return false;reset();delivery.start();phase=PreviewPhase::Playing;return true;}
void ManualSwingPreview::lose_input(){if(pending)input_result="Cancelled";pending.reset();armed=false;}
void ManualSwingPreview::toggle_pause(){if(phase==PreviewPhase::Playing){paused=!paused;lose_input();}}
bool ManualSwingPreview::single_step(){return phase==PreviewPhase::Playing&&paused?step_tick():false;}
bool ManualSwingPreview::swing_available() const {
    const auto target=tick+pending_ticks+1;
    return armed&&phase==PreviewPhase::Playing&&!paused&&!pending&&!committed&&target>=432&&target<=496;
}
bool ManualSwingPreview::record_command(std::uint64_t target,DirectX::XMFLOAT2 aim) {
    if(phase!=PreviewPhase::Playing||paused){input_result="RejectedState";return false;}
    if(pending||committed){input_result="RejectedAlready";return false;}
    if(target<=tick||target<432||target>496){input_result="RejectedDomain";return false;}
    if(!std::isfinite(aim.x)||!std::isfinite(aim.y)){input_result="RejectedAim";return false;}
    pending=SwingCommand{target,0,tick,pending_ticks,aim};input_result="Queued";
    std::fprintf(stderr,"Swing queued: boundary_tick=%llu backlog=%llu target_tick=%llu aim=(%.9g,%.9g) mode=Normal\n",tick,pending_ticks,target,aim.x,aim.y);return true;
}
void ManualSwingPreview::input_boundary(bool held,bool edge,bool eligible,DirectX::XMFLOAT2 aim) {
    if(!eligible||phase!=PreviewPhase::Playing||paused){lose_input();if(edge)input_result="RejectedState";return;}
    if(!held&&!edge)armed=true;
    if(!edge)return;
    if(!armed){input_result="RejectedHeld";return;}
    armed=false;const auto target=tick+pending_ticks+1;
    if(!record_command(target,aim))std::fprintf(stderr,"Swing rejected: reason=%s boundary_tick=%llu backlog=%llu target_tick=%llu\n",input_result,tick,pending_ticks,target);
}
bool ManualSwingPreview::step_tick() {
    ++tick;delivery.step_tick();
    if(pending&&pending->target_tick==tick){committed=pending;pending.reset();committed->consumed_tick=tick;input_result="Committed";
        std::fprintf(stderr,"Swing consumed: target_tick=%llu consumed_tick=%llu boundary_tick=%llu recorded_backlog=%llu aim=(%.9g,%.9g) sweep_tick=%llu area_tick=%llu finish_tick=%llu Contact: NotEvaluated\n",committed->target_tick,tick,committed->boundary_tick,committed->backlog,committed->aim_center.x,committed->aim_center.y,tick+24,tick+40,tick+456);}
    batter.evaluate_tick(tick,committed?std::optional(committed->consumed_tick):std::nullopt);
    if(!arrival_tick&&delivery.pitch.phase==PitchPhase::Complete)arrival_tick=tick;
    if(delivery.phase==DeliveryPhase::Complete&&batter.complete()) {phase=PreviewPhase::Complete;paused=false;pending.reset();pending_ticks=fractional_credit=0;return true;}
    return false;
}
bool ManualSwingPreview::advance(std::uint64_t elapsed_ns) {
    if(phase!=PreviewPhase::Playing||paused)return false;
    constexpr std::uint64_t ns=1'000'000'000;
    fractional_credit+=(elapsed_ns%ns)*pitch_hz;
    const auto added=(elapsed_ns/ns)*pitch_hz+fractional_credit/ns;fractional_credit%=ns;
    pending_ticks+=std::min(added,std::numeric_limits<std::uint64_t>::max()-pending_ticks);
    for(unsigned n=0;n<max_pitch_ticks_per_frame&&pending_ticks;++n){--pending_ticks;if(step_tick())return true;}return false;
}
const char* ManualSwingPreview::state_name() const {if(phase==PreviewPhase::Ready)return "Ready";if(phase==PreviewPhase::Complete)return "Complete";return paused?"Paused":"Playing";}
const char* ManualSwingPreview::swing_state() const {return input_result;}
}
