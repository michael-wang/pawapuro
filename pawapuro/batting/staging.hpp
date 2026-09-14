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
    DirectX::XMFLOAT3 camera_target_m{0, 1.30f, 16.8f};
    float vertical_fov_degrees = 36;
    DirectX::XMFLOAT3 release_position_m{-0.65f, 2.05f, 16.8f};
    float ball_marker_radius_m = 0.10f;
    float grass_half_width_m = 85;
    float grass_end_z_m = 140;
    float home_dirt_radius_m = 2.8f;
    float mound_visual_dirt_radius_m = 5.5f;
    float mound_radius_m = 2.75f;
    float mound_top_radius_m = 0.9f;
};
BattingStaging load_batting_staging(const std::filesystem::path& path);
}
