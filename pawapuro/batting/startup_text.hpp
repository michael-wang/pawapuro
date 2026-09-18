#pragma once
#include <vector>
namespace pawapuro {
// Temporary startup-only pixel runs shared by the two batting HUD callers.
struct StartupTextMask {
    struct Run { float x,y,w; };
    std::vector<Run> runs;
};
StartupTextMask raster_startup_text(const wchar_t* text,int pixel_size);
}
