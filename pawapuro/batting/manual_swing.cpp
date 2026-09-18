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
    switch(geometry) {
    case ManualGeometry::NoSwing:return "NoSwing";
    case ManualGeometry::Contact:return "Contact";
    case ManualGeometry::NoContactInWindow:return "NoContactInWindow";
    default:return "Pending";
    }
}
void ManualSwingPreview::query_contact() {
    if(geometry!=ManualGeometry::Pending||!committed)return;
    const auto finish_without_contact=[&]{geometry=ManualGeometry::NoContactInWindow;contact_dispatch_tick=tick;
        std::fprintf(stderr,"ManualGeometry NoContactInWindow: commit=%llu dispatch_tick=%llu temporal=%s queries=%u; no ball response\n",committed->consumed_tick,tick,timing_state(),contact_query_count);};
    if(!timing->overlap){finish_without_contact();return;}
    const auto interval=*timing->overlap;
    double start=std::max(double(tick-1)/pitch_hz,interval.start_s),end=std::min(double(tick)/pitch_hz,interval.end_s);
    // S is zero at its endpoints. Move only those numerical endpoints inward, not a gameplay epsilon.
    if(start==timing->start_s)start=std::nextafter(start,end);
    if(end==timing->end_s)end=std::nextafter(end,start);
    if(start<end) {
        ++contact_query_count;
        if(!searched_interval)searched_interval=TemporalInterval{start,end};else searched_interval->end_s=end;
        contact=first_manual_contact(delivery.pitch.initial,double(delivery.motion.release_tick)/pitch_hz,
            batter,committed->consumed_tick,tuning.bat_contact,start,end,contact_scratch,64,committed->tempo);
        if(contact) {
            geometry=ManualGeometry::Contact;++contact_count;contact_dispatch_tick=tick;const auto& e=*contact;
            std::fprintf(stderr,"ManualGeometry Contact: commit=%llu time_s=%.12f dispatch_tick=%llu u=%.9f normal=(%.9g,%.9g,%.9g) relative_velocity=(%.9g,%.9g,%.9g) temporal_query=[%.12f,%.12f] tempo=%s; Geometry only / No ball response\n",
                committed->consumed_tick,e.sample.preview_time_s,tick,e.sample.approach.u,e.normal.x,e.normal.y,e.normal.z,
                e.relative_velocity.x,e.relative_velocity.y,e.relative_velocity.z,interval.start_s,interval.end_s,committed->tempo.mode==SwingTempo::Original?"A":"B");
            return;
        }
    }
    if(double(tick)/pitch_hz>=interval.end_s)finish_without_contact();
}
ManualSwingPreview::ManualSwingPreview(const std::filesystem::path& d,const BattingStaging& s)
    :tuning(s),delivery(d/"pitcher/pitcher.glb",d/"pitcher/pitcher.toml",s),batter(d/"batter/ingame_s0",s),
     ball_passage(batting_interaction_passage(delivery.pitch,double(delivery.motion.release_tick)/pitch_hz,s.batting_interaction)) {
    std::fprintf(stderr,"Manual Swing S0: source=%s recipe=gate-b-world-residual-v1 mode=Normal live-attempt input; Geometry only / No ball response; query=ball slab intersect swing potential; displayed ball continues through slab\n",batter.asset.sha256.c_str());
}
void ManualSwingPreview::reset() {
    delivery.reset();batter.evaluate_tick(0,std::nullopt);tick=pending_ticks=fractional_credit=arrival_tick=0;
    timing.reset();searched_interval.reset();contact_query_count=0;authorization.reset();contact.reset();contact_count=0;contact_dispatch_tick=0;geometry=ManualGeometry::Pending;
    pending.reset();committed.reset();paused=false;armed=false;input_result="Waiting";phase=PreviewPhase::Ready;
}
bool ManualSwingPreview::start(){if(phase==PreviewPhase::Playing)return false;reset();attempt_tempo={next_tempo,tuning.compact_area_ticks,tuning.normal_finish_ticks};delivery.start();phase=PreviewPhase::Playing;return true;}
bool ManualSwingPreview::toggle_tempo(){
    if(phase==PreviewPhase::Playing)return false;
    next_tempo=next_tempo==SwingTempo::Original?SwingTempo::Compact:SwingTempo::Original;return true;
}
void ManualSwingPreview::lose_input(){if(pending)input_result="Cancelled";pending.reset();armed=false;}
void ManualSwingPreview::toggle_pause(){if(phase==PreviewPhase::Playing){paused=!paused;lose_input();}}
bool ManualSwingPreview::single_step(){return phase==PreviewPhase::Playing&&paused?step_tick():false;}
bool ManualSwingPreview::swing_available() const {
    return armed&&phase==PreviewPhase::Playing&&!paused&&!pending&&!committed;
}
bool ManualSwingPreview::record_command(std::uint64_t target,DirectX::XMFLOAT2 aim) {
    if(phase!=PreviewPhase::Playing||paused){input_result="RejectedState";return false;}
    if(pending||committed){input_result="RejectedAlready";return false;}
    if(target<=tick||target>std::numeric_limits<std::uint64_t>::max()-456){input_result="RejectedTick";return false;}
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
    if(pending&&pending->target_tick==tick){committed=pending;pending.reset();committed->consumed_tick=tick;input_result="Committed";
        timing=timing_interaction(double(tick)/pitch_hz,ball_passage,tuning.swing_phase_potential);
        const auto point=predict_arrival(delivery.pitch).state.position_m;
        authorization=authorize_normal_hit(committed->aim_center,{point.x,point.y},tuning.hit_authorization);
        const auto& a=*authorization;
        std::fprintf(stderr,"HitAuthorization %s: consumed_tick=%llu pitch=(%.9g,%.9g) error=(%.9g,%.9g) normalized=(%.9g,%.9g) q=%.9g\n",
            a.authorized?"Authorized":"RejectedSpatial",tick,a.pitch_point.x,a.pitch_point.y,a.error.x,a.error.y,a.normalized_error.x,a.normalized_error.y,a.q);
        std::fprintf(stderr,"Swing consumed: target_tick=%llu consumed_tick=%llu boundary_tick=%llu recorded_backlog=%llu aim=(%.9g,%.9g) tempo=%s compact_area_ticks=%.9g mapping=smoothstep-v1+tail-quartic-v1 sweep_tick=%.9f area_tick=%.9f finish_tick=%llu Geometry pending\n",committed->target_tick,tick,committed->boundary_tick,committed->backlog,committed->aim_center.x,committed->aim_center.y,committed->tempo.mode==SwingTempo::Original?"A":"B",committed->tempo.compact_area_ticks,double(tick)+committed->tempo.world_offset(SwingTempoTiming::sweep_ticks),double(tick)+committed->tempo.world_offset(SwingTempoTiming::area_ticks),committed->tempo.end_tick(tick));}
    if(committed&&committed->consumed_tick==tick){char diagnostic[640];timing_diagnostic(diagnostic,sizeof(diagnostic));std::fprintf(stderr,"TimingInteraction commit=%llu %s\n",tick,diagnostic);}
    batter.evaluate_tick(tick,committed?std::optional(committed->consumed_tick):std::nullopt,committed?committed->tempo:attempt_tempo);
    query_contact();
    if(!arrival_tick&&delivery.pitch.phase==PitchPhase::Complete)arrival_tick=tick;
    if(delivery.phase==DeliveryPhase::Complete&&batter.complete()&&!pending) {if(!committed)geometry=ManualGeometry::NoSwing;phase=PreviewPhase::Complete;paused=false;pending.reset();pending_ticks=fractional_credit=0;return true;}
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
    if(!timing)return "Pending";
    return timing->state==SwingTimingState::Overlap?"Overlap":timing->state==SwingTimingState::Early?"NoOverlapEarly":"NoOverlapLate";
}
void ManualSwingPreview::timing_diagnostic(char* buffer,std::size_t size) const {
    if(!timing){std::snprintf(buffer,size,"Timing:Pending Spatial:Pending");return;}
    const auto& t=*timing;
    std::snprintf(buffer,size,"Timing:%s ball_s=[%.9f,%.9f] potential_s=[%.9f,%.9f,%.9f] overlap_s=[%.9f,%.9f] efficiency=%.6f offset_ms=%+.6f Spatial:%s",
        timing_state(),ball_passage.enter_s,ball_passage.exit_s,t.start_s,t.peak_s,t.end_s,
        t.overlap?t.overlap->start_s:0,t.overlap?t.overlap->end_s:0,t.efficiency,t.offset_ms,
        authorization?(authorization->authorized?"Authorized":"RejectedSpatial"):"Pending");
}
DirectX::XMFLOAT3 ManualSwingPreview::displayed_ball_center() const {
    const double time=double(tick)/pitch_hz;
    if(delivery.ball_owner==BallOwner::Simulation&&time>=ball_passage.enter_s)
        return sample_reference_pitch(delivery.pitch.initial,std::min(time,ball_passage.exit_s)-double(delivery.motion.release_tick)/pitch_hz).position_m;
    return delivery.ball_center();
}
DirectX::XMFLOAT3 ManualSwingPreview::displayed_ball_translation() const {
    const auto p=displayed_ball_center(),o=delivery.pitch.initial.position_m;return {p.x-o.x,p.y-o.y,p.z-o.z};
}
}
