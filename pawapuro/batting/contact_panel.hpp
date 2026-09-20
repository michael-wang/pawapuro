#pragma once
#include "manual_swing.hpp"
namespace pawapuro {
// Player-facing presentation only; never participates in hit acceptance or response.
enum class TimingMarker { None, SwingWithoutContact, Contact };
struct TimingTimeline {
    double decision_start=0,decision_end=0,marker_position=0;
    TimingMarker marker=TimingMarker::None;
    bool operator==(const TimingTimeline&) const = default;
};
inline constexpr const wchar_t* batting_info_labels[]={L"揮棒時機",L"飛行時間",L"飛行距離",L"最大高度",L"擊球初速"};
struct BattingFlightMetrics {
    double elapsed_s=0,distance_m=0,max_height_m=0;
    bool operator==(const BattingFlightMetrics&) const = default;
};
struct BattingInfo {
    TimingTimeline timeline;
    std::optional<double> offset_ms;
    std::optional<double> exit_speed_kmh;
    std::optional<BattingFlightMetrics> flight;
    bool operator==(const BattingInfo&) const = default;
};
BattingInfo batting_info(const ManualSwingPreview& preview);
// Fixed stack buffers and startup-cached glyphs keep the dynamic append path allocation-free.
using BattingInfoText=std::array<std::array<char,16>,5>;
BattingInfoText format_batting_info(const BattingInfo& info);
struct ContactResultPanel {
    explicit ContactResultPanel(unsigned pixel_width,unsigned pixel_height);
    std::vector<engine::Vertex> base;
    std::array<std::vector<engine::Vertex>,2> timing_markers;
    std::array<std::vector<engine::Vertex>,2> tempo_text;
    std::array<std::vector<engine::Vertex>,19> glyphs;
    unsigned vertex_count=0;
    float x_scale=0,y_scale=0;
    void append(std::vector<engine::Vertex>& target,const ManualSwingPreview& preview) const;
};
}
