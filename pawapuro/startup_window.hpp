#pragma once
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace pawapuro {
struct WorkArea { int x,y,w,h; };
struct WindowBorders { int top,left,bottom,right; };
struct StartupWindow { int x,y,w,h; bool fitted; };
// Desktop/window coordinates only. x/y denote the client origin, not the outer frame.
inline StartupWindow fit_startup_window(WorkArea area, WindowBorders border, double fraction, int margin=8)
{
    if (!std::isfinite(fraction) || fraction<=0 || fraction>1)
        throw std::runtime_error("window.width_fraction must be finite and in (0, 1].");
    const int available_w=area.w-border.left-border.right-2*margin;
    const int available_h=area.h-border.top-border.bottom-2*margin;
    if (available_w<16 || available_h<9)
        throw std::runtime_error("Display work area cannot fit a decorated 16:9 window.");
    const double target=area.w*fraction;
    const double width=std::min({target,double(available_w),double(available_h)*16/9});
    // Whole 16:9 units avoid aspect drift after rounding, including small displays.
    const int units=std::max(1,static_cast<int>(std::floor(width/16)));
    const int w=units*16,h=units*9;
    return {area.x+(area.w-w-border.left-border.right)/2+border.left,
        area.y+(area.h-h-border.top-border.bottom)/2+border.top,w,h,width<target};
}
}
