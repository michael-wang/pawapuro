#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "batting/reference_scene.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string_view>

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
        const auto vertices = pawapuro::make_batting_reference(staging);
        engine::D3D12View view;
        view.initialize(hwnd, static_cast<UINT>(width), static_cast<UINT>(height), vertices);
        bool running = true;
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                    running = false;
                if (event.type == SDL_EVENT_WINDOW_MINIMIZED) std::fprintf(stderr, "Window minimized.\n");
                if (event.type == SDL_EVENT_WINDOW_RESTORED) std::fprintf(stderr, "Window restored.\n");
            }
            if (!running) break;
            if (!SDL_GetWindowSizeInPixels(window.get(), &width, &height)) throw std::runtime_error(SDL_GetError());
            if ((SDL_GetWindowFlags(window.get()) & SDL_WINDOW_MINIMIZED) || width == 0 || height == 0) {
                SDL_Delay(20);
                continue;
            }
            view.resize(static_cast<UINT>(width), static_cast<UINT>(height));
            view.draw(pawapuro::batting_view_projection(staging, static_cast<float>(width) / static_cast<float>(height)));
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
