#include "reference_scene.hpp"
#include <cmath>

using namespace DirectX;
namespace pawapuro {
std::vector<engine::Vertex> make_batting_reference()
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
    quad({-15, 0, -6}, {-15, 0, 35}, {15, 0, 35}, {15, 0, -6}, {0.075f, 0.20f, 0.15f});
    quad({-1.6f, 0.002f, -1}, {-1.6f, 0.002f, 20}, {1.6f, 0.002f, 20}, {1.6f, 0.002f, -1},
        {0.33f, 0.23f, 0.17f});
    // Sparse metre-scale references make the long pitching distance readable.
    for (int z = 2; z <= 20; z += 2) {
        const float distance = static_cast<float>(z);
        quad({-1.6f, 0.004f, distance}, {-1.6f, 0.004f, distance + 0.025f},
            {1.6f, 0.004f, distance + 0.025f}, {1.6f, 0.004f, distance}, {0.43f, 0.33f, 0.24f});
    }
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
    quad({-0.305f, 0.01f, 18.44f}, {-0.305f, 0.01f, 18.59f},
        {0.305f, 0.01f, 18.59f}, {0.305f, 0.01f, 18.44f}, white);

    // Release is ahead of the rubber. Cyan support/ring identifies the reference,
    // while the ivory ball stays distinct. Its 0.10 m radius is an oversized marker.
    constexpr XMFLOAT3 release{0.25f, 1.8f, 16.8f};
    const XMFLOAT3 cyan{0.16f, 0.80f, 0.86f};
    quad({release.x - 0.025f, 0.01f, release.z}, {release.x - 0.025f, 1.49f, release.z},
        {release.x + 0.025f, 1.49f, release.z}, {release.x + 0.025f, 0.01f, release.z}, cyan);
    for (int i = 0; i < 32; ++i) {
        const float a = XM_2PI * static_cast<float>(i) / 32;
        const float b = XM_2PI * static_cast<float>(i + 1) / 32;
        const auto ring_point = [&](float angle, float radius) -> XMFLOAT3 {
            return {release.x + radius * std::cos(angle), release.y + radius * std::sin(angle), release.z};
        };
        quad(ring_point(a, 0.28f), ring_point(b, 0.28f), ring_point(b, 0.31f), ring_point(a, 0.31f), cyan);
    }
    // A small faceted sphere, generated only for this one fixture, not a primitive API.
    const auto ball_point = [&](int latitude, int longitude) -> XMFLOAT3 {
        const float a = XM_PI * static_cast<float>(latitude) / 8;
        const float b = XM_2PI * static_cast<float>(longitude) / 16;
        return {release.x + 0.10f * std::sin(a) * std::cos(b),
            release.y + 0.10f * std::cos(a), release.z + 0.10f * std::sin(a) * std::sin(b)};
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

XMFLOAT4X4 batting_view_projection(float aspect)
{
    // Offset slightly toward a batter; downward framing keeps the plate visible.
    const auto view = XMMatrixLookAtLH(XMVectorSet(-0.65f, 1.65f, -2.5f, 1),
        XMVectorSet(0, 0.9f, 9, 1), XMVectorSet(0, 1, 0, 0));
    XMFLOAT4X4 result;
    XMStoreFloat4x4(&result, view * XMMatrixPerspectiveFovLH(XMConvertToRadians(65), aspect, 0.1f, 100));
    return result;
}
}
