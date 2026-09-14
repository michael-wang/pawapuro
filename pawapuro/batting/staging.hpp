#pragma once
#include <DirectXMath.h>
#include <filesystem>

namespace pawapuro {
// Spatial references, not camera tuning: 60 ft 6 in and 10 in, in metres.
inline constexpr float rubber_distance_m = 18.4404f;
inline constexpr float mound_height_m = 0.254f;
inline constexpr float mound_center_z_m = rubber_distance_m - 0.4572f;

// Startup-only snapshot. Missing keys use these defaults; invalid files are rejected.
struct BattingStaging {
    DirectX::XMFLOAT3 camera_position_m{-0.75f, 1.25f, -5.0f};
    DirectX::XMFLOAT3 camera_target_m{2.260052f, 1.30f, 16.8f};
    float vertical_fov_degrees = 36;
    DirectX::XMFLOAT3 release_position_m{-0.65f, 2.05f, 16.8f};
    DirectX::XMFLOAT3 reference_velocity_mps{1.655f, -0.6f, -41.667f};
    // Single authoritative gameplay zone; rendering consumes these same rule values.
    float strike_zone_width_m = 0.8636f;
    float strike_zone_bottom_m = 0.5f, strike_zone_top_m = 1.45f;
    float ball_marker_radius_m = 0.10f;
    float grass_half_width_m = 85;
    float grass_end_z_m = 140;
    float home_dirt_radius_m = 2.8f;
    float mound_visual_dirt_radius_m = 5.5f;
    float mound_radius_m = 2.75f;
    float mound_top_radius_m = 0.9f;
    // Preserve the original pentagon proportions: depth equals width, tip stays at Z=0.
    float home_plate_depth_m() const { return strike_zone_width_m; }
    float strike_zone_plane_z() const { return home_plate_depth_m() / 2; }
};
BattingStaging load_batting_staging(const std::filesystem::path& path);
}
