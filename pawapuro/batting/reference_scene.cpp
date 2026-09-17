#include "reference_scene.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>

using namespace DirectX;
namespace pawapuro {
void append_ball_readability(std::vector<engine::Vertex>& vertices, const BattingStaging& staging,
    XMFLOAT3 center, PitchPhase phase, bool enabled, unsigned width, unsigned height)
{
    if (!enabled || phase != PitchPhase::InFlight || width == 0 || height == 0) {
        vertices.resize(vertices.size() + ball_readability_vertex_count);
        return;
    }
    const float w=static_cast<float>(width), h=static_cast<float>(height);
    const float aspect = w/h;
    const auto p = project_batting_point(staging, center, aspect);
    const auto forward = XMVectorSubtract(XMLoadFloat3(&staging.camera_target_m), XMLoadFloat3(&staging.camera_position_m));
    const auto right = XMVector3Normalize(XMVector3Cross(XMVectorSet(0,1,0,0), forward));
    XMFLOAT3 edge;
    XMStoreFloat3(&edge, XMVectorAdd(XMLoadFloat3(&center), XMVectorScale(right, staging.ball_marker_radius_m)));
    // Follow the existing visual radius, with a small screen-space readability floor.
    const float radius = std::max(5.0f, std::abs(project_batting_point(staging, edge, aspect).x-p.x)*w/2);
    const auto band = [&](float inner, float outer, XMFLOAT3 color) {
        const auto point = [&](float angle, float r) -> XMFLOAT3 {
            return {p.x+2*r*std::cos(angle)/w, p.y+2*r*std::sin(angle)/h, 0};
        };
        for (unsigned i=0; i<32; ++i) {
            const float a=XM_2PI*static_cast<float>(i)/32, b=XM_2PI*static_cast<float>(i+1)/32;
            const auto v0=point(a,inner), v1=point(b,inner), v2=point(b,outer), v3=point(a,outer);
            vertices.insert(vertices.end(), {{v0,color},{v1,color},{v2,color},{v0,color},{v2,color},{v3,color}});
        }
    };
    band(radius-2, radius+2, {0.015f,0.015f,0.015f});
    band(radius-1, radius+1, {1,1,0.2f});
}

void append_arrival_cue(std::vector<engine::Vertex>& vertices, const BattingReference& scene,
    PitchPhase phase, ArrivalCueStyle style)
{
    if (!arrival_cue_visible(phase)) {
        vertices.resize(vertices.size()+arrival_cue_vertex_count);
        return;
    }
    const auto& cue=style==ArrivalCueStyle::Baseball ? scene.arrival_baseball : scene.arrival_ring;
    vertices.insert(vertices.end(),cue.begin(),cue.end());
}

BattingReference make_batting_reference(const BattingStaging& staging, XMFLOAT3 predicted_position, float aspect,
    std::span<const engine::Vertex> pitcher_vertices, std::span<const engine::Vertex> batter_vertices)
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
    // Broad, subtle grass bands communicate depth beyond the pitcher toward the outfield wall.
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
    // Ground markings share only this local strip operation; no field/terrain system.
    const auto chalk = [&](float ax, float az, float bx, float bz) {
        const float length = std::hypot(bx - ax, bz - az);
        const float dx = -(bz - az) * 0.025f / length, dz = (bx - ax) * 0.025f / length;
        quad({ax + dx, 0.018f, az + dz}, {bx + dx, 0.018f, bz + dz},
            {bx - dx, 0.018f, bz - dz}, {ax - dx, 0.018f, az - dz}, white);
    };
    for (float side : {-1.0f, 1.0f}) {
        const float inside = side * (half_width + 0.18f), outside = inside + side * 1.2f;
        chalk(inside, -0.6f, outside, -0.6f);
        chalk(outside, -0.6f, outside, 1.4f);
        chalk(outside, 1.4f, inside, 1.4f);
        chalk(inside, 1.4f, inside, -0.6f);
    }
    // Standard 90-degree diamond directions. Bases may lie outside this batting camera.
    constexpr float base_distance = 27.432f, wall_radius = 110, wall_height = 3.5f;
    const float base_axis = base_distance / std::sqrt(2.0f);
    const float foul_end = wall_radius / std::sqrt(2.0f);
    for (float side : {-1.0f, 1.0f}) {
        // Leave the batting-box chalk uncluttered; direction still originates at the plate tip.
        chalk(side * 2, 2, side * foul_end, foul_end);
        const float x = side * base_axis, z = base_axis, r = 0.23f;
        quad({x-r, 0.10f, z-r}, {x-r, 0.10f, z+r}, {x+r, 0.10f, z+r}, {x+r, 0.10f, z-r}, white);
        quad({x-r, 0.004f, z-r}, {x-r, 0.10f, z-r}, {x+r, 0.10f, z-r}, {x+r, 0.004f, z-r}, white);
        quad({x-r, 0.004f, z+r}, {x-r, 0.10f, z+r}, {x+r, 0.10f, z+r}, {x+r, 0.004f, z+r}, white);
        for (float edge : {-r, r})
            quad({x+edge, 0.004f, z-r}, {x+edge, 0.10f, z-r}, {x+edge, 0.10f, z+r}, {x+edge, 0.004f, z+r}, white);
    }
    // A bounded outfield silhouette only: no collision or home-run judgement.
    for (int i = 0; i < 48; ++i) {
        const float a = -XM_PIDIV4 + XM_PIDIV2 * static_cast<float>(i) / 48;
        const float b = -XM_PIDIV4 + XM_PIDIV2 * static_cast<float>(i + 1) / 48;
        const float ax = wall_radius * std::sin(a), az = wall_radius * std::cos(a);
        const float bx = wall_radius * std::sin(b), bz = wall_radius * std::cos(b);
        const XMFLOAT3 color = i % 2 ? XMFLOAT3{0.13f, 0.30f, 0.32f} : XMFLOAT3{0.15f, 0.33f, 0.35f};
        quad({ax, 0, az}, {ax, wall_height-0.12f, az}, {bx, wall_height-0.12f, bz}, {bx, 0, bz}, color);
        quad({ax, wall_height-0.12f, az}, {ax, wall_height, az}, {bx, wall_height, bz},
            {bx, wall_height-0.12f, bz}, {0.90f, 0.69f, 0.25f});
    }
    // A raised flat-top mound supplies staging presence, not simulation terrain.
    const auto mound_point = [](float angle, float radius, float y) -> XMFLOAT3 {
        return {radius * std::cos(angle), y, mound_center_z_m + radius * std::sin(angle)};
    };
    for (int i = 0; i < 48; ++i) {
        const float a = XM_2PI * static_cast<float>(i) / 48;
        const float b = XM_2PI * static_cast<float>(i + 1) / 48;
        quad(mound_point(a, staging.mound_radius_m, 0.008f), mound_point(b, staging.mound_radius_m, 0.008f),
            mound_point(b, staging.mound_top_radius_m, staging.mound_height_m),
            mound_point(a, staging.mound_top_radius_m, staging.mound_height_m), {0.46f, 0.33f, 0.22f});
        triangle({0, staging.mound_height_m, mound_center_z_m},
            mound_point(a, staging.mound_top_radius_m, staging.mound_height_m),
            mound_point(b, staging.mound_top_radius_m, staging.mound_height_m), {0.54f, 0.40f, 0.27f});
    }
    const float rubber_y = staging.mound_height_m + 0.005f;
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
    const float base_y = 0.008f + (staging.mound_height_m - 0.008f) * std::clamp(
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
    // Imported characters are ordinary immutable world geometry in this fixture.
    vertices.insert(vertices.end(), pitcher_vertices.begin(), pitcher_vertices.end());
    vertices.insert(vertices.end(), batter_vertices.begin(), batter_vertices.end());
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
    // Prediction geometry is stored separately: calculation does not reveal it.
    const auto cue_quad = [](std::vector<engine::Vertex>& target, XMFLOAT3 a, XMFLOAT3 b,
        XMFLOAT3 c, XMFLOAT3 d, XMFLOAT3 color) {
        target.insert(target.end(), {{a,color},{b,color},{c,color},{a,color},{c,color},{d,color}});
    };
    for (int i = 0; i < 48; ++i) {
        const float a = XM_2PI * static_cast<float>(i) / 48;
        const float b = XM_2PI * static_cast<float>(i + 1) / 48;
        const auto ring = [&](float angle, float r) -> XMFLOAT3 {
            return {p.x + r * std::cos(angle), p.y + r * aspect * std::sin(angle), 0};
        };
        cue_quad(scene.arrival_ring,ring(a,inner_radius),ring(b,inner_radius),ring(b,radius),ring(a,radius),{1,0.55f,0.25f});
        cue_quad(scene.arrival_baseball,ring(a,inner_radius),ring(b,inner_radius),ring(b,radius),ring(a,radius),{0.96f,0.95f,0.87f});
    }
    // Thin, static seams leave the centre open for the actual ball and aim core.
    const auto seam_line = [&](float ax, float ay, float bx, float by) {
        const float length=std::hypot(bx-ax,by-ay), half=stroke_x/radius*0.5f;
        const float dx=-(by-ay)*half/length, dy=(bx-ax)*half/length;
        const auto pt=[&](float x,float y) -> XMFLOAT3 { return {p.x+x*radius,p.y+y*radius*aspect,0}; };
        cue_quad(scene.arrival_baseball,pt(ax+dx,ay+dy),pt(bx+dx,by+dy),
            pt(bx-dx,by-dy),pt(ax-dx,ay-dy),{0.95f,0.12f,0.16f});
    };
    for (float side : {-1.0f,1.0f}) {
        const auto x=[&](float y) { return side*(0.4f+0.32f*y*y); };
        for (int i=0;i<12;++i) {
            const float a=-0.78f+1.56f*static_cast<float>(i)/12, b=-0.78f+1.56f*static_cast<float>(i+1)/12;
            seam_line(x(a),a,x(b),b);
        }
        for (int i=0;i<5;++i) {
            const float y=-0.6f+0.3f*static_cast<float>(i);
            seam_line(x(y)-0.09f,y-0.04f,x(y)+0.09f,y+0.04f);
        }
    }
    scene.arrival_ring.resize(arrival_cue_vertex_count);
    return scene;
}

XMFLOAT4X4 batting_view_projection(const BattingStaging& staging, float aspect)
{
    // Left-handed batter framing; camera orientation does not move the rubber or mound.
    const auto view = XMMatrixLookAtLH(XMLoadFloat3(&staging.camera_position_m),
        XMLoadFloat3(&staging.camera_target_m), XMVectorSet(0, 1, 0, 0));
    // Lateral position avoids the batter; lens shift centres gameplay without yawing the field.
    // All world draws, overlay points and arrival diagnostics use this same matrix function.
    const auto focus = XMVector3TransformCoord(XMVectorSet(0,
        (staging.strike_zone_bottom_m + staging.strike_zone_top_m) / 2,
        staging.strike_zone_plane_z(), 1), view);
    const float focus_z = XMVectorGetZ(focus);
    if (!std::isfinite(focus_z) || focus_z <= 0)
        throw std::runtime_error("Batting projection focus must be in front of the camera.");
    constexpr float near_z = 0.1f, far_z = 300;
    const float half_height = near_z * std::tan(XMConvertToRadians(staging.vertical_fov_degrees) / 2);
    const float half_width = half_height * aspect;
    const float shift = near_z * XMVectorGetX(focus) / focus_z;
    const auto projection = XMMatrixPerspectiveOffCenterLH(shift - half_width, shift + half_width,
        -half_height, half_height, near_z, far_z);
    XMFLOAT4X4 result;
    XMStoreFloat4x4(&result, view * projection);
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
