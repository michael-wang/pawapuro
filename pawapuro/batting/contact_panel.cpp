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
    if(const auto* timing=p.timing()) {
        result.offset_ms=timing->offset_ms;
        // Display candidate only, independent of temporal overlap and gameplay acceptance.
        result.timing=std::abs(timing->offset_ms)<=10 ? InfoTiming::Center
            : timing->offset_ms<0 ? InfoTiming::Early : InfoTiming::Late;
    }
    if(p.flight) {
        const auto& f=*p.flight;
        // First ground hit is currently the only collision. Future hits must supply their actual stop time.
        const double time=std::clamp(double(p.tick)/pitch_hz,f.start_s,f.ground_s);
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
    return text;
}
namespace { constexpr char glyph_characters[]="0123456789+-.sm"; }
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
    quad(base,24,24,500,272,dark);quad(base,24,24,4,272,gold);
    text(base,L"打擊資訊",42,34,30,white);quad(base,42,72,464,1,{.25f,.30f,.35f});
    for(unsigned row=0;row<4;++row)text(base,batting_info_labels[row],42,84+48*float(row),24,white);
    for(unsigned i=0;i<4;++i)text(timing_text[i],info_timing_labels[i],202,82,28,i==2?gold:white);
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
    pad(timing_text);pad(tempo_text);pad(glyphs);
    vertex_count=static_cast<unsigned>(base.size()+timing_text[0].size()+tempo_text[0].size()+4*16*glyphs[0].size());
}
void ContactResultPanel::append(std::vector<engine::Vertex>& target,const ManualSwingPreview& p) const {
    const auto info=batting_info(p);const auto values=format_batting_info(info);
    const auto add=[&](const auto& vertices){target.insert(target.end(),vertices.begin(),vertices.end());};
    add(base);add(timing_text[static_cast<unsigned>(info.timing)]);
    const auto tempo=p.phase==PreviewPhase::Ready||p.phase==PreviewPhase::Complete?p.next_tempo:p.attempt_tempo.mode;
    add(tempo_text[tempo==SwingTempo::Original?0:1]);
    for(unsigned row=0;row<4;++row)for(unsigned slot=0;slot<16;++slot){
        const char c=row==0&&!info.offset_ms?' ':values[row][slot];const char* found=c?std::strchr(glyph_characters,c):nullptr;
        const auto index=found?static_cast<std::size_t>(found-glyph_characters):glyphs.size()-1;
        const float x=(row==0?296.f:202.f)+16*float(slot),y=82+48*float(row);
        for(auto vertex:glyphs[index]){vertex.position.x+=x*x_scale;vertex.position.y-=y*y_scale;target.push_back(vertex);}
    }
}
}
