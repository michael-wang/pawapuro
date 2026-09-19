#include "manual_swing.hpp"
#include "bat_contact_detail.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
namespace pawapuro {
ContactSample sample_manual_contact(const BallState& initial,double release,const IngameMotion& motion,
    std::uint64_t commit,double time,engine::GlbPose& scratch,SwingTempoTiming tempo) {
    const auto ball=sample_reference_pitch(initial,time-release);
    const auto bat=motion.sample_barrel(time,commit,scratch,tempo);
    return {time,ball,bat,closest_barrel_point(ball.position_m,bat)};
}
std::optional<BatContact> first_manual_contact(const BallState& initial,double release,const IngameMotion& motion,
    std::uint64_t commit,BatContactEnvelope envelope,double start,double end,engine::GlbPose& scratch,unsigned substeps,SwingTempoTiming tempo) {
    auto sample=[&](double t){return sample_manual_contact(initial,release,motion,commit,t,scratch,tempo);};
    auto bat_sample=[&](double t){return motion.sample_barrel(t,commit,scratch,tempo);};
    return contact_detail::first(sample,bat_sample,release,envelope,start,end,substeps);
}
const char* ManualSwingPreview::contact_state() const {
    switch(geometry()) {
    case ManualGeometry::NoSwing:return "NoSwing";
    case ManualGeometry::Contact:return "Contact";
    case ManualGeometry::NoContactInWindow:return "NoContactInWindow";
    default:return "Pending";
    }
}
void ManualSwingPreview::query_contact() {
    if(!active_attempt)return;
    auto& attempt=attempts[*active_attempt];
    if(attempt.geometry!=ManualGeometry::Pending)return;
    const auto finish_without_contact=[&]{attempt.geometry=ManualGeometry::NoContactInWindow;attempt.contact_dispatch_tick=tick;
        std::fprintf(stderr,"RawOverlap NoContactInWindow: commit=%llu dispatch_tick=%llu temporal=%s queries=%u; diagnostic only\n",attempt.command.consumed_tick,tick,timing_state(),attempt.contact_query_count);};
    if(!attempt.timing.overlap){finish_without_contact();return;}
    const auto interval=*attempt.timing.overlap;
    double start=std::max(double(tick-1)/pitch_hz,interval.start_s),end=std::min(double(tick)/pitch_hz,interval.end_s);
    // S is zero at its endpoints. Move only those numerical endpoints inward, not a gameplay epsilon.
    if(start==attempt.timing.start_s)start=std::nextafter(start,end);
    if(end==attempt.timing.end_s)end=std::nextafter(end,start);
    if(start<end) {
        ++attempt.contact_query_count;
        if(!attempt.searched_interval)attempt.searched_interval=TemporalInterval{start,end};else attempt.searched_interval->end_s=end;
        attempt.contact=first_manual_contact(delivery.pitch.initial,double(delivery.motion.release_tick)/pitch_hz,
            batter,attempt.command.consumed_tick,tuning.bat_contact,start,end,contact_scratch,64,attempt.command.tempo);
        if(attempt.contact) {
            attempt.geometry=ManualGeometry::Contact;++attempt.contact_count;attempt.contact_dispatch_tick=tick;const auto& e=*attempt.contact;
            std::fprintf(stderr,"RawOverlap Contact: commit=%llu time_s=%.12f dispatch_tick=%llu u=%.9f normal=(%.9g,%.9g,%.9g) relative_velocity=(%.9g,%.9g,%.9g) temporal_query=[%.12f,%.12f] tempo=%s; Raw physical overlap diagnostic\n",
                attempt.command.consumed_tick,e.sample.preview_time_s,tick,e.sample.approach.u,e.normal.x,e.normal.y,e.normal.z,
                e.relative_velocity.x,e.relative_velocity.y,e.relative_velocity.z,interval.start_s,interval.end_s,attempt.command.tempo.mode==SwingTempo::Original?"A":"B");
            return;
        }
    }
    if(double(tick)/pitch_hz>=interval.end_s)finish_without_contact();
}
const char* ManualSwingPreview::gameplay_state() const {
    switch(gameplay_result()) {
    case GameplayResult::NoSwing:return "NoSwing";
    case GameplayResult::Miss:return "Miss";
    case GameplayResult::Contact:return "Contact";
    default:return "Pending";
    }
}
void ManualSwingPreview::dispatch_gameplay_contact() {
    if(flight||!active_attempt)return;
    auto& a=attempts[*active_attempt];
    if(!a.response||double(tick)/pitch_hz<a.response->contact_time_s)return;
    a.gameplay=GameplayResult::Contact;a.gameplay_dispatch_tick=tick;
    flight.emplace(*a.response,tuning.gameplay_ball.radius_m);pending.reset();armed=false;
    const auto& r=*a.response;const auto& auth=a.authorization;
    std::fprintf(stderr,"GameplayContact attempt=%zu commit=%llu dispatch_tick=%llu efficiency=%.9f offset_ms=%+.9f ex=%.9g ey=%.9g q=%.9g spatial=%.9g energy=%.9g Power=%d Trajectory=%d effective_s=%.12f speed_mps=%.9g speed_kmh=%.9g longitudinal_deg=%.9g spray_deg=%.9g origin=(%.9g,%.9g,%.9g) ground_s=%.12f RawOverlap=%s (diagnostic only)\n",
        *active_attempt+1,a.command.consumed_tick,tick,a.timing.efficiency,a.timing.offset_ms,auth.normalized_error.x,auth.normalized_error.y,auth.q,
        r.spatial_transfer,r.energy_transfer,tuning.batter_profile.power,tuning.batter_profile.trajectory,r.contact_time_s,r.exit_speed_mps,r.exit_speed_mps*3.6f,r.longitudinal_angle_deg,r.spray_angle_deg,
        r.launch_position_m.x,r.launch_position_m.y,r.launch_position_m.z,flight->ground_s,contact_state());
}
ManualSwingPreview::ManualSwingPreview(const std::filesystem::path& d,const BattingStaging& s)
    :tuning(s),delivery(d/"pitcher/pitcher.glb",d/"pitcher/pitcher.toml",s),batter(d/"batter/ingame_s0",s),
     ball_passage(batting_interaction_passage(delivery.pitch,double(delivery.motion.release_tick)/pitch_hz,s.batting_interaction)) {
    std::fprintf(stderr,"Manual Swing S0: source=%s recipe=gate-b-world-residual-v1 mode=Normal live-attempt input; Raw physical overlap diagnostic; query=ball slab intersect swing potential; incoming display analytic before plane; unhit world ball hidden after arrival\n",batter.asset.sha256.c_str());
}
void ManualSwingPreview::reset() {
    delivery.reset();batter.evaluate_tick(0,std::nullopt);tick=pending_ticks=fractional_credit=arrival_tick=0;
    attempts.clear();active_attempt.reset();flight.reset();preparing=true;
    pending.reset();paused=false;armed=false;input_result="Waiting";phase=PreviewPhase::Ready;
}
bool ManualSwingPreview::start(){
    if(phase==PreviewPhase::Playing&&!can_restart_after_contact())return false;
    reset();attempt_tempo={next_tempo,tuning.compact_area_ticks,tuning.normal_finish_ticks};
    delivery.start();phase=PreviewPhase::Playing;return true;
}
bool ManualSwingPreview::toggle_tempo(){
    if(phase==PreviewPhase::Playing)return false;
    next_tempo=next_tempo==SwingTempo::Original?SwingTempo::Compact:SwingTempo::Original;return true;
}
void ManualSwingPreview::lose_input(){if(pending)input_result="Cancelled";pending.reset();armed=false;}
void ManualSwingPreview::toggle_pause(){if(phase==PreviewPhase::Playing){paused=!paused;lose_input();}}
bool ManualSwingPreview::single_step(){return step_ticks(1);}
bool ManualSwingPreview::step_ticks(unsigned count){
    if(phase!=PreviewPhase::Playing||!paused||count==0)return false;
    // Review steps own no wall-time debt; every intermediate tick stays authoritative.
    pending_ticks=fractional_credit=0;
    for(unsigned i=0;i<count;++i)if(step_tick())return true;
    return false;
}
bool ManualSwingPreview::swing_available() const {
    return armed&&phase==PreviewPhase::Playing&&!paused&&!pending&&!active_attempt&&intent_live(tick+pending_ticks+1);
}
bool ManualSwingPreview::record_command(std::uint64_t target,DirectX::XMFLOAT2 aim) {
    if(phase!=PreviewPhase::Playing||paused){input_result="RejectedState";return false;}
    if(pending||active_attempt){input_result="RejectedAlready";return false;}
    if(target<=tick||target>std::numeric_limits<std::uint64_t>::max()-456){input_result="RejectedTick";return false;}
    if(!intent_live(target)){input_result="RejectedPitchClosed";return false;}
    if(!std::isfinite(aim.x)||!std::isfinite(aim.y)){input_result="RejectedAim";return false;}
    pending=SwingCommand{target,0,tick,pending_ticks,aim,SwingMode::Normal,attempt_tempo};input_result="Queued";
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
    if(pending&&pending->target_tick==tick){
        auto command=*pending;pending.reset();command.consumed_tick=tick;
        const auto point=predict_arrival(delivery.pitch).state.position_m;
        attempts.push_back({command,timing_interaction(double(tick)/pitch_hz,ball_passage,tuning.swing_phase_potential),
            authorize_normal_hit(command.aim_center,{point.x,point.y},normal_authorization_region(tuning),tuning.gameplay_ball)});
        auto& attempt=attempts.back();
        attempt.response=ball_response(attempt.timing,attempt.authorization,tuning.batter_profile,tuning.ball_response,
            delivery.pitch.initial,double(delivery.motion.release_tick)/pitch_hz);
        if(!attempt.response)attempt.gameplay=GameplayResult::Miss;
        active_attempt=attempts.size()-1;preparing=false;input_result="Committed";
        char diagnostic[640];timing_diagnostic(diagnostic,sizeof(diagnostic));
        std::fprintf(stderr,"Swing consumed: attempt=%zu target_tick=%llu consumed_tick=%llu boundary_tick=%llu backlog=%llu aim=(%.9g,%.9g) tempo=%s finish_tick=%llu %s\n",
            attempts.size(),command.target_tick,tick,command.boundary_tick,command.backlog,command.aim_center.x,command.aim_center.y,
            command.tempo.mode==SwingTempo::Compact?"B":"A",command.tempo.end_tick(tick),diagnostic);
    }
    const auto* command=preparing?nullptr:committed();
    batter.evaluate_tick(tick,command?std::optional(command->consumed_tick):std::nullopt,command?command->tempo:attempt_tempo);
    query_contact();
    dispatch_gameplay_contact();
    if(active_attempt&&batter.complete()) {
        active_attempt.reset();armed=false; // No active-swing press/held key survives this boundary.
        if(intent_live(tick+1)) {
            preparing=true;batter.evaluate_tick(tick,std::nullopt,attempt_tempo);input_result="Rearmed";
            std::fprintf(stderr,"Swing rearmed: swings=%zu world_tick=%llu next_target=%llu hard-reset=current-world-preparation\n",attempts.size(),tick,tick+1);
        }
    }
    if(!arrival_tick&&delivery.pitch.phase==PitchPhase::Complete)arrival_tick=tick;
    if(delivery.phase==DeliveryPhase::Complete&&batter.complete()&&!pending&&(!flight||flight->complete(double(tick)/pitch_hz))) {phase=PreviewPhase::Complete;paused=false;pending_ticks=fractional_credit=0;return true;}
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
const char* ManualSwingPreview::timing_state() const {
    if(!timing())return "Pending";
    return timing()->state==SwingTimingState::Overlap?"Overlap":timing()->state==SwingTimingState::Early?"NoOverlapEarly":"NoOverlapLate";
}
void ManualSwingPreview::timing_diagnostic(char* buffer,std::size_t size) const {
    if(!timing()){std::snprintf(buffer,size,"Timing:Pending Spatial:Pending");return;}
    const auto& t=*timing();
    std::snprintf(buffer,size,"Timing:%s ball_s=[%.9f,%.9f] potential_s=[%.9f,%.9f,%.9f] overlap_s=[%.9f,%.9f] efficiency=%.6f offset_ms=%+.6f Spatial:%s",
        timing_state(),ball_passage.enter_s,ball_passage.exit_s,t.start_s,t.peak_s,t.end_s,
        t.overlap?t.overlap->start_s:0,t.overlap?t.overlap->end_s:0,t.efficiency,t.offset_ms,
        authorization()?(authorization()->authorized?"Authorized":"RejectedSpatial"):"Pending");
}
DirectX::XMFLOAT3 ManualSwingPreview::displayed_ball_center() const {
    const double time=double(tick)/pitch_hz;
    if(flight)return flight->sample(time).position_m;
    if(delivery.pitch.phase==PitchPhase::InFlight&&time>=ball_passage.enter_s)
        return sample_reference_pitch(delivery.pitch.initial,time-double(delivery.motion.release_tick)/pitch_hz).position_m;
    return delivery.ball_center();
}
DirectX::XMFLOAT3 ManualSwingPreview::displayed_ball_translation() const {
    const auto p=displayed_ball_center(),o=delivery.pitch.initial.position_m;return {p.x-o.x,p.y-o.y,p.z-o.z};
}
}
