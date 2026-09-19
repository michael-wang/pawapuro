#include "staging.hpp"
#include "../startup_window.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
std::array<float, 47> values(const pawapuro::BattingStaging& s)
{
    return {static_cast<float>(s.normal_finish_ticks),s.batting_interaction.half_depth_m,s.swing_phase_potential.normal_start_ms,s.swing_phase_potential.normal_peak_ms,s.swing_phase_potential.normal_end_ms,
        s.camera_position_m.x, s.camera_position_m.y, s.camera_position_m.z,
        s.camera_target_m.x, s.camera_target_m.y, s.camera_target_m.z, s.vertical_fov_degrees,
        s.release_position_m.x, s.release_position_m.y, s.release_position_m.z,
        s.gameplay_ball.radius_m, s.grass_half_width_m, s.grass_end_z_m, s.mound_radius_m, s.mound_top_radius_m, s.home_dirt_radius_m, s.mound_visual_dirt_radius_m, s.mound_height_m,
        s.reference_velocity_mps.x, s.reference_velocity_mps.y, s.reference_velocity_mps.z, s.strike_zone_width_m, s.strike_zone_bottom_m, s.strike_zone_top_m,
        s.pitcher_blockout_position_m.x, s.pitcher_blockout_position_m.y, s.pitcher_blockout_position_m.z, s.pitcher_blockout_height_m,
        s.batter_blockout_position_m.x, s.batter_blockout_position_m.y, s.batter_blockout_position_m.z, s.batter_blockout_height_m, s.blockout_head_scale, s.blockout_foot_planar_scale, s.blockout_foot_height_scale, s.blockout_hand_scale, s.blockout_bat_thickness_scale,s.bat_contact.ball_radius_m,s.bat_contact.bat_radius_m,s.hit_authorization.normal_radius_x_m,s.hit_authorization.normal_radius_y_m,s.player_aim.cursor_speed_mps};
}
}
int main(int argc, char** argv)
{
    try {
        if (argc != 3) throw std::runtime_error("Expected authored TOML and build-local test directory.");
        const std::filesystem::path authored = argv[1], directory = argv[2];
        std::filesystem::create_directories(directory);
        const auto fixture = directory / "candidate.toml";
        const auto write = [&](const char* text) { std::ofstream(fixture) << text << "\n[batter_profile]\ndisplay_name=\"Michael\"\ncontact=75\npower=85\ntrajectory=3\n"; };
        const pawapuro::BattingStaging defaults;
        // Tuning the authored preset must not require changing Native fallback values.
        (void)pawapuro::load_batting_staging(authored);
        const pawapuro::WindowBorders border{48,8,8,8};
        for (const auto area : {pawapuro::WorkArea{0,0,3840,2080}, {0,0,1366,728},
            {0,0,3440,900}, {-1920,40,1920,1000}}) {
            const auto w=pawapuro::fit_startup_window(area,border,0.75);
            const int left=w.x-border.left,top=w.y-border.top;
            if (left<area.x+8 || top<area.y+8 || left+w.w+border.left+border.right>area.x+area.w-8
                || top+w.h+border.top+border.bottom>area.y+area.h-8 || w.w*9!=w.h*16)
                throw std::runtime_error("Startup window fit/aspect failed.");
            if (std::abs(2*(left-area.x)+w.w+border.left+border.right-area.w)>1
                || std::abs(2*(top-area.y)+w.h+border.top+border.bottom-area.h)>1)
                throw std::runtime_error("Outer window not centred in work area.");
        }
        const auto large=pawapuro::fit_startup_window({0,0,3840,2080},border,0.75);
        if (large.w!=2880 || large.h!=1620 || large.fitted)
            throw std::runtime_error("75 percent 4K window differs.");
        write("[window]\nwidth_fraction=1\n");
        if (pawapuro::load_batting_staging(fixture).window_width_fraction!=1)
            throw std::runtime_error("Window override not loaded.");
        write("");
        if (pawapuro::load_batting_staging(fixture).window_width_fraction!=0.75)
            throw std::runtime_error("Window default differs.");
        if (values(pawapuro::load_batting_staging(fixture)) != values(defaults))
            throw std::runtime_error("Missing fields did not use defaults.");
        write("[swing_tempo]\ncompact_area_ticks=29.5\nnormal_finish_ticks=288\n");
        if(pawapuro::load_batting_staging(fixture).compact_area_ticks!=29.5||pawapuro::load_batting_staging(fixture).normal_finish_ticks!=288)
            throw std::runtime_error("Tempo startup override not loaded.");
        write("[batting_interaction]\nhalf_depth_m=0.3\n[swing_phase_potential]\nnormal_start_ms=60\nnormal_peak_ms=120\nnormal_end_ms=200\n");
        const auto timing=pawapuro::load_batting_staging(fixture);
        if(timing.batting_interaction.half_depth_m!=.3f||timing.swing_phase_potential.normal_start_ms!=60
            ||timing.swing_phase_potential.normal_peak_ms!=120||timing.swing_phase_potential.normal_end_ms!=200)
            throw std::runtime_error("Timing startup overrides not loaded.");
        write("[camera]\nvertical_fov_degrees = 42\n");
        if (pawapuro::load_batting_staging(fixture).vertical_fov_degrees != 42)
            throw std::runtime_error("Valid integer override was not applied.");
        write("[field]\nhome_dirt_radius_m = 3.1\n[mound]\nvisual_dirt_radius_m = 6.2\nheight_m = 0.65\ntop_radius_m = 1.5\n");
        const auto dirt = pawapuro::load_batting_staging(fixture);
        if (dirt.home_dirt_radius_m != 3.1f || dirt.mound_visual_dirt_radius_m != 6.2f || dirt.mound_height_m != 0.65f || dirt.mound_top_radius_m != 1.5f)
            throw std::runtime_error("Visual dirt overrides were not applied.");
        write("[camera]\nposition_m = [-0.75, 1.25, -5]\n");
        if (pawapuro::load_batting_staging(fixture).camera_position_m.x != -0.75f)
            throw std::runtime_error("Opposite-side camera override was not applied.");
        write("[reference_pitch]\ninitial_velocity_mps = [1, -1, -40]\n");
        if (pawapuro::load_batting_staging(fixture).reference_velocity_mps.z != -40)
            throw std::runtime_error("Reference velocity override was not applied.");
        write("[strike_zone]\nwidth_m = 0.8636\nbottom_m = 0.6\ntop_m = 1.4\n");
        const auto zone = pawapuro::load_batting_staging(fixture);
        if (zone.strike_zone_width_m != 0.8636f || zone.strike_zone_bottom_m != 0.6f || zone.strike_zone_top_m != 1.4f)
            throw std::runtime_error("Gameplay strike-zone overrides were not applied.");
        for (float width : {0.25f, 0.75f, 0.86f, 0.95f, 1.5f}) {
            const auto text = "[strike_zone]\nwidth_m = " + std::to_string(width) + "\n[hit_authorization]\nnormal_radius_x_m=0.1\n";
            write(text.c_str());
            if (pawapuro::load_batting_staging(fixture).strike_zone_width_m != width)
                throw std::runtime_error("Gameplay width candidate was not loaded.");
        }
        write("[pitcher_blockout]\nposition_m=[0.1,0.3,18]\nheight_m=2.0\n[batter_blockout]\nposition_m=[1.5,0.01,0.2]\nheight_m=1.6\n");
        const auto people = pawapuro::load_batting_staging(fixture);
        if (people.pitcher_blockout_position_m.x != 0.1f || people.pitcher_blockout_height_m != 2
            || people.batter_blockout_position_m.z != 0.2f || people.batter_blockout_height_m != 1.6f)
            throw std::runtime_error("Blockout overrides were not loaded.");
        write("[character_style]\nhead_scale=1.1\nfoot_planar_scale=1.7\nfoot_height_scale=0.8\nhand_scale=1.4\nbat_thickness_scale=1.2\n");
        const auto style = pawapuro::load_batting_staging(fixture);
        if (style.blockout_head_scale != 1.1f || style.blockout_foot_planar_scale != 1.7f || style.blockout_foot_height_scale != 0.8f
            || style.blockout_hand_scale != 1.4f || style.blockout_bat_thickness_scale != 1.2f)
            throw std::runtime_error("Character style overrides were not loaded.");
        write("[bat_contact]\nball_radius_m=0.04\nbat_radius_m=0.03\n");
        const auto contact=pawapuro::load_batting_staging(fixture);
        if(contact.bat_contact.ball_radius_m!=.04f || contact.bat_contact.bat_radius_m!=.03f || contact.gameplay_ball.radius_m!=defaults.gameplay_ball.radius_m)
            throw std::runtime_error("Raw diagnostic envelope mixed with gameplay ball radius.");
        write("[gameplay_ball]\nradius_m=0.1\n");
        const auto gameplay_ball=pawapuro::load_batting_staging(fixture);
        if(gameplay_ball.gameplay_ball.radius_m!=.1f||gameplay_ball.bat_contact.ball_radius_m!=.037f)
            throw std::runtime_error("Gameplay ball override changed raw diagnostic radius.");
        write("[hit_authorization]\nnormal_radius_x_m=0.2\nnormal_radius_y_m=0.1\n[player_aim]\ncursor_speed_mps=0.5\n");
        const auto aim=pawapuro::load_batting_staging(fixture);
        if(aim.hit_authorization.normal_radius_x_m!=.2f||aim.hit_authorization.normal_radius_y_m!=.1f||aim.player_aim.cursor_speed_mps!=.5f)
            throw std::runtime_error("Player aim tuning not loaded.");
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
            {"[batting_interaction]\nhalf_depth_m=0\n", "batting_interaction.half_depth_m"},
            {"[batting_interaction]\nhalf_depth_m=nan\n", "batting_interaction.half_depth_m"},
            {"[batting_interaction]\nhalf_depth_m='deep'\n", "batting_interaction.half_depth_m"},
            {"[batting_interaction]\nx_extent=1\n", "batting_interaction.x_extent"},
            {"[swing_phase_potential]\nnormal_start_ms=125\n", "start < peak < end"},
            {"[swing_phase_potential]\nnormal_peak_ms=190\n", "start < peak < end"},
            {"[swing_phase_potential]\nnormal_end_ms=10\n", "start < peak < end"},
            {"[swing_phase_potential]\nnormal_start_ms=-1\n", "swing_phase_potential.normal_start_ms"},
            {"[swing_phase_potential]\nnormal_peak_ms=inf\n", "swing_phase_potential.normal_peak_ms"},
            {"[swing_phase_potential]\nnormal_end_ms=nan\n", "swing_phase_potential.normal_end_ms"},
            {"[swing_phase_potential]\nnormal_end_ms='late'\n", "swing_phase_potential.normal_end_ms"},
            {"[swing_phase_potential]\ncontact_start_ms=20\n", "swing_phase_potential.contact_start_ms"},
            {"[hit_authorization]\nnormal_radius_x_m='wide'\n", "hit_authorization.normal_radius_x_m"},
            {"[hit_authorization]\nextra=1\n", "hit_authorization.extra"},
            {"hit_authorization=3\n", "hit_authorization must be a table"},
            {"[player_aim]\nnormal_radius_x_m=0.26\n", "player_aim.normal_radius_x_m"},
            {"[swing_tempo]\nnormal_finish_ticks=45\n", "swing_tempo.normal_finish_ticks"}, {"[swing_tempo]\nnormal_finish_ticks=457\n", "swing_tempo.normal_finish_ticks"}, {"[swing_tempo]\nnormal_finish_ticks=240.5\n", "swing_tempo.normal_finish_ticks"}, {"[swing_tempo]\nnormal_finish_ticks=nan\n", "swing_tempo.normal_finish_ticks"}, {"[swing_tempo]\ncompact_area_ticks=0\n", "swing_tempo.compact_area_ticks"},
            {"[swing_tempo]\ncompact_area_ticks=41\n", "swing_tempo.compact_area_ticks"},
            {"[swing_tempo]\ncompact_area_ticks=nan\n", "swing_tempo.compact_area_ticks"},
            {"[swing_tempo]\ncompact_area_ticks='fast'\n", "swing_tempo.compact_area_ticks"},
            {"[window]\nwidth_fraction=0\n", "window.width_fraction"},
            {"[window]\nwidth_fraction=-0.1\n", "window.width_fraction"},
            {"[window]\nwidth_fraction=1.01\n", "window.width_fraction"},
            {"[window]\nwidth_fraction=nan\n", "window.width_fraction"},
            {"[window]\nwidth_fraction=inf\n", "window.width_fraction"},
            {"[window]\nwidth_fraction='large'\n", "window.width_fraction"},
            {"[hit_authorization]\nnormal_radius_x_m=0\n", "hit_authorization.normal_radius_x_m"},
            {"[hit_authorization]\nnormal_radius_y_m=-0.1\n", "hit_authorization.normal_radius_y_m"},
            {"[player_aim]\ncursor_speed_mps=0\n", "player_aim.cursor_speed_mps"},
            {"[hit_authorization]\nnormal_radius_x_m=nan\n", "hit_authorization.normal_radius_x_m"},
            {"[hit_authorization]\nnormal_radius_y_m=inf\n", "hit_authorization.normal_radius_y_m"},
            {"[player_aim]\ncursor_speed_mps=nan\n", "player_aim.cursor_speed_mps"},
            {"[hit_authorization]\nnormal_radius_x_m=1.0\n", "hit_authorization radii"},
            {"[hit_authorization]\nnormal_radius_y_m=1.0\n", "hit_authorization radii"},
            {"[player_aim]\nmode='Power'\n", "player_aim.mode"},
            {"[bat_contact]\nball_radius_m=0\n", "bat_contact.ball_radius_m"},
            {"[bat_contact]\nbat_radius_m=-0.01\n", "bat_contact.bat_radius_m"},
            {"[bat_contact]\nball_radius_m=nan\n", "bat_contact.ball_radius_m"},
            {"[bat_contact]\nbat_radius_m='visual'\n", "bat_contact.bat_radius_m"},
            {"[bat_contact]\nradius=0.07\n", "bat_contact.radius"},
            {"[character_style]\nfoot_height_scale=0\n", "character_style.foot_height_scale"},
            {"[character_style]\nfoot_scale=1.6\n", "character_style.foot_scale"},
            {"[character_style]\nhead_scale=nan\n", "character_style.head_scale"},
            {"[character_style]\nfoot_planar_scale=0\n", "character_style.foot_planar_scale"},
            {"[character_style]\nhand_scale='large'\n", "character_style.hand_scale"},
            {"[character_style]\nbat_thickness_scale=5\n", "character_style.bat_thickness_scale"},
            {"[mound]\nheight_m=0\n", "mound.height_m"},
            {"[mound]\nheight_m=nan\n", "mound.height_m"},
            {"[pitcher_blockout]\nheight_m=0\n", "pitcher_blockout.height_m"},
            {"[pitcher_blockout]\nposition_m=[0,0,0]\n", "pitcher_blockout.position_m[2]"},
            {"[batter_blockout]\nheight_m=nan\n", "batter_blockout.height_m"},
            {"[batter_blockout]\nposition_m=[-1,0,0]\n", "batter_blockout.position_m[0]"},
            {"[strike_zone_reference]\nwidth_m = 0.8636\n", "Unknown staging key: strike_zone_reference"},
            {"[strike_zone]\nwidth_m = 1.6\n", "strike_zone.width_m"},
            {"[strike_zone]\nwidth_m = -1\n", "strike_zone.width_m"},
            {"[strike_zone]\nbottom_m = 'low'\n", "strike_zone.bottom_m"},
            {"[strike_zone]\ntop_m = nan\n", "strike_zone.top_m"},
            {"[strike_zone]\nbottom_m = 1\ntop_m = 0.9\n", "strike_zone.top_m"},
            {"[reference_pitch]\ninitial_velocity_mps = [1, 0, 0]\n", "reference_pitch.initial_velocity_mps[2]"},
            {"[reference_pitch]\ninitial_velocity_mps = [nan, 0, -40]\n", "reference_pitch.initial_velocity_mps[0]"},
            {"[reference_pitch]\ninitial_velocity_mps = [1, 0]\n", "reference_pitch.initial_velocity_mps"},
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
            {"[ground_ball_response]\nmin_rebound_apex_height_m = 0.05\n", "Unknown"},
            {"[ground_ball_response]\nimpact_horizontal_retention = 0.85\n", "Unknown"},
            {"[ground_ball_response]\nroll_deceleration_mps2 = 5.5\n", "Unknown"},
            {"[ground_ball_response]\nrebound_vertical_ratio = 0\n", "ground_ball_response"},
            {"[ground_ball_response]\nrebound_vertical_ratio = inf\n", "ground_ball_response"},
            {"[ground_ball_response]\nground_horizontal_deceleration_mps2 = 1001\n", "ground_ball_response"},
            {"[ground_ball_response]\nrebound_vertical_ratio = 1.0\n", "ground_ball_response"},
            {"[ground_ball_response]\nrebound_vertical_ratio = -0.1\n", "ground_ball_response"},
            {"[ground_ball_response]\nrebound_vertical_ratio = 1.1\n", "ground_ball_response"},
            {"[ground_ball_response]\nrebound_vertical_ratio = nan\n", "ground_ball_response"},
            {"[ground_ball_response]\nfirst_impact_horizontal_retention = 0\n", "ground_ball_response"},
            {"[ground_ball_response]\nfirst_impact_horizontal_retention = -1\n", "ground_ball_response"},
            {"[ground_ball_response]\nfirst_impact_horizontal_retention = 1.1\n", "ground_ball_response"},
            {"[ground_ball_response]\nfirst_impact_horizontal_retention = inf\n", "ground_ball_response"},
            {"[ground_ball_response]\nground_horizontal_deceleration_mps2 = 0\n", "ground_ball_response"},
            {"[ground_ball_response]\nground_horizontal_deceleration_mps2 = -1\n", "ground_ball_response"},
            {"[ground_ball_response]\nground_horizontal_deceleration_mps2 = nan\n", "ground_ball_response"},
            {"[ground_ball_response]\nground_horizontal_deceleration_mps2 = 'x'\n", "ground_ball_response"},
            {"[ground_ball_response]\nunknown = 1\n", "ground_ball_response"},
            {"[unknown]\n", "Unknown staging key"},
            {"[gameplay_ball]\nradius_m = -1\n", "gameplay_ball.radius_m"},
            {"[gameplay_ball]\nradius_m = 0\n", "gameplay_ball.radius_m"},
            {"[gameplay_ball]\nradius_m = 0.201\n", "gameplay_ball.radius_m"},
            {"[gameplay_ball]\nradius_m = nan\n", "gameplay_ball.radius_m"},
            {"[gameplay_ball]\nradius_m = inf\n", "gameplay_ball.radius_m"},
            {"[gameplay_ball]\nradius_m = '0.085'\n", "gameplay_ball.radius_m"},
            {"[gameplay_ball]\nunknown = 1\n", "gameplay_ball.unknown"},
            {"gameplay_ball = 0.085\n", "gameplay_ball"},
            {"[release]\nball_marker_radius_m = 0.085\n", "release.ball_marker_radius_m"},
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
        std::cout << "Defaults, authored preset, valid override, missing file and invalid cases passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
