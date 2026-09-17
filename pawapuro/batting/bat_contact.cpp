#include "bat_contact_detail.hpp"
namespace pawapuro {
std::optional<BatContact> first_bat_contact(const BallState& initial,double release,const BatterMotion& batter,
    BatContactEnvelope envelope,double start,double end,unsigned substeps,double phase_offset)
{
    if (!std::isfinite(phase_offset)) throw std::runtime_error("Invalid contact phase offset.");
    engine::GlbPose scratch;
    auto sample=[&](double t){return sample_contact(initial,release,batter,t,scratch,phase_offset);};
    auto bat_sample=[&](double t){return batter.sample_barrel(t+phase_offset,scratch);};
    return contact_detail::first(sample,bat_sample,release,envelope,start,end,substeps);
}
}
