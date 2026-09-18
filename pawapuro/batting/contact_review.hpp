#pragma once
#include "manual_swing.hpp"
#include "player_aim.hpp"
#include "reference_scene.hpp"

namespace pawapuro {
inline constexpr std::size_t contact_review_vertex_count=PlayerAim::vertex_count;
inline bool contact_review_arrival_visible(const ManualSwingPreview& preview) {
    return arrival_cue_visible(preview.delivery.pitch.phase)||preview.gameplay_result()==GameplayResult::Contact;
}
// Presentation borrows the existing attempt only for this append; no second snapshot or gameplay owner.
inline void append_contact_review(std::vector<engine::Vertex>& vertices,const PlayerAim& aim,const ManualSwingPreview& preview) {
    const auto* contact=preview.gameplay_result()==GameplayResult::Contact?preview.latest():nullptr;
    aim.append_triangles_at(vertices,contact?contact->command.aim_center:aim.center());
}
}
