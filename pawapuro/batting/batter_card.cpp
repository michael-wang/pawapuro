#include "batter_card.hpp"
#include "startup_text.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace pawapuro {
BatterCard::BatterCard(const BatterProfile& p,unsigned width,unsigned height) {
    if(!width||!height)throw std::runtime_error("Batter card needs client pixels.");
    const int length=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,p.display_name.c_str(),-1,nullptr,0);
    if(length<=1)throw std::runtime_error("Batter card needs a non-empty UTF-8 name.");
    std::wstring name(static_cast<std::size_t>(length),L'\0');
    if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,p.display_name.c_str(),-1,name.data(),length))
        throw std::runtime_error("Batter card name conversion failed.");
    name.pop_back();
    if(p.trajectory<1||p.trajectory>4)throw std::runtime_error("Trajectory must be in [1, 4].");
    lines={name,L"CONTACT   "+std::wstring(1,ability_grade(p.contact)),
        L"POWER     "+std::wstring(1,ability_grade(p.power)),L"彈道        "+std::to_wstring(p.trajectory)};
    const float scale=float(height)/1080;
    const auto point=[&](float x,float y){return DirectX::XMFLOAT3{2*x*scale/float(width)-1,1-2*y*scale/float(height),0};};
    const auto quad=[&](float x,float y,float w,float h,DirectX::XMFLOAT3 color){
        const auto a=point(x,y),b=point(x+w,y),c=point(x+w,y+h),d=point(x,y+h);
        triangles.insert(triangles.end(),{{a,color},{b,color},{c,color},{a,color},{c,color},{d,color}});
    };
    const DirectX::XMFLOAT3 white{.95f,.96f,1},dark{.035f,.06f,.09f},gold{1,.9f,.58f};
    constexpr float top=852;
    quad(24,top,320,204,dark);quad(24,top,4,204,gold);quad(42,top+58,284,1,{.25f,.30f,.35f});
    const auto text=[&](const std::wstring& value,float x,float y,int size,float max_width,DirectX::XMFLOAT3 color){
        const auto mask=raster_startup_text(value.c_str(),static_cast<int>(std::lround(float(size)*scale)));
        float extent=0;for(const auto& run:mask.runs)extent=std::max(extent,run.x+run.w);
        // Keep a longer authored name inside this concrete card.
        const float fit=std::min(1.f,max_width*scale/extent);
        for(const auto& run:mask.runs)quad(x+run.x*fit/scale,y+run.y/scale,run.w*fit/scale,1/scale,color);
    };
    text(lines[0],42,top+12,32,284,white);
    for(unsigned i=1;i<lines.size();++i){
        const float y=top+72+34*float(i-1);
        text(lines[i].substr(0,lines[i].find(L' ')),42,y,24,200,white);
        text(lines[i].substr(lines[i].size()-1),290,y-3,28,36,gold);
    }
}
}
