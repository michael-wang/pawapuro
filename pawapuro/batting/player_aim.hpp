#pragma once
#include "staging.hpp"
#include "engine/rendering/d3d12_view.hpp"
#include <vector>

namespace pawapuro {
struct AimDiagnostic { float dx,dy,ex,ey,q; };
// App-owned development intent, independent of BattingPreview and its 240 Hz clock.
class PlayerAim {
public:
    explicit PlayerAim(const BattingStaging& staging);
    DirectX::XMFLOAT2 center() const { return center_m; }
    void recenter();
    // Signed world axes; the current fixed camera maps right/up to +X/+Y.
    void move(float x_axis,float y_axis,double elapsed_s);
    AimDiagnostic diagnostic(DirectX::XMFLOAT3 predicted) const;
    static constexpr size_t vertex_count = 32*6+2*6+6;
    void append_triangles(std::vector<engine::Vertex>& destination) const;
    void append_triangles_at(std::vector<engine::Vertex>& destination,DirectX::XMFLOAT2 center) const;
private:
    PlayerAimTuning tuning;
    HitAuthorizationTuning authorization;
    DirectX::XMFLOAT2 center_m{};
    float half_width,bottom,top,plane_z,visual_z;
};
}
