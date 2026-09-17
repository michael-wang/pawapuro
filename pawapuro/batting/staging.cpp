#include "staging.hpp"
#include <toml++/toml.hpp>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace pawapuro {
namespace {
void only_keys(const toml::table& table, std::initializer_list<std::string_view> allowed,
    const std::string& prefix)
{
    for (const auto& [key, value] : table) {
        (void)value;
        if (std::find(allowed.begin(), allowed.end(), key.str()) == allowed.end())
            throw std::runtime_error("Unknown staging key: " + prefix + std::string(key.str()));
    }
}
float checked_number(const toml::node& node, const std::string& key, float low, float high)
{
    const auto value = node.value<double>();
    if (!node.is_number() || !value || !std::isfinite(*value) || *value < low || *value > high)
        throw std::runtime_error(key + " must be a finite number in [" +
            std::to_string(low) + ", " + std::to_string(high) + "].");
    return static_cast<float>(*value);
}
float number(const toml::table& table, const char* key, float fallback, float low, float high)
{
    const auto node = table.at_path(key);
    if (node) return checked_number(*node.node(), key, low, high);
    std::fprintf(stderr, "Staging default: %s = %g\n", key, fallback);
    return fallback;
}
DirectX::XMFLOAT3 vector(const toml::table& table, const char* key,
    DirectX::XMFLOAT3 fallback, DirectX::XMFLOAT3 low, DirectX::XMFLOAT3 high)
{
    const auto node = table.at_path(key);
    if (!node) {
        std::fprintf(stderr, "Staging default: %s = [%g, %g, %g]\n", key, fallback.x, fallback.y, fallback.z);
        return fallback;
    }
    const auto* values = node.as_array();
    if (!values || values->size() != 3) throw std::runtime_error(std::string(key) + " must contain exactly three numbers.");
    return {checked_number(*values->get(0), std::string(key) + "[0]", low.x, high.x),
        checked_number(*values->get(1), std::string(key) + "[1]", low.y, high.y),
        checked_number(*values->get(2), std::string(key) + "[2]", low.z, high.z)};
}
}

BattingStaging load_batting_staging(const std::filesystem::path& path)
{
    // Parse and validate the whole candidate before geometry/camera can consume it.
    // toml++ remains private to this caller; no parser nodes escape into the engine.
    const auto utf8 = path.u8string();
    const std::string source(utf8.begin(), utf8.end());
    try {
        const auto table = toml::parse_file(utf8);
        only_keys(table, {"window", "camera", "release", "field", "mound", "reference_pitch", "strike_zone", "pitcher_blockout", "batter_blockout", "character_style", "bat_contact", "player_aim"}, "");
        for (const char* section : {"window", "camera", "release", "field", "mound", "reference_pitch", "strike_zone", "pitcher_blockout", "batter_blockout", "character_style", "bat_contact", "player_aim"}) {
            if (const auto* node = table.get(section)) {
                if (!node->is_table()) throw std::runtime_error(std::string(section) + " must be a table.");
                const auto& fields = *node->as_table();
                const std::string prefix = std::string(section) + ".";
                if (prefix == "window.") only_keys(fields, {"width_fraction"}, prefix);
                if (prefix == "player_aim.") only_keys(fields, {"normal_radius_x_m", "normal_radius_y_m", "cursor_speed_mps"}, prefix);
                if (prefix == "bat_contact.") only_keys(fields, {"ball_radius_m", "bat_radius_m"}, prefix);
                if (prefix == "camera.") only_keys(fields, {"preset", "position_m", "target_m", "vertical_fov_degrees"}, prefix);
                if (prefix == "release.") only_keys(fields, {"position_m", "ball_marker_radius_m"}, prefix);
                if (prefix == "field.") only_keys(fields, {"grass_half_width_m", "grass_end_z_m", "home_dirt_radius_m"}, prefix);
                if (prefix == "reference_pitch.") only_keys(fields, {"initial_velocity_mps"}, prefix);
                if (prefix == "pitcher_blockout." || prefix == "batter_blockout.")
                    only_keys(fields, {"position_m", "height_m"}, prefix);
                if (prefix == "character_style.")
                    only_keys(fields, {"head_scale", "foot_planar_scale", "foot_height_scale", "hand_scale", "bat_thickness_scale"}, prefix);
                if (prefix == "strike_zone.") only_keys(fields, {"width_m", "bottom_m", "top_m"}, prefix);
                if (prefix == "mound.") only_keys(fields, {"radius_m", "top_radius_m", "visual_dirt_radius_m", "height_m"}, prefix);
            }
        }
        if (const auto preset = table.at_path("camera.preset")) {
            if (preset.value<std::string>() != "right_handed_pitcher_vs_left_handed_batter")
                throw std::runtime_error("camera.preset must be 'right_handed_pitcher_vs_left_handed_batter'; other presets are not implemented.");
        } else {
            std::fprintf(stderr, "Staging default: camera.preset = right_handed_pitcher_vs_left_handed_batter\n");
        }
        BattingStaging candidate;
        if (const auto node=table.at_path("window.width_fraction")) {
            const auto value=node.value<double>();
            if (!node.is_number() || !value || !std::isfinite(*value) || *value<=0 || *value>1)
                throw std::runtime_error("window.width_fraction must be a finite number in (0, 1].");
            candidate.window_width_fraction=*value;
        }
        candidate.bat_contact.ball_radius_m=number(table,"bat_contact.ball_radius_m",candidate.bat_contact.ball_radius_m,0.001f,0.2f);
        candidate.bat_contact.bat_radius_m=number(table,"bat_contact.bat_radius_m",candidate.bat_contact.bat_radius_m,0.001f,0.2f);
        candidate.camera_position_m = vector(table, "camera.position_m", candidate.camera_position_m,
            {-1.5f, 0.8f, -10}, {1.5f, 2.2f, -2});
        candidate.camera_target_m = vector(table, "camera.target_m", candidate.camera_target_m,
            {-3, 0.4f, 10}, {3, 2.8f, 25});
        candidate.vertical_fov_degrees = number(table, "camera.vertical_fov_degrees", candidate.vertical_fov_degrees, 30, 70);
        candidate.release_position_m = vector(table, "release.position_m", candidate.release_position_m,
            {-1.5f, 1.4f, 15}, {-0.1f, 3, rubber_distance_m});
        candidate.reference_velocity_mps = vector(table, "reference_pitch.initial_velocity_mps", candidate.reference_velocity_mps,
            {-5, -5, -60}, {5, 5, -20});
        candidate.strike_zone_width_m = number(table, "strike_zone.width_m", candidate.strike_zone_width_m, 0.25f, 1.5f);
        candidate.strike_zone_bottom_m = number(table, "strike_zone.bottom_m", candidate.strike_zone_bottom_m, 0.2f, 1);
        candidate.strike_zone_top_m = number(table, "strike_zone.top_m", candidate.strike_zone_top_m, 0.8f, 2);
        if (candidate.strike_zone_top_m - candidate.strike_zone_bottom_m < 0.2f)
            throw std::runtime_error("strike_zone.top_m must exceed bottom_m by at least 0.2 m.");
        candidate.player_aim.normal_radius_x_m=number(table,"player_aim.normal_radius_x_m",candidate.player_aim.normal_radius_x_m,0.001f,1.5f);
        candidate.player_aim.normal_radius_y_m=number(table,"player_aim.normal_radius_y_m",candidate.player_aim.normal_radius_y_m,0.001f,1.8f);
        candidate.player_aim.cursor_speed_mps=number(table,"player_aim.cursor_speed_mps",candidate.player_aim.cursor_speed_mps,0.001f,10.0f);
        if (candidate.player_aim.normal_radius_x_m >= candidate.strike_zone_width_m ||
            candidate.player_aim.normal_radius_y_m >= candidate.strike_zone_top_m-candidate.strike_zone_bottom_m)
            throw std::runtime_error("player_aim radii must be smaller than the zone width/height.");
        candidate.pitcher_blockout_position_m = vector(table, "pitcher_blockout.position_m", candidate.pitcher_blockout_position_m,
            {-0.5f, 0, 17}, {0.5f, 1, 19});
        candidate.pitcher_blockout_height_m = number(table, "pitcher_blockout.height_m", candidate.pitcher_blockout_height_m, 1.25f, 2.5f);
        candidate.batter_blockout_position_m = vector(table, "batter_blockout.position_m", candidate.batter_blockout_position_m,
            {0.75f, 0, -1}, {2, 0.5f, 1});
        candidate.batter_blockout_height_m = number(table, "batter_blockout.height_m", candidate.batter_blockout_height_m, 1.25f, 2.25f);
        candidate.blockout_head_scale = number(table, "character_style.head_scale", candidate.blockout_head_scale, 0.8f, 1.25f);
        candidate.blockout_foot_planar_scale = number(table, "character_style.foot_planar_scale", candidate.blockout_foot_planar_scale, 1, 2.5f);
        candidate.blockout_foot_height_scale = number(table, "character_style.foot_height_scale", candidate.blockout_foot_height_scale, 0.5f, 1.5f);
        candidate.blockout_hand_scale = number(table, "character_style.hand_scale", candidate.blockout_hand_scale, 0.75f, 1.75f);
        candidate.blockout_bat_thickness_scale = number(table, "character_style.bat_thickness_scale", candidate.blockout_bat_thickness_scale, 0.75f, 1.75f);
        candidate.ball_marker_radius_m = number(table, "release.ball_marker_radius_m", candidate.ball_marker_radius_m, 0.03f, 0.15f);
        candidate.grass_half_width_m = number(table, "field.grass_half_width_m", candidate.grass_half_width_m, 40, 120);
        candidate.grass_end_z_m = number(table, "field.grass_end_z_m", candidate.grass_end_z_m, 90, 180);
        candidate.home_dirt_radius_m = number(table, "field.home_dirt_radius_m", candidate.home_dirt_radius_m, 1.5f, 4);
        candidate.mound_visual_dirt_radius_m = number(table, "mound.visual_dirt_radius_m", candidate.mound_visual_dirt_radius_m, 3.5f, 7);
        candidate.mound_height_m = number(table, "mound.height_m", candidate.mound_height_m, 0.125f, 1.0f);
        candidate.mound_radius_m = number(table, "mound.radius_m", candidate.mound_radius_m, 2.2f, 3.5f);
        candidate.mound_top_radius_m = number(table, "mound.top_radius_m", candidate.mound_top_radius_m, 0.7f, 2.0f);
        std::fprintf(stderr, "Staging loaded: %s | owner: pawapuro/batting/staging.cpp | right_handed_pitcher_vs_left_handed_batter\n",
            source.c_str());
        std::fprintf(stderr, "Camera: [%g, %g, %g] -> [%g, %g, %g], vertical FOV %g degrees\n",
            candidate.camera_position_m.x, candidate.camera_position_m.y, candidate.camera_position_m.z,
            candidate.camera_target_m.x, candidate.camera_target_m.y, candidate.camera_target_m.z, candidate.vertical_fov_degrees);
        return candidate;
    } catch (const toml::parse_error& error) {
        std::ostringstream message;
        message << "Staging file: " << source << "\n" << error;
        throw std::runtime_error(message.str());
    } catch (const std::runtime_error& error) {
        throw std::runtime_error("Staging file: " + source + "\n" + error.what());
    }
}
}
