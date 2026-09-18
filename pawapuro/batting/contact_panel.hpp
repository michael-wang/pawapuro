#pragma once
#include "manual_swing.hpp"
namespace pawapuro {
enum class ContactPanelState { Ready, Waiting, Swinging, NoSwing, Contact, Miss };
ContactPanelState contact_panel_state(const ManualSwingPreview& preview);
inline constexpr const wchar_t* contact_panel_labels[]={L"未出棒",L"擊球成立",L"揮空"};
int contact_panel_highlight(ContactPanelState state);
// Fixed strings and six complete triangle variants, cached once at startup.
struct ContactResultPanel {
    explicit ContactResultPanel(unsigned pixel_width,unsigned pixel_height);
    std::array<std::vector<engine::Vertex>,6> variants;
    std::array<std::vector<engine::Vertex>,18> annotations;
    unsigned vertex_count=0,base_vertex_count=0,annotation_vertex_count=0;
    void append(std::vector<engine::Vertex>& target,const ManualSwingPreview& preview) const;
};
}
