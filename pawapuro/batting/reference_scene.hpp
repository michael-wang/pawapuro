#pragma once
#include "reference_pitch.hpp"
#include "engine/rendering/d3d12_view.hpp"
#include <vector>
#include <optional>
#include "staging.hpp"

namespace pawapuro {
inline constexpr unsigned ball_readability_vertex_count = 32 * 6 * 2;
// App-owned toggle; hidden states append degenerate triangles to retain fixed capacity.
void append_ball_readability(std::vector<engine::Vertex>& vertices, const BattingStaging& staging,
    DirectX::XMFLOAT3 center, PitchPhase phase, bool enabled, unsigned width, unsigned height);
enum class PitchMarkerVisualState { Hidden, Current, Previous };
inline PitchMarkerVisualState pitch_marker_visual_state(PitchPhase phase) {
    return phase==PitchPhase::Ready?PitchMarkerVisualState::Hidden:phase==PitchPhase::InFlight?PitchMarkerVisualState::Current:PitchMarkerVisualState::Previous;
}
enum class ArrivalCueStyle { Baseball, Ring };
inline constexpr unsigned arrival_cue_vertex_count = 48*6 + 48*3 + 2*(12+5)*6;
inline bool arrival_cue_visible(PitchPhase phase) { return phase == PitchPhase::InFlight; }
struct BattingReference {
    std::vector<engine::Vertex> arrival_baseball, arrival_ring;
    std::vector<engine::Vertex> previous_baseball, previous_ring;
    std::vector<engine::Vertex> vertices;
    unsigned ball_vertex_start = 0;
    unsigned overlay_vertex_start = 0;
    DirectX::XMFLOAT2 zone_min_ndc{}, zone_max_ndc{}, prediction_ndc{};
};
void append_arrival_cue(std::vector<engine::Vertex>& vertices, const BattingReference& scene,
    PitchMarkerVisualState state, ArrivalCueStyle style);
inline void append_arrival_cue(std::vector<engine::Vertex>& vertices, const BattingReference& scene,
    bool visible, ArrivalCueStyle style) {
    append_arrival_cue(vertices,scene,visible?PitchMarkerVisualState::Current:PitchMarkerVisualState::Hidden,style);
}
inline void append_arrival_cue(std::vector<engine::Vertex>& vertices, const BattingReference& scene,
    PitchPhase phase, ArrivalCueStyle style) {
    append_arrival_cue(vertices,scene,arrival_cue_visible(phase),style);
}
BattingReference make_batting_reference(const BattingStaging& staging,
    DirectX::XMFLOAT3 predicted_position, float aspect, std::span<const engine::Vertex> pitcher_vertices, std::span<const engine::Vertex> batter_vertices = {});
DirectX::XMFLOAT2 project_batting_point(const BattingStaging& staging, DirectX::XMFLOAT3 point, float aspect);
// Optional overlay only; invalid camera setup still fails through batting_view_projection.
std::optional<DirectX::XMFLOAT2> try_project_batting_point(const BattingStaging& staging, DirectX::XMFLOAT3 point, float aspect);
DirectX::XMFLOAT4X4 batting_view_projection(const BattingStaging& staging, float aspect);
}
