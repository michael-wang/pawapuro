#pragma once
#include "manual_swing.hpp"
namespace pawapuro {
// Player-facing presentation only; never participates in hit acceptance or response.
enum class InfoTiming { Waiting, Early, Center, Late };
inline constexpr const wchar_t* batting_info_labels[]={L"揮棒時機",L"飛行時間",L"飛行距離",L"最大高度"};
inline constexpr const wchar_t* info_timing_labels[]={L"--",L"太早",L"剛好",L"太晚"};
struct BattingFlightMetrics {
    double elapsed_s=0,distance_m=0,max_height_m=0;
    bool operator==(const BattingFlightMetrics&) const = default;
};
struct BattingInfo {
    InfoTiming timing=InfoTiming::Waiting;
    std::optional<double> offset_ms;
    std::optional<BattingFlightMetrics> flight;
    bool operator==(const BattingInfo&) const = default;
};
BattingInfo batting_info(const ManualSwingPreview& preview);
// Fixed stack buffers and startup-cached glyphs keep the dynamic append path allocation-free.
using BattingInfoText=std::array<std::array<char,16>,4>;
BattingInfoText format_batting_info(const BattingInfo& info);
struct ContactResultPanel {
    explicit ContactResultPanel(unsigned pixel_width,unsigned pixel_height);
    std::vector<engine::Vertex> base;
    std::array<std::vector<engine::Vertex>,4> timing_text;
    std::array<std::vector<engine::Vertex>,2> tempo_text;
    std::array<std::vector<engine::Vertex>,16> glyphs;
    unsigned vertex_count=0;
    float x_scale=0,y_scale=0;
    void append(std::vector<engine::Vertex>& target,const ManualSwingPreview& preview) const;
};
}
