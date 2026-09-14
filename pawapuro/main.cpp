#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "batting/reference_scene.hpp"
#include "batting/reference_pitch.hpp"
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
        const auto scene = pawapuro::make_batting_reference(staging);
        pawapuro::ReferencePitch pitch(staging.release_position_m, staging.reference_velocity_mps);
        engine::D3D12View view;
        view.initialize(hwnd, static_cast<UINT>(width), static_cast<UINT>(height), scene.vertices, scene.ball_vertex_start);
        const auto report_arrival = [&] {
            const auto& p = pitch.current.position_m;
            const auto& v = pitch.current.velocity_mps;
            std::fprintf(stderr, "Reference arrival: Complete tick=%llu time=%.9f s position=[%.6f, %.6f, %.6f] m "
                "velocity=[%.6f, %.6f, %.6f] m/s previous_z=%.6f plane_z=%.6f\n",
                pitch.tick, static_cast<double>(pitch.tick) / pawapuro::pitch_hz,
                p.x, p.y, p.z, v.x, v.y, v.z, pitch.previous.position_m.z, pawapuro::plate_front_z_m);
        };
        auto last_time = SDL_GetTicksNS();
        std::string last_title;
        bool running = true;
        while (running) {
            const auto now = SDL_GetTicksNS();
            // Account for the old state before handling this frame's input boundary.
            if (pitch.advance(now - last_time)) report_arrival();
            last_time = now;
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                    running = false;
                if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                    if (event.key.scancode == SDL_SCANCODE_ESCAPE) running = false;
                    if (event.key.scancode == SDL_SCANCODE_SPACE && pitch.release()) {
                        const auto& p = pitch.current.position_m;
                        const auto& v = pitch.current.velocity_mps;
                        std::fprintf(stderr, "Reference release: InFlight tick=0 position=[%.6f, %.6f, %.6f] m "
                            "velocity=[%.6f, %.6f, %.6f] m/s; 240 Hz; gravity-only fixture; owner=pawapuro/batting/reference_pitch.cpp\n",
                            p.x, p.y, p.z, v.x, v.y, v.z);
                    }
                    if (event.key.scancode == SDL_SCANCODE_P) pitch.toggle_pause();
                    if (event.key.scancode == SDL_SCANCODE_PERIOD && pitch.single_step()) report_arrival();
                }
                if (event.type == SDL_EVENT_WINDOW_MINIMIZED) {
                    if (pitch.phase == pawapuro::PitchPhase::InFlight && !pitch.paused) pitch.toggle_pause();
                    std::fprintf(stderr, "Window minimized; active pitch paused (P to resume).\n");
                }
                if (event.type == SDL_EVENT_WINDOW_RESTORED) std::fprintf(stderr, "Window restored.\n");
            }
            if (!running) break;
            if (!SDL_GetWindowSizeInPixels(window.get(), &width, &height)) throw std::runtime_error(SDL_GetError());
            if ((SDL_GetWindowFlags(window.get()) & SDL_WINDOW_MINIMIZED) || width == 0 || height == 0) {
                SDL_Delay(20);
                continue;
            }
            view.resize(static_cast<UINT>(width), static_cast<UINT>(height));
            const auto& p = pitch.current.position_m;
            char title[512];
            std::snprintf(title, sizeof(title), "Pawapuro | Right-handed pitcher vs left-handed batter | %s | tick=%llu | "
                "ball=(%.3f, %.3f, %.3f) m | backlog=%llu | Space:release P:pause .:step Esc:quit",
                pitch.state_name(), pitch.tick, p.x, p.y, p.z, pitch.pending_ticks);
            if (last_title != title) {
                if (!SDL_SetWindowTitle(window.get(), title)) throw std::runtime_error(SDL_GetError());
                last_title = title;
            }
            const auto& origin = staging.release_position_m;
            view.draw(pawapuro::batting_view_projection(staging, static_cast<float>(width) / static_cast<float>(height)),
                {p.x - origin.x, p.y - origin.y, p.z - origin.z});
        }
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
