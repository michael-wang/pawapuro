#include "reference_scene.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>

using namespace DirectX;
namespace pawapuro {
BattingReference make_batting_reference(const BattingStaging& staging, XMFLOAT3 predicted_position, float aspect)
{
    // Metres, +Y up, +Z from home plate toward the pitcher. These are Native fixtures.
    BattingReference scene;
    auto& vertices = scene.vertices;
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
    const float half_width = staging.strike_zone_width_m / 2;
    const float plate_depth = staging.home_plate_depth_m();
    const XMFLOAT3 point{0, 0.012f, 0};
    const XMFLOAT3 left{-half_width, 0.012f, half_width};
    const XMFLOAT3 front_left{-half_width, 0.012f, plate_depth};
    const XMFLOAT3 front_right{half_width, 0.012f, plate_depth};
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
    // Two static fixtures share these small geometry operations, not a character/pose system.
    const auto ellipsoid = [&](XMFLOAT3 centre, XMFLOAT3 radius, XMFLOAT3 color) {
        const auto point_at = [&](int lat, int lon) -> XMFLOAT3 {
            const float a = XM_PI * static_cast<float>(lat) / 8;
            const float b = XM_2PI * static_cast<float>(lon) / 16;
            return {centre.x + radius.x * std::sin(a) * std::cos(b),
                centre.y + radius.y * std::cos(a), centre.z + radius.z * std::sin(a) * std::sin(b)};
        };
        for (int lat = 0; lat < 8; ++lat) {
            const float shade = 0.72f + 0.28f * (1 - static_cast<float>(lat) / 8);
            for (int lon = 0; lon < 16; ++lon)
                quad(point_at(lat, lon), point_at(lat + 1, lon), point_at(lat + 1, lon + 1),
                    point_at(lat, lon + 1), {color.x * shade, color.y * shade, color.z * shade});
        }
    };
    const auto segment = [&](XMFLOAT3 a, XMFLOAT3 b, float start_radius, float end_radius, XMFLOAT3 color) {
        const auto axis = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&b), XMLoadFloat3(&a)));
        const auto helper = std::abs(XMVectorGetY(axis)) < 0.9f ? XMVectorSet(0, 1, 0, 0) : XMVectorSet(1, 0, 0, 0);
        const auto side = XMVector3Normalize(XMVector3Cross(axis, helper));
        const auto up = XMVector3Cross(axis, side);
        const auto rim = [&](XMFLOAT3 centre, float radius, int i) {
            const float angle = XM_2PI * static_cast<float>(i) / 12;
            XMFLOAT3 result;
            XMStoreFloat3(&result, XMVectorAdd(XMLoadFloat3(&centre),
                XMVectorScale(XMVectorAdd(XMVectorScale(side, std::cos(angle)), XMVectorScale(up, std::sin(angle))), radius)));
            return result;
        };
        for (int i = 0; i < 12; ++i) {
            quad(rim(a, start_radius, i), rim(b, end_radius, i), rim(b, end_radius, i + 1), rim(a, start_radius, i + 1), color);
            triangle(a, rim(a, start_radius, i + 1), rim(a, start_radius, i), color);
            triangle(b, rim(b, end_radius, i), rim(b, end_radius, i + 1), color);
        }
    };
    for (bool batter : {false, true}) {
        const auto origin = batter ? staging.batter_blockout_position_m : staging.pitcher_blockout_position_m;
        const float h = batter ? staging.batter_blockout_height_m : staging.pitcher_blockout_height_m;
        const XMFLOAT3 shirt = batter ? XMFLOAT3{0.65f, 0.36f, 0.73f} : XMFLOAT3{0.22f, 0.48f, 0.78f};
        const XMFLOAT3 skin{0.97f, 0.74f, 0.51f}, shoes{0.16f, 0.19f, 0.24f}, pants{0.76f, 0.80f, 0.86f};
        // Pitcher faces -Z; batter torso faces the plate (-X), feet spread along Z.
        const auto at = [&](float x, float y, float z) -> XMFLOAT3 {
            return {origin.x + (batter ? z : x) * h, origin.y + y * h, origin.z + (batter ? -x : z) * h};
        };
        const auto part = [&](XMFLOAT3 centre, XMFLOAT3 radii, XMFLOAT3 color) {
            ellipsoid(centre, {radii.x * h, radii.y * h, radii.z * h}, color);
        };
        part(at(0, 0.47f, 0), {0.18f, 0.19f, 0.14f}, shirt);
        part(at(0, 0.77f, 0), {0.23f, 0.22f, 0.21f}, skin);
        part(at(0, 0.96f, 0), {0.24f, 0.04f, 0.22f}, shirt);
        for (float side : {-1.0f, 1.0f}) {
            segment(at(side * 0.10f, 0.34f, 0), at(side * 0.15f, 0.10f, -0.02f), h * 0.065f, h * 0.06f, pants);
            part(at(side * 0.15f, 0.045f, -0.045f), {0.085f, 0.045f, 0.12f}, shoes);
        }
        if (!batter) {
            for (float side : {-1.0f, 1.0f}) {
                part(at(side * 0.075f, 0.80f, -0.202f), {0.025f, 0.032f, 0.015f}, shoes);
                segment(at(side * 0.16f, 0.57f, 0), at(side * 0.22f, 0.43f, -0.10f), h * 0.055f, h * 0.05f, shirt);
                segment(at(side * 0.22f, 0.43f, -0.10f), at(side * 0.035f, 0.57f, -0.21f), h * 0.05f, h * 0.045f, skin);
            }
            // Left glove (+X from the catcher); the right throwing hand rests beside it.
            part(at(0.035f, 0.57f, -0.23f), {0.09f, 0.10f, 0.07f}, {0.55f, 0.30f, 0.13f});
        } else {
            const XMFLOAT3 grip{origin.x - 0.18f * h, origin.y + 0.62f * h, origin.z - 0.12f * h};
            const XMFLOAT3 tip{origin.x + 0.12f * h, origin.y + 1.18f * h, origin.z - 0.34f * h};
            // Ready bat stays on the catcher side; no swing path or contact model.
            segment(grip, tip, h * 0.018f, h * 0.035f, {0.94f, 0.68f, 0.27f});
            for (float side : {-1.0f, 1.0f}) {
                const XMFLOAT3 hand{grip.x + (side + 1) * 0.015f * h,
                    grip.y + (side + 1) * 0.028f * h, grip.z - (side + 1) * 0.011f * h};
                segment(at(side * 0.16f, 0.56f, 0), at(side * 0.20f, 0.44f, -0.16f), h * 0.055f, h * 0.05f, shirt);
                segment(at(side * 0.20f, 0.44f, -0.16f), hand, h * 0.045f, h * 0.04f, skin);
                part(hand, {0.045f, 0.045f, 0.045f}, skin);
            }
            // Head looks toward the pitcher (+Z) while the torso remains side-on.
            part({origin.x - 0.08f * h, origin.y + 0.78f * h, origin.z + 0.21f * h},
                {0.05f, 0.05f, 0.045f}, skin);
        }
    }
    scene.ball_vertex_start = static_cast<unsigned>(vertices.size());
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
    // Project the one gameplay zone, then flatten its bounds into an axis-aligned overlay.
    scene.overlay_vertex_start = static_cast<unsigned>(vertices.size());
    scene.zone_min_ndc = {1e9f, 1e9f};
    scene.zone_max_ndc = {-1e9f, -1e9f};
    for (float x : {-half_width, half_width}) {
        for (float y : {staging.strike_zone_bottom_m, staging.strike_zone_top_m}) {
            const auto p = project_batting_point(staging, {x, y, staging.strike_zone_plane_z()}, aspect);
            scene.zone_min_ndc.x = std::min(scene.zone_min_ndc.x, p.x);
            scene.zone_min_ndc.y = std::min(scene.zone_min_ndc.y, p.y);
            scene.zone_max_ndc.x = std::max(scene.zone_max_ndc.x, p.x);
            scene.zone_max_ndc.y = std::max(scene.zone_max_ndc.y, p.y);
        }
    }
    const float overlay_left = scene.zone_min_ndc.x, overlay_right = scene.zone_max_ndc.x;
    const float bottom = scene.zone_min_ndc.y, top = scene.zone_max_ndc.y;
    // Thickness scales with the projected zone, not an authored pixel rectangle.
    const float stroke_x = (overlay_right - overlay_left) * 0.008f, stroke_y = stroke_x * aspect;
    const XMFLOAT3 blue{0.55f, 0.72f, 1};
    for (float x : {overlay_left, overlay_right})
        quad({x - stroke_x / 2, bottom, 0}, {x - stroke_x / 2, top, 0},
            {x + stroke_x / 2, top, 0}, {x + stroke_x / 2, bottom, 0}, blue);
    for (float y : {bottom, top})
        quad({overlay_left, y - stroke_y / 2, 0}, {overlay_left, y + stroke_y / 2, 0},
            {overlay_right, y + stroke_y / 2, 0}, {overlay_right, y - stroke_y / 2, 0}, blue);
    scene.prediction_ndc = project_batting_point(staging, predicted_position, aspect);
    const auto p = scene.prediction_ndc;
    // Match the ball's projected radius at the evaluation plane; keep the stroke inside it.
    const auto forward = XMVectorSubtract(XMLoadFloat3(&staging.camera_target_m), XMLoadFloat3(&staging.camera_position_m));
    const auto camera_right = XMVector3Normalize(XMVector3Cross(XMVectorSet(0, 1, 0, 0), forward));
    XMFLOAT3 edge_position;
    XMStoreFloat3(&edge_position, XMVectorAdd(XMLoadFloat3(&predicted_position),
        XMVectorScale(camera_right, staging.ball_marker_radius_m)));
    const auto edge = project_batting_point(staging, edge_position, aspect);
    const float radius = std::abs(edge.x - p.x);
    const float inner_radius = std::max(0.0f, radius - stroke_x);
    const XMFLOAT3 prediction_color{1, 0.55f, 0.25f};
    for (int i = 0; i < 48; ++i) {
        const float a = XM_2PI * static_cast<float>(i) / 48;
        const float b = XM_2PI * static_cast<float>(i + 1) / 48;
        const auto ring = [&](float angle, float r) -> XMFLOAT3 {
            return {p.x + r * std::cos(angle), p.y + r * aspect * std::sin(angle), 0};
        };
        quad(ring(a, inner_radius), ring(b, inner_radius), ring(b, radius), ring(a, radius), prediction_color);
    }
    return scene;
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

DirectX::XMFLOAT2 project_batting_point(const BattingStaging& staging, DirectX::XMFLOAT3 point, float aspect)
{
    const auto matrix = batting_view_projection(staging, aspect);
    XMFLOAT4 clip;
    XMStoreFloat4(&clip, XMVector4Transform(XMVectorSet(point.x, point.y, point.z, 1), XMLoadFloat4x4(&matrix)));
    if (!std::isfinite(clip.w) || clip.w <= 0)
        throw std::runtime_error("Batting overlay point is behind the camera.");
    return {clip.x / clip.w, clip.y / clip.w};
}
}
