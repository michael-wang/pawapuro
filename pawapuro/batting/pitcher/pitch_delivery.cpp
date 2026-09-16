#include "pitch_delivery.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>

namespace pawapuro {
static_assert(animation_hz==pitch_hz);
PitchDelivery::PitchDelivery(const std::filesystem::path& glb, const std::filesystem::path& metadata, const BattingStaging& staging)
    : motion(glb,metadata,staging), pitch(staging.release_position_m,staging.reference_velocity_mps,staging.strike_zone_plane_z())
{
    if (!motion.release_tick || motion.release_tick>motion.end_tick)
        throw std::runtime_error("PitchDelivery requires a release marker after Ready and within the clip");
}
void PitchDelivery::reset()
{
    motion.reset(); pitch.reset(); tick=pending_ticks=fractional_credit=0;
    release_count=0; release_alignment_error_m=0; ball_owner=BallOwner::Hand;
    phase=DeliveryPhase::Ready; paused=false;
}
bool PitchDelivery::start()
{
    if (phase==DeliveryPhase::Delivering) return false;
    reset(); motion.start(); phase=DeliveryPhase::Delivering; return true;
}
void PitchDelivery::toggle_pause() { if (phase==DeliveryPhase::Delivering) paused=!paused; }
bool PitchDelivery::single_step() { return phase==DeliveryPhase::Delivering && paused ? step_tick() : false; }
bool PitchDelivery::step_tick()
{
    const bool already_flying=ball_owner==BallOwner::Simulation;
    ++tick;
    motion.step_tick(); // Evaluate the new tick before sampling its grip.
    if (!already_flying && motion.tick==motion.release_tick) {
        const auto grip=ball_center(), origin=pitch.initial.position_m;
        const float dx=grip.x-origin.x,dy=grip.y-origin.y,dz=grip.z-origin.z;
        release_alignment_error_m=std::sqrt(dx*dx+dy*dy+dz*dz);
        if (!(release_alignment_error_m<0.0001f) || release_count || !pitch.release())
            throw std::runtime_error("PitchDelivery release alignment/one-shot invariant failed");
        ++release_count; ball_owner=BallOwner::Simulation;
        const auto v=pitch.initial.velocity_mps;
        std::fprintf(stderr,"Release: delivery_tick=%llu animation_tick=%llu grip=(%.9g,%.9g,%.9g) "
            "owner=Hand->Simulation pitch_tick=0 initial=(%.9g,%.9g,%.9g) velocity=(%.9g,%.9g,%.9g) error_m=%.9g\n",
            tick,motion.tick,grip.x,grip.y,grip.z,origin.x,origin.y,origin.z,v.x,v.y,v.z,release_alignment_error_m);
    } else if (already_flying) {
        pitch.step_tick(); // Release tick itself never integrates physics.
    }
    // Both accepted recovery and baseball evaluation must finish; neither
    // child advances after its own completion. One clock remains authoritative.
    if (motion.phase==MotionPhase::Complete && pitch.phase==PitchPhase::Complete) {
        phase=DeliveryPhase::Complete; paused=false; pending_ticks=fractional_credit=0; return true;
    }
    return false;
}
bool PitchDelivery::advance(std::uint64_t elapsed_ns)
{
    if (phase!=DeliveryPhase::Delivering || paused) return false;
    constexpr std::uint64_t ns=1'000'000'000;
    fractional_credit+=(elapsed_ns%ns)*pitch_hz;
    const auto added=(elapsed_ns/ns)*pitch_hz+fractional_credit/ns;
    fractional_credit%=ns;
    pending_ticks+=std::min(added,std::numeric_limits<std::uint64_t>::max()-pending_ticks);
    for (unsigned n=0;n<max_pitch_ticks_per_frame && pending_ticks;++n) {
        --pending_ticks; if (step_tick()) return true;
    }
    return false;
}
DirectX::XMFLOAT3 PitchDelivery::ball_center() const
{
    if (ball_owner==BallOwner::Simulation) return pitch.current.position_m;
    return {motion.grip_world[12],motion.grip_world[13],motion.grip_world[14]};
}
DirectX::XMFLOAT3 PitchDelivery::ball_translation() const
{
    const auto p=ball_center(),o=pitch.initial.position_m;return {p.x-o.x,p.y-o.y,p.z-o.z};
}
const char* PitchDelivery::state_name() const
{
    if (phase==DeliveryPhase::Ready) return "Ready";
    if (phase==DeliveryPhase::Complete) return "Complete";
    if (paused) return "Paused";
    return pitch.phase==PitchPhase::Complete ? "Recovering" : "Delivering";
}
const char* PitchDelivery::owner_name() const { return ball_owner==BallOwner::Hand ? "Hand" : "Simulation"; }
}
