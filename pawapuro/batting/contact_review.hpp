#pragma once
#include "manual_swing.hpp"
#include "player_aim.hpp"
#include "reference_scene.hpp"

namespace pawapuro {
inline constexpr std::size_t contact_review_vertex_count=PlayerAim::vertex_count;
inline bool batting_practice_pitch_marker_visible(const ManualSwingPreview& preview) {
    // One practice marker: reveal at release, retain after arrival until pitch reset.
    return preview.delivery.pitch.phase!=PitchPhase::Ready;
}
inline PitchMarkerVisualState batting_practice_pitch_marker_state(const ManualSwingPreview& preview) {
    return pitch_marker_visual_state(preview.delivery.pitch.phase);
}
inline bool batting_practice_world_ball_visible(const ManualSwingPreview& preview) {
    return preview.flight.has_value()||preview.delivery.pitch.phase!=PitchPhase::Complete;
}
// The retained arrival cue is the review reference; the reticle remains next-pitch intent.
inline void append_contact_review(std::vector<engine::Vertex>& vertices,const PlayerAim& aim) {
    aim.append_triangles(vertices);
}
}
