#include "contact_panel.hpp"
#include "startup_text.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <stdexcept>
namespace pawapuro {
ContactPanelState contact_panel_state(const ManualSwingPreview& p) {
    switch(p.gameplay_result()) {
    case GameplayResult::NoSwing:return ContactPanelState::NoSwing;
    case GameplayResult::Miss:return ContactPanelState::Miss;
    case GameplayResult::Contact:return ContactPanelState::Contact;
    default:return p.phase==PreviewPhase::Ready ? ContactPanelState::Ready
        : p.committed() ? ContactPanelState::Swinging : ContactPanelState::Waiting;
    }
}

int contact_panel_highlight(ContactPanelState state) {
    switch(state) {
    case ContactPanelState::NoSwing:return 0;
    case ContactPanelState::Contact:return 1;
    case ContactPanelState::Miss:return 2;
    default:return -1;
    }
}
ContactResultPanel::ContactResultPanel(unsigned width,unsigned height) {
    if(!width||!height)throw std::runtime_error("Contact panel needs client pixels");
    const float scale=float(height)/1080;
    const wchar_t* texts[]={L"本球／最近出棒結果",L"按 Space 開始",L"等待出棒",L"揮棒中",
        contact_panel_labels[0],contact_panel_labels[1],contact_panel_labels[2],
        L"時間重疊＋瞄準授權",L"共同決定擊球是否成立。",
        L"Space：下一球",L"物理重疊僅供開發診斷。",L"A 原節奏",L"B 快出棒",L"下一球：A 原節奏",L"下一球：B 快出棒",
        L"T：下一球前切換",L"揮棒時機：等待",
        L"瞄準授權：等待",L"瞄準授權：通過",L"瞄準授權：超出範圍",
        L"揮棒時機：無重疊（早）",L"揮棒時機：有重疊",L"揮棒時機：無重疊（晚）",L"可再次出棒",L"揮棒中",L"出棒機會已結束",L"出棒已排程",L"等待第一次出棒"};
    std::array<StartupTextMask,28> masks;
    for(unsigned i=0;i<masks.size();++i)masks[i]=raster_startup_text(texts[i],static_cast<int>(std::lround((i==0?30:i>=4&&i<=6?28:20)*scale)));
    for(unsigned variant=0;variant<variants.size()+annotations.size();++variant) {
        auto& v=variant<6?variants[variant]:annotations[variant-6];const auto state=static_cast<ContactPanelState>(variant);
        const int selected=contact_panel_highlight(state);
        const auto point=[&](float x,float y){return DirectX::XMFLOAT3{2*x*scale/float(width)-1,1-2*y*scale/float(height),0};};
        const auto quad=[&](float x,float y,float w,float h,DirectX::XMFLOAT3 color){
            auto a=point(x,y),b=point(x+w,y),c=point(x+w,y+h),d=point(x,y+h);
            v.insert(v.end(),{{a,color},{b,color},{c,color},{a,color},{c,color},{d,color}});
        };
        const auto text=[&](unsigned index,float x,float y,DirectX::XMFLOAT3 color){
            for(const auto& run:masks[index].runs)quad(x+run.x/scale,y+run.y/scale,run.w/scale,1/scale,color);
        };
        const DirectX::XMFLOAT3 white{0.95f,0.96f,1},dark{0.035f,0.06f,0.09f},bright{1,0.9f,0.58f};
        if(variant>=6) {
            const unsigned index=variant-6;
            if(index<6)text(11+index,42,index<2?394.f:index<4?420.f:index==4?446.f:474.f,white);
            if(index>=7&&index<10)text(17+index-7,42,510,bright);
            if(index>=10&&index<13)text(20+index-10,42,474,bright);
            if(index>=13)text(23+index-13,42,76,bright);
            annotation_vertex_count=std::max(annotation_vertex_count,static_cast<unsigned>(v.size()));
            continue;
        }
        quad(24,24,540,526,dark);text(0,42,34,white);
        if(variant<1)text(1+variant,42,76,white);
        for(unsigned row=0;row<3;++row) {
            const float y=110+46*float(row);const bool active=int(row)==selected;
            if(active) {
                quad(38,y-2,510,42,bright);
                v.insert(v.end(),{{point(46,y+10),dark},{point(58,y+18),dark},{point(46,y+26),dark}});
            }
            text(4+row,70,y,active?dark:white);
        }
        text(7,42,256,white);text(8,42,282,white);
        if(state==ContactPanelState::Contact){text(9,42,326,bright);text(10,42,352,bright);}
        vertex_count=std::max(vertex_count,static_cast<unsigned>(v.size()));
    }
    base_vertex_count=vertex_count;
    for(auto& v:variants)v.resize(base_vertex_count);
    for(auto& v:annotations)v.resize(annotation_vertex_count);
    vertex_count=base_vertex_count+6*annotation_vertex_count;
    std::fprintf(stderr,"Contact panel: cached fixed Chinese text, six variants, %u vertices; no per-frame rasterization\n",vertex_count);
}
void ContactResultPanel::append(std::vector<engine::Vertex>& target,const ManualSwingPreview& p) const {
    const auto& vertices=variants.at(static_cast<unsigned>(contact_panel_state(p)));target.insert(target.end(),vertices.begin(),vertices.end());
    const auto add=[&](unsigned index){const auto& v=annotations[index];target.insert(target.end(),v.begin(),v.end());};
    const auto mode=p.phase==PreviewPhase::Ready?p.next_tempo:p.attempt_tempo.mode;
    add(mode==SwingTempo::Original?0:1);
    add(p.phase==PreviewPhase::Complete?(p.next_tempo==SwingTempo::Original?2:3):6);
    add(4);add(!p.timing()?5:p.timing()->state==SwingTimingState::Early?10:p.timing()->state==SwingTimingState::Overlap?11:12);
    add(!p.authorization()?7:p.authorization()->authorized?8:9);
    add(p.phase==PreviewPhase::Ready||p.paused?6:p.active_attempt?14:p.pending?16:p.rearmed()?13:
        p.phase==PreviewPhase::Playing&&p.intent_live(p.tick+p.pending_ticks+1)&&!p.latest()?17:
        p.latest()||p.phase==PreviewPhase::Playing?15:6);
}
}
