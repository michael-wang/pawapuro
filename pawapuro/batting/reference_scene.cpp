#include "reference_scene.hpp"
#include <cmath>
#include <algorithm>

using namespace DirectX;
namespace pawapuro {
std::vector<engine::Vertex> make_batting_reference(const BattingStaging& staging)
{
    // Metres, +Y up, +Z from home plate toward the pitcher. These are Native fixtures.
    std::vector<engine::Vertex> vertices;
    const auto triangle = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 color) {
        vertices.insert(vertices.end(), {{a, color}, {b, color}, {c, color}});
    };
    const auto quad = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d, XMFLOAT3 color) {
        triangle(a, b, c, color);
        triangle(a, c, d, color);
    };
    const float extent = staging.grass_half_width_m;
    quad({-extent, 0, -12}, {-extent, 0, staging.grass_end_z_m},
        {extent, 0, staging.grass_end_z_m}, {extent, 0, -12}, {0.075f, 0.20f, 0.15f});
    // Broad, subtle grass bands communicate depth beyond the pitcher without walls.
    for (float z = 20; z < staging.grass_end_z_m; z += 20) {
        const float end = std::min(z + 10, staging.grass_end_z_m);
        quad({-extent, 0.001f, z}, {-extent, 0.001f, end}, {extent, 0.001f, end},
            {extent, 0.001f, z}, {0.08f, 0.215f, 0.16f});
    }
    // Two flat dirt patches supply staging context; only the mound below is raised.
    const auto dirt_disk = [&](float center_z, float radius) {
        for (int i = 0; i < 64; ++i) {
            const float a = XM_2PI * static_cast<float>(i) / 64;
            const float b = XM_2PI * static_cast<float>(i + 1) / 64;
            triangle({0, 0.003f, center_z},
                {radius * std::cos(a), 0.003f, center_z + radius * std::sin(a)},
                {radius * std::cos(b), 0.003f, center_z + radius * std::sin(b)}, {0.33f, 0.23f, 0.17f});
        }
    };
    dirt_disk(0, staging.home_dirt_radius_m);
    dirt_disk(mound_center_z_m, staging.mound_visual_dirt_radius_m);
    // Five-sided plate: point toward catcher, full-width edge toward pitcher.
    constexpr float half_width = 0.2159f;
    const XMFLOAT3 point{0, 0.012f, 0};
    const XMFLOAT3 left{-half_width, 0.012f, half_width};
    const XMFLOAT3 front_left{-half_width, 0.012f, 0.4318f};
    const XMFLOAT3 front_right{half_width, 0.012f, 0.4318f};
    const XMFLOAT3 right{half_width, 0.012f, half_width};
    const XMFLOAT3 white{0.96f, 0.95f, 0.87f};
    triangle(point, left, front_left, white);
    triangle(point, front_left, front_right, white);
    triangle(point, front_right, right, white);
    // A shallow mound with a flat top supplies height/stance context, not terrain.
    const auto mound_point = [](float angle, float radius, float y) -> XMFLOAT3 {
        return {radius * std::cos(angle), y, mound_center_z_m + radius * std::sin(angle)};
    };
    for (int i = 0; i < 48; ++i) {
        const float a = XM_2PI * static_cast<float>(i) / 48;
        const float b = XM_2PI * static_cast<float>(i + 1) / 48;
        quad(mound_point(a, staging.mound_radius_m, 0.008f), mound_point(b, staging.mound_radius_m, 0.008f),
            mound_point(b, staging.mound_top_radius_m, mound_height_m),
            mound_point(a, staging.mound_top_radius_m, mound_height_m), {0.46f, 0.33f, 0.22f});
        triangle({0, mound_height_m, mound_center_z_m},
            mound_point(a, staging.mound_top_radius_m, mound_height_m),
            mound_point(b, staging.mound_top_radius_m, mound_height_m), {0.54f, 0.40f, 0.27f});
    }
    constexpr float rubber_y = mound_height_m + 0.005f;
    quad({-0.3048f, rubber_y, rubber_distance_m}, {-0.3048f, rubber_y, rubber_distance_m + 0.1524f},
        {0.3048f, rubber_y, rubber_distance_m + 0.1524f}, {0.3048f, rubber_y, rubber_distance_m}, white);

    // The ruler is tied to the rubber centre, not a free staging offset.
    constexpr float ruler_z = rubber_distance_m + 0.1524f / 2;
    const XMFLOAT3 gold{0.90f, 0.69f, 0.25f};
    quad({-0.01f, rubber_y, ruler_z}, {-0.01f, rubber_y + 2, ruler_z},
        {0.01f, rubber_y + 2, ruler_z}, {0.01f, rubber_y, ruler_z}, gold);
    for (int step = 0; step <= 4; ++step) {
        const float y = rubber_y + static_cast<float>(step) * 0.5f;
        quad({-0.09f, y, ruler_z}, {-0.09f, y + 0.02f, ruler_z},
            {0.09f, y + 0.02f, ruler_z}, {0.09f, y, ruler_z}, gold);
    }

    const XMFLOAT3 release = staging.release_position_m;
    const float ring_inner = staging.ball_marker_radius_m + 0.15f;
    const float ring_outer = ring_inner + 0.03f;
    const float distance = std::hypot(release.x, release.z - mound_center_z_m);
    const float base_y = 0.008f + (mound_height_m - 0.008f) * std::clamp(
        (staging.mound_radius_m - distance) / (staging.mound_radius_m - staging.mound_top_radius_m), 0.0f, 1.0f);
    const XMFLOAT3 cyan{0.16f, 0.80f, 0.86f};
    // The post meets the mound surface and stops at the ring, not inside the ball.
    quad({release.x - 0.025f, base_y, release.z}, {release.x - 0.025f, release.y - ring_outer, release.z},
        {release.x + 0.025f, release.y - ring_outer, release.z}, {release.x + 0.025f, base_y, release.z}, cyan);
    for (int i = 0; i < 32; ++i) {
        const float a = XM_2PI * static_cast<float>(i) / 32;
        const float b = XM_2PI * static_cast<float>(i + 1) / 32;
        const auto ring_point = [&](float angle, float radius) -> XMFLOAT3 {
            return {release.x + radius * std::cos(angle), release.y + radius * std::sin(angle), release.z};
        };
        quad(ring_point(a, ring_inner), ring_point(b, ring_inner), ring_point(b, ring_outer), ring_point(a, ring_outer), cyan);
    }
    // A small faceted sphere, generated only for this one fixture, not a primitive API.
    const auto ball_point = [&](int latitude, int longitude) -> XMFLOAT3 {
        const float a = XM_PI * static_cast<float>(latitude) / 8;
        const float b = XM_2PI * static_cast<float>(longitude) / 16;
        return {release.x + staging.ball_marker_radius_m * std::sin(a) * std::cos(b),
            release.y + staging.ball_marker_radius_m * std::cos(a), release.z + staging.ball_marker_radius_m * std::sin(a) * std::sin(b)};
    };
    for (int lat = 0; lat < 8; ++lat) {
        const float shade = 0.65f + 0.35f * (1 - static_cast<float>(lat) / 8);
        for (int lon = 0; lon < 16; ++lon) {
            quad(ball_point(lat, lon), ball_point(lat + 1, lon), ball_point(lat + 1, lon + 1),
                ball_point(lat, lon + 1), {shade, shade, shade * 0.92f});
        }
    }
    return vertices;
}

XMFLOAT4X4 batting_view_projection(const BattingStaging& staging, float aspect)
{
    // Left-handed batter framing; camera orientation does not move the rubber or mound.
    const auto view = XMMatrixLookAtLH(XMLoadFloat3(&staging.camera_position_m),
        XMLoadFloat3(&staging.camera_target_m), XMVectorSet(0, 1, 0, 0));
    XMFLOAT4X4 result;
    XMStoreFloat4x4(&result, view * XMMatrixPerspectiveFovLH(XMConvertToRadians(staging.vertical_fov_degrees), aspect, 0.1f, 300));
    return result;
}
}
