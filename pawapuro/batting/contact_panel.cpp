#include "contact_panel.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <stdexcept>
namespace pawapuro {
ContactPanelState contact_panel_state(const ManualSwingPreview& p) {
    switch(p.geometry) {
    case ManualGeometry::NoSwing:return ContactPanelState::NoSwing;
    case ManualGeometry::NoContactInWindow:return ContactPanelState::NoContact;
    case ManualGeometry::Contact:
        return p.contact && p.contact->sample.ball.position_m.z<p.delivery.pitch.evaluation_plane_z
            ? ContactPanelState::ExtendedContact : ContactPanelState::Contact;
    default:return p.phase==PreviewPhase::Ready ? ContactPanelState::Ready
        : p.committed ? ContactPanelState::Swinging : ContactPanelState::Waiting;
    }
}
int contact_panel_highlight(ContactPanelState state) {
    switch(state) {
    case ContactPanelState::NoSwing:return 0;
    case ContactPanelState::Contact:case ContactPanelState::ExtendedContact:return 1;
    case ContactPanelState::NoContact:return 2;
    default:return -1;
    }
}
namespace {
struct TextMask {
    struct Run {float x,y,w;};
    std::vector<Run> runs;
};
TextMask raster(const wchar_t* text,int size) {
    // GDI is only a startup rasterizer. No system font bytes or GDI objects reach the renderer.
    struct RasterResources {
        HDC dc=CreateCompatibleDC(nullptr); HFONT font=nullptr; HBITMAP bitmap=nullptr;
        HGDIOBJ old_font=nullptr,old_bitmap=nullptr;
        ~RasterResources(){if(dc){if(old_font)SelectObject(dc,old_font);if(old_bitmap)SelectObject(dc,old_bitmap);}
            if(font)DeleteObject(font);if(bitmap)DeleteObject(bitmap);if(dc)DeleteDC(dc);}
    } r;
    if(!r.dc)throw std::runtime_error("Contact panel: CreateCompatibleDC failed");
    r.font=CreateFontW(-size,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,CHINESEBIG5_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"Microsoft JhengHei");
    if(!r.font)throw std::runtime_error("Contact panel: CreateFontW failed");
    r.old_font=SelectObject(r.dc,r.font);
    const int length=static_cast<int>(std::wcslen(text));SIZE extent{};
    if(!GetTextExtentPoint32W(r.dc,text,length,&extent))throw std::runtime_error("Contact panel: text measurement failed");
    const int w=extent.cx+4,h=extent.cy+4;
    BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=w;
    info.bmiHeader.biHeight=-h;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    void* pixels=nullptr;r.bitmap=CreateDIBSection(r.dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);
    if(!r.bitmap)throw std::runtime_error("Contact panel: CreateDIBSection failed");
    r.old_bitmap=SelectObject(r.dc,r.bitmap);PatBlt(r.dc,0,0,w,h,BLACKNESS);
    SetTextColor(r.dc,RGB(255,255,255));SetBkColor(r.dc,RGB(0,0,0));
    if(!TextOutW(r.dc,0,0,text,length))throw std::runtime_error("Contact panel: TextOutW failed");
    GdiFlush();const auto* data=static_cast<const unsigned*>(pixels);TextMask mask;
    for(int y=0;y<h;++y)for(int x=0;x<w;) {
        if(!(data[y*w+x]&0xffffff)){++x;continue;}
        const int start=x;while(x<w&&(data[y*w+x]&0xffffff))++x;
        mask.runs.push_back({float(start),float(y),float(x-start)});
    }
    if(mask.runs.empty())throw std::runtime_error("Contact panel: empty text raster");
    return mask;
}
}
ContactResultPanel::ContactResultPanel(unsigned width,unsigned height) {
    if(!width||!height)throw std::runtime_error("Contact panel needs client pixels");
    const float scale=float(height)/1080;
    const wchar_t* texts[]={L"本球結果",L"按 Space 開始",L"等待出棒",L"揮棒中",
        contact_panel_labels[0],contact_panel_labels[1],contact_panel_labels[2],
        L"接觸測試：只檢查出棒後的一小段，",L"球暫時不會飛出去。",
        L"依球繼續前進的位置判斷；",L"畫面中的球仍停住。",L"A 原節奏",L"B 快出棒",L"下一球：A 原節奏",L"下一球：B 快出棒",
        L"T：下一球前切換",L"已按下，但不在本輪可出棒時段。"};
    std::array<TextMask,17> masks;
    for(unsigned i=0;i<masks.size();++i)masks[i]=raster(texts[i],static_cast<int>(std::lround((i==0?30:i>=4&&i<=6?28:20)*scale)));
    for(unsigned variant=0;variant<variants.size()+annotations.size();++variant) {
        auto& v=variant<7?variants[variant]:annotations[variant-7];const auto state=static_cast<ContactPanelState>(variant);
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
        if(variant>=7) {
            const unsigned index=variant-7;
            if(index<6)text(11+index,42,index<2?394.f:index<4?420.f:index==4?446.f:474.f,white);
            annotation_vertex_count=std::max(annotation_vertex_count,static_cast<unsigned>(v.size()));
            continue;
        }
        quad(24,24,540,510,dark);text(0,42,34,white);
        if(variant<3)text(1+variant,42,76,white);
        for(unsigned row=0;row<3;++row) {
            const float y=110+46*float(row);const bool active=int(row)==selected;
            if(active) {
                quad(38,y-2,510,42,bright);
                v.insert(v.end(),{{point(46,y+10),dark},{point(58,y+18),dark},{point(46,y+26),dark}});
            }
            text(4+row,70,y,active?dark:white);
        }
        text(7,42,256,white);text(8,42,282,white);
        if(state==ContactPanelState::ExtendedContact){text(9,42,326,bright);text(10,42,352,bright);}
        vertex_count=std::max(vertex_count,static_cast<unsigned>(v.size()));
    }
    base_vertex_count=vertex_count;
    for(auto& v:variants)v.resize(base_vertex_count);
    for(auto& v:annotations)v.resize(annotation_vertex_count);
    vertex_count=base_vertex_count+4*annotation_vertex_count;
    std::fprintf(stderr,"Contact panel: cached fixed Chinese text, seven variants, %u vertices; no per-frame rasterization\n",vertex_count);
}
void ContactResultPanel::append(std::vector<engine::Vertex>& target,const ManualSwingPreview& p) const {
    const auto& vertices=variants.at(static_cast<unsigned>(contact_panel_state(p)));target.insert(target.end(),vertices.begin(),vertices.end());
    const auto add=[&](unsigned index){const auto& v=annotations[index];target.insert(target.end(),v.begin(),v.end());};
    const auto mode=p.phase==PreviewPhase::Ready?p.next_tempo:p.attempt_tempo.mode;
    add(mode==SwingTempo::Original?0:1);
    add(p.phase==PreviewPhase::Complete?(p.next_tempo==SwingTempo::Original?2:3):6);
    add(4);add(p.domain_rejected?5:6);
}
}
