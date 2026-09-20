#include "contact_panel.hpp"
#include "startup_text.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>
namespace pawapuro {
BattingInfo batting_info(const ManualSwingPreview& p) {
    BattingInfo result;
    const auto& ball=p.ball_passage;
    // Fixed for this pitch, never fitted to the latest swing; extremes clamp at the track ends.
    const auto& potential=p.tuning.swing_phase_potential;
    const double decision_start=ball.enter_s-potential.normal_end_ms/1000.;
    const double decision_end=ball.exit_s-potential.normal_start_ms/1000.;
    const double duration=decision_end-decision_start;
    if(!std::isfinite(duration)||duration<=0)throw std::runtime_error("Batting timeline needs positive finite decision duration");
    const double display_start=decision_start-.25*duration,display_end=decision_end+.25*duration;
    const auto position=[&](double time){return std::clamp((time-display_start)/(display_end-display_start),0.,1.);};
    result.timeline.decision_start=position(decision_start);result.timeline.decision_end=position(decision_end);
    if(const auto* timing=p.timing()) {
        result.offset_ms=timing->offset_ms;
        const bool contact=p.latest()->gameplay==GameplayResult::Contact;
        result.timeline.marker=contact?TimingMarker::Contact:TimingMarker::SwingWithoutContact;
        result.timeline.marker_position=position(double(p.committed()->consumed_tick)/pitch_hz);
    }
    if(p.flight) {
        result.exit_speed_kmh=double(p.latest()->response->exit_speed_mps)*3.6;
        const auto& f=*p.flight;
        // First ground hit is currently the only collision. Future hits must supply their actual stop time.
        const double time=std::clamp(double(p.tick)/pitch_hz,f.start_s,f.first_hit_s());
        const auto current=f.sample(time).position_m;
        const double elapsed=time-f.start_s;
        const double peak_elapsed=std::clamp(-double(f.initial.velocity_mps.y)/earth_gravity_mps2,0.,elapsed);
        const auto peak=f.sample(f.start_s+peak_elapsed).position_m;
        result.flight=BattingFlightMetrics{elapsed,
            std::hypot(double(current.x)-f.initial.position_m.x,double(current.z)-f.initial.position_m.z),
            std::max(0.,double(peak.y)-f.ground_height_m)};
    }
    return result;
}
BattingInfoText format_batting_info(const BattingInfo& info) {
    BattingInfoText text{};
    for(auto& row:text)std::snprintf(row.data(),row.size(),"--");
    const auto number=[&](unsigned row,const char* format,double value){
        const int size=std::snprintf(text[row].data(),text[row].size(),format,value);
        if(size<0||size>=int(text[row].size()))std::snprintf(text[row].data(),text[row].size(),"--");
    };
    if(info.offset_ms)number(0,"%+.1f ms",*info.offset_ms);
    if(info.flight){number(1,"%.2f s",info.flight->elapsed_s);number(2,"%.1f m",info.flight->distance_m);number(3,"%.1f m",info.flight->max_height_m);}
    if(info.exit_speed_kmh)number(4,"%.1f km/h",*info.exit_speed_kmh);
    return text;
}
namespace {
constexpr char glyph_characters[]="0123456789+-.smk/h";
constexpr float timeline_x=232,timeline_top=100,timeline_height=144;
}
ContactResultPanel::ContactResultPanel(unsigned width,unsigned height) {
    if(!width||!height)throw std::runtime_error("Batting info needs client pixels");
    const float scale=float(height)/1080;
    x_scale=2*scale/float(width);y_scale=2*scale/float(height);
    const auto point=[&](float x,float y){return DirectX::XMFLOAT3{x*x_scale-1,1-y*y_scale,0};};
    const auto quad=[&](std::vector<engine::Vertex>& v,float x,float y,float w,float h,DirectX::XMFLOAT3 color){
        const auto a=point(x,y),b=point(x+w,y),c=point(x+w,y+h),d=point(x,y+h);
        v.insert(v.end(),{{a,color},{b,color},{c,color},{a,color},{c,color},{d,color}});
    };
    const auto text=[&](std::vector<engine::Vertex>& v,const wchar_t* value,float x,float y,int size,DirectX::XMFLOAT3 color){
        const auto mask=raster_startup_text(value,static_cast<int>(std::lround(size*scale)));
        for(const auto& run:mask.runs)quad(v,x+run.x/scale,y+run.y/scale,run.w/scale,1/scale,color);
    };
    const DirectX::XMFLOAT3 white{.95f,.96f,1},muted{.66f,.71f,.76f},gold{1,.9f,.58f},dark{.035f,.06f,.09f};
    quad(base,24,24,500,460,dark);quad(base,24,24,4,460,gold);
    text(base,L"打擊資訊",42,34,30,white);quad(base,42,72,464,1,{.25f,.30f,.35f});
    for(unsigned row=0;row<5;++row)text(base,batting_info_labels[row],42,84+48*float(row)+(row?140:0),24,white);
    quad(base,timeline_x-1,timeline_top,2,timeline_height,muted);
    text(base,L"早",258,90,18,muted);text(base,L"晚",258,230,18,muted);
    // Two tiny local baseball variants, cached with the rest of this concrete panel.
    for(unsigned filled=0;filled<2;++filled){
        auto& vertices=timing_markers[filled];const auto seam=filled?DirectX::XMFLOAT3{.8f,.25f,.25f}:muted;
        for(unsigned i=0;i<24;++i){
            const float a=DirectX::XM_2PI*float(i)/24,b=DirectX::XM_2PI*float(i+1)/24;
            const auto outer_a=point(8*std::cos(a),8*std::sin(a)),outer_b=point(8*std::cos(b),8*std::sin(b));
            if(filled)vertices.insert(vertices.end(),{{point(0,0),white},{outer_a,white},{outer_b,white}});
            else {const auto inner_a=point(6.5f*std::cos(a),6.5f*std::sin(a)),inner_b=point(6.5f*std::cos(b),6.5f*std::sin(b));
                vertices.insert(vertices.end(),{{outer_a,muted},{outer_b,muted},{inner_b,muted},{outer_a,muted},{inner_b,muted},{inner_a,muted}});}
        }
        for(float side:{-1.f,1.f})for(unsigned i=0;i<8;++i){
            const float y=-5+1.25f*float(i),next_y=y+1.25f;
            const float x=side*(2+2*y*y/25),next_x=side*(2+2*next_y*next_y/25);
            const auto a=point(x-.5f,y),b=point(x+.5f,y),c=point(next_x+.5f,next_y),d=point(next_x-.5f,next_y);
            vertices.insert(vertices.end(),{{a,seam},{b,seam},{c,seam},{a,seam},{c,seam},{d,seam}});
        }
    }
    // Small concrete helper area; lower-left Batter Profile remains independent.
    const float helper_x=float(width)/scale-350;
    quad(base,helper_x,24,326,112,dark);
    text(base,L"Space：開始／下一球",helper_x+14,34,18,muted);
    text(base,L"T：下一球前切換",helper_x+14,84,18,muted);
    text(base,L"P：暫停　.／Shift+.：步進",helper_x+14,109,16,muted);
    text(tempo_text[0],L"A 原節奏",helper_x+14,59,18,muted);
    text(tempo_text[1],L"B 快出棒",helper_x+14,59,18,muted);
    for(unsigned i=0;i<sizeof(glyph_characters)-1;++i){
        const wchar_t value[]{wchar_t(glyph_characters[i]),0};text(glyphs[i],value,0,0,26,gold);
        float extent=0;for(const auto& v:glyphs[i])extent=std::max(extent,v.position.x+1);
        const float fit=std::min(1.f,14*x_scale/extent);
        for(auto& v:glyphs[i])v.position.x=-1+(v.position.x+1)*fit;
    }
    // Last slot is a blank; every slot/state has an identical triangle count.
    const auto pad=[](auto& variants){std::size_t count=0;for(const auto& v:variants)count=std::max(count,v.size());for(auto& v:variants)v.resize(count);};
    pad(timing_markers);pad(tempo_text);pad(glyphs);
    vertex_count=static_cast<unsigned>(base.size()+6+timing_markers[0].size()+tempo_text[0].size()+5*16*glyphs[0].size());
}
void ContactResultPanel::append(std::vector<engine::Vertex>& target,const ManualSwingPreview& p) const {
    const auto info=batting_info(p);const auto values=format_batting_info(info);
    const auto add=[&](const auto& vertices){target.insert(target.end(),vertices.begin(),vertices.end());};
    add(base);
    const float top=timeline_top+timeline_height*float(info.timeline.decision_start),bottom=timeline_top+timeline_height*float(info.timeline.decision_end);
    const auto point=[&](float x,float y){return DirectX::XMFLOAT3{x*x_scale-1,1-y*y_scale,0};};
    const DirectX::XMFLOAT3 yellow{1,.85f,.2f};
    const auto band_a=point(timeline_x-3,top),band_b=point(timeline_x+3,top),band_c=point(timeline_x+3,bottom),band_d=point(timeline_x-3,bottom);
    target.insert(target.end(),{{band_a,yellow},{band_b,yellow},{band_c,yellow},{band_a,yellow},{band_c,yellow},{band_d,yellow}});
    if(info.timeline.marker==TimingMarker::None)target.resize(target.size()+timing_markers[0].size());
    else for(auto vertex:timing_markers[info.timeline.marker==TimingMarker::Contact?1:0]){
        vertex.position.x+=timeline_x*x_scale;
        vertex.position.y-=(timeline_top+timeline_height*float(info.timeline.marker_position))*y_scale;
        target.push_back(vertex);
    }
    const auto tempo=p.phase==PreviewPhase::Ready||p.phase==PreviewPhase::Complete?p.next_tempo:p.attempt_tempo.mode;
    add(tempo_text[tempo==SwingTempo::Original?0:1]);
    for(unsigned row=0;row<5;++row)for(unsigned slot=0;slot<16;++slot){
        const char c=values[row][slot];const char* found=c?std::strchr(glyph_characters,c):nullptr;
        const auto index=found?static_cast<std::size_t>(found-glyph_characters):glyphs.size()-1;
        const float text_scale=row==0?.75f:1.f;
        const float x=(row==0?286.f:202.f)+16*text_scale*float(slot),y=row==0?158:222+48*float(row);
        for(auto vertex:glyphs[index]){vertex.position.x=-1+(vertex.position.x+1)*text_scale+x*x_scale;vertex.position.y=1+(vertex.position.y-1)*text_scale-y*y_scale;target.push_back(vertex);}
    }
}
}
