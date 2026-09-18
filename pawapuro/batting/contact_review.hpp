#pragma once
#include "manual_swing.hpp"
#include "player_aim.hpp"
#include "reference_scene.hpp"

namespace pawapuro {
inline constexpr std::size_t contact_review_vertex_count=PlayerAim::vertex_count;
inline bool contact_review_arrival_visible(const ManualSwingPreview& preview) {
    return arrival_cue_visible(preview.delivery.pitch.phase)||preview.gameplay_result()==GameplayResult::Contact;
}
// The retained arrival cue is the review reference; the reticle remains next-pitch intent.
inline void append_contact_review(std::vector<engine::Vertex>& vertices,const PlayerAim& aim) {
    aim.append_triangles(vertices);
}
}
