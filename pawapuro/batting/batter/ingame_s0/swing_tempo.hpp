#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
namespace pawapuro {
enum class SwingTempo { Original, Compact };
// Per-attempt snapshot. Commit remains world time; the original evaluator owns entry conditions.
struct SwingTempoTiming {
    SwingTempo mode=SwingTempo::Original;
    double compact_area_ticks=30;
    static constexpr double area_ticks=40, sweep_ticks=24, finish_ticks=456;
    double sample_tick(double world,std::optional<std::uint64_t> commit) const {
        if(mode==SwingTempo::Original||!commit||world<=double(*commit))return world;
        const double x=world-double(*commit),u=std::clamp(x/compact_area_ticks,0.,1.);
        return world+(area_ticks-compact_area_ticks)*(3*u*u-2*u*u*u);
    }
    double world_offset(double source_offset) const {
        if(mode==SwingTempo::Original)return source_offset;
        const double d=area_ticks-compact_area_ticks;
        if(source_offset>=area_ticks)return source_offset-d;
        double lo=0,hi=compact_area_ticks;
        for(unsigned i=0;i<50;++i){const double mid=(lo+hi)/2;
            if(sample_tick(mid,0)<source_offset)lo=mid;else hi=mid;}
        return (lo+hi)/2;
    }
    std::uint64_t end_tick(std::uint64_t commit) const {
        return commit+static_cast<std::uint64_t>(std::ceil(world_offset(finish_ticks)));
    }
};
}
