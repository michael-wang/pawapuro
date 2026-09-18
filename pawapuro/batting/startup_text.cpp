#include "startup_text.hpp"
#include <Windows.h>
#include <cwchar>
#include <stdexcept>
namespace pawapuro {
StartupTextMask raster_startup_text(const wchar_t* text,int size) {
    // GDI is only a startup rasterizer. No system font bytes or GDI objects reach the renderer.
    struct RasterResources {
        HDC dc=CreateCompatibleDC(nullptr); HFONT font=nullptr; HBITMAP bitmap=nullptr;
        HGDIOBJ old_font=nullptr,old_bitmap=nullptr;
        ~RasterResources(){if(dc){if(old_font)SelectObject(dc,old_font);if(old_bitmap)SelectObject(dc,old_bitmap);}
            if(font)DeleteObject(font);if(bitmap)DeleteObject(bitmap);if(dc)DeleteDC(dc);}
    } r;
    if(!r.dc)throw std::runtime_error("Batting text: CreateCompatibleDC failed");
    r.font=CreateFontW(-size,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,CHINESEBIG5_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"Microsoft JhengHei");
    if(!r.font)throw std::runtime_error("Batting text: CreateFontW failed");
    r.old_font=SelectObject(r.dc,r.font);
    const int length=static_cast<int>(std::wcslen(text));SIZE extent{};
    if(!GetTextExtentPoint32W(r.dc,text,length,&extent))throw std::runtime_error("Batting text: text measurement failed");
    const int w=extent.cx+4,h=extent.cy+4;
    BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=w;
    info.bmiHeader.biHeight=-h;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    void* pixels=nullptr;r.bitmap=CreateDIBSection(r.dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);
    if(!r.bitmap)throw std::runtime_error("Batting text: CreateDIBSection failed");
    r.old_bitmap=SelectObject(r.dc,r.bitmap);PatBlt(r.dc,0,0,w,h,BLACKNESS);
    SetTextColor(r.dc,RGB(255,255,255));SetBkColor(r.dc,RGB(0,0,0));
    if(!TextOutW(r.dc,0,0,text,length))throw std::runtime_error("Batting text: TextOutW failed");
    GdiFlush();const auto* data=static_cast<const unsigned*>(pixels);StartupTextMask mask;
    for(int y=0;y<h;++y)for(int x=0;x<w;) {
        if(!(data[y*w+x]&0xffffff)){++x;continue;}
        const int start=x;while(x<w&&(data[y*w+x]&0xffffff))++x;
        mask.runs.push_back({float(start),float(y),float(x-start)});
    }
    if(mask.runs.empty())throw std::runtime_error("Batting text: empty text raster");
    return mask;
}
}
