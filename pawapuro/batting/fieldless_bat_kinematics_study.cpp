// Study-only caller, linked into ball_response_test, never the production app.
#include "review_fixture.hpp"
#include "bat_contact_detail.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <type_traits>

namespace {
using namespace pawapuro;
using namespace DirectX;
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
constexpr double missing=std::numeric_limits<double>::quiet_NaN();
double length(XMFLOAT3 v){return std::sqrt(double(v.x)*v.x+double(v.y)*v.y+double(v.z)*v.z);}
XMFLOAT3 subtract(XMFLOAT3 a,XMFLOAT3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
double dot(XMFLOAT3 a,XMFLOAT3 b){return double(a.x)*b.x+double(a.y)*b.y+double(a.z)*b.z;}
XMFLOAT3 unit(XMFLOAT3 v){const auto d=length(v);return d>0?XMFLOAT3{float(v.x/d),float(v.y/d),float(v.z/d)}:XMFLOAT3{};}
// Coordinate azimuth only. Primary deflection below is relative to incoming motion.
double azimuth(XMFLOAT3 v){return std::hypot(v.x,v.z)>1e-8?std::atan2(double(v.x),double(v.z))*180/3.141592653589793:missing;}
void vector(std::ostream& s,XMFLOAT3 v){s<<','<<v.x<<','<<v.y<<','<<v.z;}

// Ephemeral before/after byte snapshots of these same live objects, not a replay format.
template<class T> void bytes(std::string& out,const T& value){
    static_assert(std::is_trivially_copyable_v<T>);
    out.append(reinterpret_cast<const char*>(&value),sizeof(value));
}
std::string state(const ManualSwingPreview& p){
    std::string out;
    bytes(out,p.tick);bytes(out,p.pending_ticks);bytes(out,p.phase);bytes(out,p.paused);
    bytes(out,p.arrival_tick);bytes(out,p.active_attempt);bytes(out,p.pending);
    bytes(out,p.attempts.size());for(const auto& a:p.attempts)bytes(out,a);
    bytes(out,p.delivery.tick);bytes(out,p.delivery.pitch);bytes(out,p.flight);
    bytes(out,p.gameplay_result());bytes(out,p.batter.tick);bytes(out,p.batter.samples);
    bytes(out,p.batter.grip_world);bytes(out,p.batter.barrel_world);bytes(out,p.batter.tip_world);
    for(const auto& v:p.batter.pose.local)bytes(out,v);
    for(const auto& v:p.batter.pose.world)bytes(out,v);
    for(const auto& v:p.batter.pose.skin)bytes(out,v);
    for(const auto& v:p.batter.triangles)bytes(out,v);
    return out;
}
std::string row(const ManualSwingPreview& p){
    const auto& a=*p.latest();const auto& r=*a.response;
    const auto commit=a.command.consumed_tick;const double commit_s=double(commit)/pitch_hz,time=r.contact_time_s;
    engine::GlbPose scratch;
    const auto sample=sample_manual_contact(p.delivery.pitch.initial,double(p.delivery.motion.release_tick)/pitch_hz,
        p.batter,commit,time,scratch,a.command.tempo);
    const auto axis=unit(subtract(sample.bat.tip,sample.bat.barrel));
    const auto point=sample.approach.closest;
    // Same epsilon as bat_contact_detail; fixed closest CENTERLINE material point.
    // Raw contact evidence below instead uses its solver's surface material point.
    constexpr double epsilon=.0001;
    const auto local=XMVector3TransformCoord(XMLoadFloat3(&point),XMMatrixInverse(nullptr,contact_detail::matrix(sample.bat.barrel_world)));
    const auto before=p.batter.sample_barrel(time-epsilon,commit,scratch,a.command.tempo);
    const auto after=p.batter.sample_barrel(time+epsilon,commit,scratch,a.command.tempo);
    XMFLOAT3 velocity;
    XMStoreFloat3(&velocity,XMVectorScale(XMVectorSubtract(
        XMVector3TransformCoord(local,contact_detail::matrix(after.barrel_world)),
        XMVector3TransformCoord(local,contact_detail::matrix(before.barrel_world))),float(1/(2*epsilon))));
    const auto delta=subtract(sample.ball.position_m,point);
    const bool normal_valid=length(delta)>1e-8;
    const auto normal=unit(delta),relative=subtract(sample.ball.velocity_mps,velocity);
    const double closing=normal_valid?-dot(relative,normal):missing;
    const bool reflection_valid=normal_valid&&closing>0;
    XMFLOAT3 candidate{float(missing),float(missing),float(missing)};
    if(reflection_valid)candidate={float(sample.ball.velocity_mps.x+2*closing*normal.x),
        float(sample.ball.velocity_mps.y+2*closing*normal.y),float(sample.ball.velocity_mps.z+2*closing*normal.z)};
    const double horizontal=reflection_valid?std::hypot(candidate.x,candidate.z):missing;
    const double incoming_horizontal=std::hypot(sample.ball.velocity_mps.x,sample.ball.velocity_mps.z);
    const bool direction_valid=reflection_valid&&horizontal>1e-8&&incoming_horizontal>1e-8;
    // Positive turns from straight return toward its right in the XZ coordinate drawing.
    // atan2(ref.z*out.x-ref.x*out.z, ref.x*out.x+ref.z*out.z); no field input.
    const double rx=-sample.ball.velocity_mps.x/incoming_horizontal,rz=-sample.ball.velocity_mps.z/incoming_horizontal;
    const double deflection=direction_valid?std::atan2(rz*candidate.x-rx*candidate.z,rx*candidate.x+rz*candidate.z)*180/3.141592653589793:missing;
    std::ostringstream s;s<<std::setprecision(17);
    s<<commit<<','<<commit_s<<','<<a.timing.offset_ms<<','<<a.timing.efficiency<<','<<time<<','<<(time-commit_s)*1000;
    vector(s,sample.ball.position_m);vector(s,sample.ball.velocity_mps);vector(s,sample.bat.barrel);vector(s,sample.bat.tip);
    for(const auto value:sample.bat.barrel_world)s<<','<<value;
    vector(s,axis);s<<','<<azimuth(axis)<<','<<sample.approach.u;vector(s,point);
    const double envelope=double(p.tuning.bat_contact.ball_radius_m)+p.tuning.bat_contact.bat_radius_m;
    s<<','<<sample.approach.distance_m<<','<<envelope<<','<<(sample.approach.distance_m<=envelope);
    vector(s,velocity);s<<','<<length(velocity)<<','<<azimuth(velocity)<<','<<normal_valid;
    vector(s,normal_valid?normal:XMFLOAT3{float(missing),float(missing),float(missing)});s<<','<<(normal_valid?azimuth(normal):missing);
    vector(s,relative);s<<','<<length(relative)<<','<<closing<<','<<reflection_valid;
    vector(s,candidate);s<<','<<horizontal<<','<<direction_valid<<','<<deflection<<','<<rx<<','<<rz;
    s<<','<<bool(a.contact)<<','<<(a.contact?a.contact->sample.preview_time_s:missing);
    const XMFLOAT3 absent{float(missing),float(missing),float(missing)};
    vector(s,a.contact?a.contact->normal:absent);vector(s,a.contact?a.contact->bat_point_velocity:absent);vector(s,a.contact?a.contact->relative_velocity:absent);
    // Observational production columns, never inputs to the mechanical probe.
    s<<','<<a.authorization.authorized<<','<<a.authorization.q<<','<<r.temporal_transfer<<','<<r.energy_transfer
        <<','<<r.exit_speed_mps<<','<<r.spray_angle_deg<<','<<r.longitudinal_angle_deg<<','<<p.flight->ground_s<<','<<p.flight->stop_s;
    vector(s,p.flight->sample(p.flight->ground_s).position_m);vector(s,p.flight->sample(p.flight->stop_s).position_m);
    return s.str()+'\n';
}
// S1: independent ordered orientation and fixed-u velocities; no contact geometry.
std::string direction_basis_row(const ManualSwingPreview& p){
    const auto& a=*p.latest();const auto& r=*a.response;const auto commit=a.command.consumed_tick;
    const double time=r.contact_time_s,commit_s=double(commit)/pitch_hz;
    const auto ball=sample_reference_pitch(p.delivery.pitch.initial,time-double(p.delivery.motion.release_tick)/pitch_hz);
    const double incoming=std::hypot(ball.velocity_mps.x,ball.velocity_mps.z);
    const double rx=incoming>1e-8?-ball.velocity_mps.x/incoming:missing,rz=incoming>1e-8?-ball.velocity_mps.z/incoming:missing;
    const auto deflection=[&](XMFLOAT3 v){return incoming>1e-8&&std::hypot(v.x,v.z)>1e-8
        ?std::atan2(rz*v.x-rx*v.z,rx*v.x+rz*v.z)*180/3.141592653589793:missing;};
    const auto angle_difference=[](double left,double right){return std::remainder(left-right,360.);};
    engine::GlbPose scratch;
    const auto bat=p.batter.sample_barrel(time,commit,scratch,a.command.tempo);
    const auto axis=unit(subtract(bat.tip,bat.barrel));
    const XMFLOAT3 raw{axis.z,0,-axis.x}; // cross(world_up, ordered barrel -> tip). Never flip.
    const double magnitude=std::hypot(raw.x,raw.z);
    const bool valid=magnitude>1e-8;
    const XMFLOAT3 direction=valid?XMFLOAT3{float(raw.x/magnitude),0,float(raw.z/magnitude)}:XMFLOAT3{float(missing),0,float(missing)};
    constexpr double epsilon=.0001;
    const auto before=p.batter.sample_barrel(time-epsilon,commit,scratch,a.command.tempo);
    const auto after=p.batter.sample_barrel(time+epsilon,commit,scratch,a.command.tempo);
    std::ostringstream s;s<<std::setprecision(17);
    s<<commit<<','<<commit_s<<','<<a.timing.offset_ms<<','<<time<<','<<(time-commit_s)*1000;
    vector(s,ball.velocity_mps);s<<','<<rx<<','<<rz;vector(s,axis);s<<','<<azimuth(axis);
    vector(s,raw);vector(s,direction);s<<','<<valid<<','<<azimuth(direction)<<','<<deflection(direction);
    std::array<double,3> angles{};bool all_valid=true;unsigned index=0;
    for(const double u:{0.,.5,1.}){
        // Same authored material coordinate at both times, independent of the ball.
        const auto point=[&](const BatBarrelSample& b){return XMFLOAT3{
            float(std::lerp(double(b.barrel.x),double(b.tip.x),u)),float(std::lerp(double(b.barrel.y),double(b.tip.y),u)),
            float(std::lerp(double(b.barrel.z),double(b.tip.z),u))};};
        const auto lo=point(before),hi=point(after);
        const XMFLOAT3 velocity{float((double(hi.x)-lo.x)/(2*epsilon)),float((double(hi.y)-lo.y)/(2*epsilon)),float((double(hi.z)-lo.z)/(2*epsilon))};
        const double horizontal=std::hypot(velocity.x,velocity.z),angle=deflection(velocity);
        const bool point_valid=horizontal>1e-8&&incoming>1e-8;all_valid&=point_valid;angles[index++]=angle;
        vector(s,velocity);s<<','<<length(velocity)<<','<<horizontal<<','<<azimuth(velocity)<<','<<angle<<','<<point_valid;
    }
    const auto sign=[](double v){return (v>0)-(v<0);};
    double spread=missing;
    if(all_valid)spread=std::max({std::abs(angle_difference(angles[0],angles[1])),std::abs(angle_difference(angles[0],angles[2])),std::abs(angle_difference(angles[1],angles[2]))});
    s<<','<<spread<<','<<(all_valid?int(sign(angles[0])==sign(angles[1])&&sign(angles[1])==sign(angles[2])):-1)
        <<','<<!all_valid<<','<<angle_difference(deflection(direction),angles[1])<<','<<r.spray_angle_deg<<'\n';
    return s.str();
}
struct StudyRows {std::string reflection,basis;bool operator==(const StudyRows&) const = default;};
StudyRows sweep(const std::filesystem::path& directory,const BattingStaging& tuning,std::uint64_t cadence){
    ManualSwingPreview p(directory,tuning);PlayerAim aim(tuning);p.start();
    std::vector<std::uint64_t> commits;
    // Current live input supports every positive tick before passage exit, not the old authoring 432..496 bounds.
    for(std::uint64_t tick=1;p.intent_live(tick);++tick)
        if(timing_interaction(double(tick)/pitch_hz,p.ball_passage,tuning.swing_phase_potential).overlap)commits.push_back(tick);
    // Baseline regression assertion only AFTER deriving the domain from production.
    check(commits.size()==32&&commits.front()==431&&commits.back()==462,"S1 domain changed; stop interpretation");
    p.reset();StudyRows result;
    for(const auto commit:commits){
        const BattingReviewFixture fixture{commit,0,0};check(fixture.start(p,aim),"study fixture start");
        while(!p.latest()||p.latest()->gameplay==GameplayResult::Pending||p.geometry()==ManualGeometry::Pending)p.advance(cadence);
        fixture.verify(*p.latest());
        check(p.latest()->timing.overlap&&p.authorization()->authorized&&p.latest()->response&&p.flight
            &&p.gameplay_result()==GameplayResult::Contact,"study temporal/spatial contact missing");
        check(p.committed()->tempo.mode==SwingTempo::Compact,"study did not use production fixture tempo");
        const auto saved=state(p);const auto measured=row(p);
        check(saved==state(p),"study mutated authoritative preview/attempt/pitch/pose/flight");
        check(measured==row(p)&&saved==state(p),"repeated study sampling changed results/state");
        const auto basis=direction_basis_row(p);
        check(saved==state(p)&&basis==direction_basis_row(p)&&saved==state(p),"S1 sampling changed state/results");
        result.reflection+=measured;result.basis+=basis;
    }
    check(!result.reflection.empty(),"empty kinematics sweep");return result;
}
}
void fieldless_bat_kinematics_study(const std::filesystem::path& directory,const pawapuro::BattingStaging& tuning,const std::filesystem::path& output){
    const auto first=sweep(directory,tuning,33'333'333),replay=sweep(directory,tuning,8'333'333);
    check(first==replay,"fieldless sweep replay/cadence changed CSV");
    std::ostringstream csv;
    csv<<"commit_tick,commit_time_s,offset_ms,efficiency,contact_time_s,local_phase_ms,ball_x,ball_y,ball_z,ball_vx,ball_vy,ball_vz,barrel_x,barrel_y,barrel_z,tip_x,tip_y,tip_z";
    for(unsigned i=0;i<16;++i)csv<<",barrel_world_"<<i;
    csv<<",axis_x,axis_y,axis_z,axis_azimuth_deg,u,closest_x,closest_y,closest_z,separation_m,envelope_m,inside_envelope,bat_vx,bat_vy,bat_vz,bat_speed,bat_velocity_azimuth_deg,normal_valid,nx,ny,nz,normal_azimuth_deg,relative_vx,relative_vy,relative_vz,relative_speed,closing,reflection_valid,candidate_vx,candidate_vy,candidate_vz,candidate_horizontal_speed,direction_valid,fieldless_deflection_deg,return_ref_x,return_ref_z,raw_contact_exists,raw_time_s,raw_nx,raw_ny,raw_nz,raw_bat_vx,raw_bat_vy,raw_bat_vz,raw_relative_vx,raw_relative_vy,raw_relative_vz,authorized,q,temporal_transfer,energy_transfer,production_speed,production_spray_deg,production_longitudinal_deg,ground_s,stop_s,ground_x,ground_y,ground_z,stop_x,stop_y,stop_z\n"<<first.reflection;
    std::ofstream file(output/"fieldless-kinematics.csv");file<<csv.str();check(bool(file),"study CSV write failed");
    std::ofstream basis(output/"direction-basis.csv");
    basis<<"commit_tick,commit_time_s,offset_ms,contact_time_s,local_phase_ms,ball_vx,ball_vy,ball_vz,return_ref_x,return_ref_z,axis_x,axis_y,axis_z,axis_azimuth_deg,axis_raw_x,axis_raw_y,axis_raw_z,axis_direction_x,axis_direction_y,axis_direction_z,axis_valid,axis_direction_azimuth_deg,axis_deflection_deg";
    for(const auto name:{"u0","u05","u1"})for(const auto column:{"vx","vy","vz","speed","horizontal_speed","azimuth_deg","deflection_deg","valid"})basis<<','<<name<<'_'<<column;
    basis<<",point_spread_deg,point_sign_agreement,any_point_invalid,axis_minus_mid_deg,production_spray_deg\n"<<first.basis;
    check(bool(basis),"S1 CSV write failed");
    std::cout<<"PASS fieldless study: dense production sweep, byte-identical replay CSV, immutable sampling\n";
}
