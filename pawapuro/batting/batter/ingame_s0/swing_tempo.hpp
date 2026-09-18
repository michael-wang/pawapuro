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
    // Zero retains the historical sampler for asset/geometry regression callers.
    // Manual attempts snapshot the startup finish duration instead.
    std::uint64_t tail_finish_ticks=0;
    static constexpr double area_ticks=40, sweep_ticks=24, finish_ticks=456;
    static constexpr double protected_ticks=45.6; // 190 ms in WORLD time at 240 Hz.
    double entry_sample_tick(double world,std::optional<std::uint64_t> commit) const {
        if(mode==SwingTempo::Original||!commit||world<=double(*commit))return world;
        const double x=world-double(*commit),u=std::clamp(x/compact_area_ticks,0.,1.);
        return world+(area_ticks-compact_area_ticks)*(3*u*u-2*u*u*u);
    }
    double sample_tick(double world,std::optional<std::uint64_t> commit) const {
        if(!tail_finish_ticks||!commit||world<=double(*commit)+protected_ticks)
            return entry_sample_tick(world,commit);
        const double c=double(*commit),x=world-c;
        if(x>=double(tail_finish_ticks))return c+finish_ticks;
        const double source_start=entry_sample_tick(c+protected_ticks,commit)-c;
        const double duration=double(tail_finish_ticks)-protected_ticks,span=finish_ticks-source_start;
        const double u=(x-protected_ticks)/duration,v=1-u,m=duration/span;
        // Monotonic quartic: source speed 1 at entry, then a broad acceleration
        // and progressively smaller settling spacing, reaching speed 0 at hold.
        const double eased=1-v*v*v*(1+3*u)+m*u*v*v*v;
        return c+source_start+span*eased;
    }
    double world_offset(double source_offset) const {
        if(tail_finish_ticks&&source_offset>entry_sample_tick(protected_ticks,0)) {
            if(source_offset>=finish_ticks)return double(tail_finish_ticks);
            double lo=protected_ticks,hi=double(tail_finish_ticks);
            for(unsigned i=0;i<50;++i){const double mid=(lo+hi)/2;
                if(sample_tick(mid,0)<source_offset)lo=mid;else hi=mid;}
            return (lo+hi)/2;
        }
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
