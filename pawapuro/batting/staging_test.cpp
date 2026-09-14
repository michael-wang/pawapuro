#include "staging.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
std::array<float, 17> values(const pawapuro::BattingStaging& s)
{
    return {s.camera_position_m.x, s.camera_position_m.y, s.camera_position_m.z,
        s.camera_target_m.x, s.camera_target_m.y, s.camera_target_m.z, s.vertical_fov_degrees,
        s.release_position_m.x, s.release_position_m.y, s.release_position_m.z,
        s.ball_marker_radius_m, s.grass_half_width_m, s.grass_end_z_m, s.mound_radius_m, s.mound_top_radius_m, s.home_dirt_radius_m, s.mound_visual_dirt_radius_m};
}
}
int main(int argc, char** argv)
{
    try {
        if (argc != 3) throw std::runtime_error("Expected authored TOML and build-local test directory.");
        const std::filesystem::path authored = argv[1], directory = argv[2];
        std::filesystem::create_directories(directory);
        const auto fixture = directory / "candidate.toml";
        const auto write = [&](const char* text) { std::ofstream(fixture) << text; };
        const pawapuro::BattingStaging defaults;
        // Tuning the authored preset must not require changing Native fallback values.
        (void)pawapuro::load_batting_staging(authored);
        write("");
        if (values(pawapuro::load_batting_staging(fixture)) != values(defaults))
            throw std::runtime_error("Missing fields did not use defaults.");
        write("[camera]\nvertical_fov_degrees = 42\n");
        if (pawapuro::load_batting_staging(fixture).vertical_fov_degrees != 42)
            throw std::runtime_error("Valid integer override was not applied.");
        write("[field]\nhome_dirt_radius_m = 3.1\n[mound]\nvisual_dirt_radius_m = 6.2\n");
        const auto dirt = pawapuro::load_batting_staging(fixture);
        if (dirt.home_dirt_radius_m != 3.1f || dirt.mound_visual_dirt_radius_m != 6.2f)
            throw std::runtime_error("Visual dirt overrides were not applied.");
        write("[camera]\nposition_m = [-0.75, 1.25, -5]\n");
        if (pawapuro::load_batting_staging(fixture).camera_position_m.x != -0.75f)
            throw std::runtime_error("Opposite-side camera override was not applied.");
        const auto reject = [&](const std::filesystem::path& path, const char* text) {
            try { (void)pawapuro::load_batting_staging(path); }
            catch (const std::runtime_error& error) {
                const std::string message = error.what();
                if (message.find("Staging file:") != std::string::npos && message.find(text) != std::string::npos) return;
                throw std::runtime_error("Missing source/key diagnostic: " + message);
            }
            throw std::runtime_error("Invalid staging was accepted.");
        };
        std::filesystem::remove(fixture);
        reject(fixture, "candidate.toml");
        struct Invalid { const char* toml; const char* diagnostic; };
        const Invalid invalid[] = {
            {"[camera\n", "line"},
            {"[camera]\nvertical_fov_degrees = 0\n", "camera.vertical_fov_degrees"},
            {"[camera]\nvertical_fov_degrees = nan\n", "camera.vertical_fov_degrees"},
            {"[camera]\nvertical_fov_degrees = inf\n", "camera.vertical_fov_degrees"},
            {"[camera]\nvertical_fov_degrees = '40'\n", "camera.vertical_fov_degrees"},
            {"[camera]\nvertical_fov_degrees = true\n", "camera.vertical_fov_degrees"},
            {"[camera]\nposition_m = [0, 1]\n", "camera.position_m"},
            {"[camera]\nposition_m = [0.7, nan, -5]\n", "camera.position_m[1]"},
            {"[camera]\nposition_m = [-2, 1.4, -5]\n", "camera.position_m[0]"},
            {"[camera]\npreset = 'right_handed'\n", "camera.preset"},
            {"[camera]\nfvo = 40\n", "camera.fvo"},
            {"camera = 3\n", "camera must be a table"},
            {"[unknown]\n", "Unknown staging key"},
            {"[release]\nball_marker_radius_m = -1\n", "release.ball_marker_radius_m"},
            {"[release]\nposition_m = [-0.65, 0, 16.8]\n", "release.position_m[1]"},
            {"[field]\ngrass_end_z_m = 35\n", "field.grass_end_z_m"},
            {"[field]\ngrass_half_width_m = 1000\n", "field.grass_half_width_m"},
            {"[mound]\ntop_radius_m = 4\n", "mound.top_radius_m"},
            {"[field]\nhome_dirt_radius_m = -1\n", "field.home_dirt_radius_m"},
            {"[field]\nhome_dirt_radius_m = 10\n", "field.home_dirt_radius_m"},
            {"[mound]\nvisual_dirt_radius_m = 2\n", "mound.visual_dirt_radius_m"},
            {"[mound]\nvisual_dirt_radius_m = 8\n", "mound.visual_dirt_radius_m"},
            {"[release]\nposition_m = [0.65, 2.05, 16.8]\n", "release.position_m[0]"}
        };
        for (const auto& test : invalid) { write(test.toml); reject(fixture, test.diagnostic); }
        std::filesystem::remove(fixture);
        std::cout << "Defaults, authored preset, valid override, missing file and 23 invalid cases passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
