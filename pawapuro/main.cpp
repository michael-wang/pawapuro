#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "batting/reference_scene.hpp"
#include "batting/reference_pitch.hpp"
#include "batting/pitcher/pitcher_motion.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <string>

int main(int argc, char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    int result = 0;
    try {
        const auto utf8_path = [](const char* text) {
            return std::filesystem::path(std::u8string(text, text + std::strlen(text)));
        };
        const char* base_path = SDL_GetBasePath();
        if (!base_path) throw std::runtime_error(SDL_GetError());
        auto staging_path = utf8_path(base_path) / "staging.toml";
        if (argc == 3 && std::string_view(argv[1]) == "--staging") staging_path = utf8_path(argv[2]);
        else if (argc != 1) throw std::runtime_error("Usage: pawapuro [--staging path/to/staging.toml]");
        const auto staging = pawapuro::load_batting_staging(staging_path);
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
            SDL_CreateWindow("Pawapuro | Right-handed pitcher vs left-handed batter", 1920, 1080, 0), SDL_DestroyWindow);
        if (!window) throw std::runtime_error(SDL_GetError());
        const auto hwnd = static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(window.get()),
            SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
        if (!hwnd) throw std::runtime_error("SDL window has no HWND.");
        int width = 0, height = 0;
        if (!SDL_GetWindowSizeInPixels(window.get(), &width, &height)) throw std::runtime_error(SDL_GetError());
        std::fprintf(stderr, "Window: %d x %d pixels, fixed windowed 16:9\n", width, height);
        pawapuro::ReferencePitch pitch(staging.release_position_m, staging.reference_velocity_mps, staging.strike_zone_plane_z());
        const auto prediction = pawapuro::predict_arrival(pitch);
        const float aspect = static_cast<float>(width) / static_cast<float>(height);
        pawapuro::PitcherMotion motion(utf8_path(base_path)/"batting/pitcher/pitcher.glb",
            utf8_path(base_path)/"batting/pitcher/pitcher.toml",staging);
        const auto scene=pawapuro::make_batting_reference(staging,prediction.state.position_m,aspect,{});
        std::fprintf(stderr,"S2 preview: Space plays animation only; ball/prediction remain independent reference fixtures.\n");
        const auto screen_point = [&](DirectX::XMFLOAT2 ndc) {
            return DirectX::XMFLOAT2{(ndc.x + 1) * width / 2, (1 - ndc.y) * height / 2};
        };
        const auto predicted_screen = screen_point(scene.prediction_ndc);
        const auto zone_left_top = screen_point({scene.zone_min_ndc.x, scene.zone_max_ndc.y});
        const auto zone_right_bottom = screen_point({scene.zone_max_ndc.x, scene.zone_min_ndc.y});
        std::fprintf(stderr, "Overlay bounds: left=%.6f top=%.6f right=%.6f bottom=%.6f; predicted pixel=[%.6f, %.6f]\n",
            zone_left_top.x, zone_left_top.y, zone_right_bottom.x, zone_right_bottom.y, predicted_screen.x, predicted_screen.y);
        engine::D3D12View view;
        view.initialize(hwnd, static_cast<UINT>(width), static_cast<UINT>(height), scene.vertices, scene.ball_vertex_start, scene.overlay_vertex_start, static_cast<UINT>(motion.triangles.size()));
        auto last_time = SDL_GetTicksNS();
        std::string last_title;
        bool running = true;
        while (running) {
            const auto now = SDL_GetTicksNS();
            // Account for the old state before handling this frame's input boundary.
            if (motion.advance(now - last_time)) std::fprintf(stderr,"Animation Complete: tick=%llu time=%.9f\n",motion.tick,motion.time_s());
            last_time = now;
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                    running = false;
                if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                    if (event.key.scancode == SDL_SCANCODE_ESCAPE) running = false;
                    if (event.key.scancode == SDL_SCANCODE_SPACE && motion.start()) {
                        last_time=SDL_GetTicksNS();
                        std::fprintf(stderr,"Animation start/replay: tick=0; reference ball unchanged.\n");
                    }
                    if (event.key.scancode == SDL_SCANCODE_P) motion.toggle_pause();
                    if (event.key.scancode == SDL_SCANCODE_PERIOD) motion.single_step();
                }
                if (event.type == SDL_EVENT_WINDOW_MINIMIZED) {
                    if (motion.phase == pawapuro::MotionPhase::Playing && !motion.paused) motion.toggle_pause();
                    std::fprintf(stderr, "Window minimized; active animation paused (P to resume).\n");
                }
                if (event.type == SDL_EVENT_WINDOW_RESTORED) { last_time=SDL_GetTicksNS(); std::fprintf(stderr,"Window restored; animation remains paused.\n"); }
            }
            if (!running) break;
            if (!SDL_GetWindowSizeInPixels(window.get(), &width, &height)) throw std::runtime_error(SDL_GetError());
            if ((SDL_GetWindowFlags(window.get()) & SDL_WINDOW_MINIMIZED) || width == 0 || height == 0) {
                SDL_Delay(20);
                continue;
            }
            view.resize(static_cast<UINT>(width), static_cast<UINT>(height));
            char title[512];
            std::snprintf(title,sizeof(title),"Pawapuro | Pitcher animation preview | %s | tick=%llu | time=%.6f | "
                "grip=(%.3f, %.3f, %.3f) | backlog=%llu | Independent reference ball | Space:play/replay P:pause .:step Esc:quit",
                motion.state_name(),motion.tick,motion.time_s(),motion.grip_world[12],motion.grip_world[13],motion.grip_world[14],motion.pending_ticks);
            if (last_title != title) {
                if (!SDL_SetWindowTitle(window.get(), title)) throw std::runtime_error(SDL_GetError());
                last_title = title;
            }
            view.draw(pawapuro::batting_view_projection(staging, static_cast<float>(width) / static_cast<float>(height)),
                {0,0,0},motion.triangles);
        }
        if (motion.samples) std::fprintf(stderr,"CPU motion: samples=%llu pose_mean_us=%.3f skin_expand_basis_mean_us=%.3f\n",
            motion.samples,motion.pose_us/static_cast<double>(motion.samples),motion.skin_us/static_cast<double>(motion.samples));
        // view is destroyed before window; SDL remains alive through both destructors.
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Pawapuro: %s\n", error.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Pawapuro could not continue", error.what(), nullptr);
        result = 1;
    }
    SDL_Quit();
    std::fprintf(stderr, "Application exit: %d\n", result);
    return result;
}
