#pragma once
#include <DirectXMath.h>
#include <filesystem>

namespace pawapuro {
// Longitudinal field reference in metres; mound height is presentation Data.
inline constexpr float rubber_distance_m = 18.4404f;
inline constexpr float mound_center_z_m = rubber_distance_m - 0.4572f;

// Startup-only snapshot. Missing keys use these defaults; invalid files are rejected.
struct BatContactEnvelope { float ball_radius_m=0.037f,bat_radius_m=0.033f; };
struct HitAuthorizationTuning { float normal_radius_x_m=0.26f, normal_radius_y_m=0.13f; };
struct PlayerAimTuning { float cursor_speed_mps=0.65f; };
struct BattingInteractionTuning { float half_depth_m=0.40f; };
struct SwingPhasePotentialTuning { float normal_start_ms=75,normal_peak_ms=125,normal_end_ms=190; };
struct BattingStaging {
    double window_width_fraction=0.75;
    double compact_area_ticks=30;
    unsigned normal_finish_ticks=240;
    PlayerAimTuning player_aim;
    HitAuthorizationTuning hit_authorization;
    BattingInteractionTuning batting_interaction;
    SwingPhasePotentialTuning swing_phase_potential;
    BatContactEnvelope bat_contact; // Gameplay candidate; independent of visual geometry.
    DirectX::XMFLOAT3 camera_position_m{-0.75f, 1.25f, -5.0f};
    DirectX::XMFLOAT3 camera_target_m{-0.75f, 1.30f, 16.8f};
    float vertical_fov_degrees = 36;
    DirectX::XMFLOAT3 release_position_m{-0.65f, 2.05f, 16.8f};
    DirectX::XMFLOAT3 reference_velocity_mps{1.655f, -0.6f, -41.667f};
    // Single authoritative gameplay zone; rendering consumes these same rule values.
    float strike_zone_width_m = 0.8636f;
    float strike_zone_bottom_m = 0.5f, strike_zone_top_m = 1.45f;
    float ball_marker_radius_m = 0.085f;
    float grass_half_width_m = 85;
    float grass_end_z_m = 140;
    float home_dirt_radius_m = 2.8f;
    float mound_visual_dirt_radius_m = 5.5f;
    float mound_height_m = 0.254f;
    float mound_radius_m = 2.75f;
    float mound_top_radius_m = 0.9f;
    DirectX::XMFLOAT3 pitcher_blockout_position_m{0, 0.259f, 18.5166f};
    float pitcher_blockout_height_m = 1.85f;
    DirectX::XMFLOAT3 batter_blockout_position_m{1.25f, 0.008f, 0};
    float batter_blockout_height_m = 1.7f;
    // Shared presentation multipliers for the two static Character Style v1 fixtures.
    float blockout_head_scale = 1.08f, blockout_foot_planar_scale = 1.9f, blockout_foot_height_scale = 0.9f;
    float blockout_hand_scale = 1.3f, blockout_bat_thickness_scale = 1.25f;
    // Preserve the original pentagon proportions: depth equals width, tip stays at Z=0.
    float home_plate_depth_m() const { return strike_zone_width_m; }
    float strike_zone_plane_z() const { return home_plate_depth_m() / 2; }
};
BattingStaging load_batting_staging(const std::filesystem::path& path);
}
