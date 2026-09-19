#pragma once
#include "manual_swing.hpp"
#include "player_aim.hpp"
#include <charconv>
#include <string_view>

namespace pawapuro {
// Development intent only. Decisions and flight remain owned by ManualSwingPreview.
struct BattingReviewFixture {
    std::uint64_t commit_tick;
    float ex,ey;
    DirectX::XMFLOAT2 aim_center(const ManualSwingPreview& preview) const {
        const auto p=predict_arrival(preview.delivery.pitch).state.position_m;
        const auto region=normal_authorization_region(preview.tuning);
        const float radius=preview.tuning.gameplay_ball.radius_m;
        return {p.x-ex*(region.normal_radius_x_m+radius),p.y-ey*(region.normal_radius_y_m+radius)};
    }
    bool start(ManualSwingPreview& preview,PlayerAim& aim) const {
        if(preview.phase==PreviewPhase::Playing&&!preview.can_restart_after_contact())return false;
        preview.next_tempo=SwingTempo::Compact;
        if(!preview.start())return false;
        const auto center=aim_center(preview);
        if(!aim.set_center(center))throw std::runtime_error("Batting fixture aim is outside PlayerAim bounds or non-finite.");
        if(!preview.record_command(commit_tick,center))throw std::runtime_error("Batting fixture commit tick cannot be consumed during this pitch.");
        return true;
    }
    // Unlike an OS key edge, recorded fixture intent survives pause/focus loss.
    static void toggle_pause(ManualSwingPreview& preview) {
        if(preview.phase==PreviewPhase::Playing)preview.paused=!preview.paused;
    }
    void verify(const SwingAttempt& attempt) const {
        const auto actual=attempt.authorization.normalized_error;
        if(attempt.command.consumed_tick!=commit_tick||std::abs(actual.x-ex)>1e-6f||std::abs(actual.y-ey)>1e-6f)
            throw std::runtime_error("Batting fixture consumed intent differs from request.");
    }
};
struct BattingStartupOptions {
    std::optional<std::string_view> staging_path;
    std::optional<BattingReviewFixture> fixture;
};
inline BattingStartupOptions parse_batting_options(int argc,const char* const* argv) {
    constexpr const char* usage="Usage: pawapuro [--staging <path>] [--batting-fixture <uint64 commit_tick> <finite ex> <finite ey>]";
    BattingStartupOptions result;
    const auto scalar=[&](const char* text) {
        std::string_view s(text);
        if(!s.empty()&&s.front()=='+') {
            s.remove_prefix(1);
            if(!s.empty()&&s.front()=='-')throw std::runtime_error(usage);
        }
        float value=0;
        const auto parsed=std::from_chars(s.data(),s.data()+s.size(),value);
        if(s.empty()||parsed.ec!=std::errc{}||parsed.ptr!=s.data()+s.size()||!std::isfinite(value))throw std::runtime_error(usage);
        return value;
    };
    for(int i=1;i<argc;++i) {
        const std::string_view option(argv[i]);
        if(option=="--staging"&&!result.staging_path&&i+1<argc) {
            result.staging_path=argv[++i];
            if(result.staging_path->empty()||result.staging_path->starts_with("--"))throw std::runtime_error(usage);
        } else if(option=="--batting-fixture"&&!result.fixture&&i+3<argc) {
            const std::string_view tick(argv[++i]);std::uint64_t value=0;
            const auto parsed=std::from_chars(tick.data(),tick.data()+tick.size(),value);
            if(tick.empty()||parsed.ec!=std::errc{}||parsed.ptr!=tick.data()+tick.size())throw std::runtime_error(usage);
            const float ex=scalar(argv[++i]),ey=scalar(argv[++i]);
            result.fixture=BattingReviewFixture{value,ex,ey};
        } else throw std::runtime_error(usage);
    }
    return result;
}
}
