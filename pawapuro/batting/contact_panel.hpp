#pragma once
#include "manual_swing.hpp"
namespace pawapuro {
enum class ContactPanelState { Ready, Waiting, Swinging, NoSwing, Contact, NoContact, ExtendedContact };
ContactPanelState contact_panel_state(const ManualSwingPreview& preview);
inline constexpr const wchar_t* contact_panel_labels[]={L"未出棒",L"碰到球",L"未測到碰球"};
int contact_panel_highlight(ContactPanelState state);
// Fixed strings and seven complete triangle variants, cached once at startup.
struct ContactResultPanel {
    explicit ContactResultPanel(unsigned pixel_width,unsigned pixel_height);
    std::array<std::vector<engine::Vertex>,7> variants;
    unsigned vertex_count=0;
    void append(std::vector<engine::Vertex>& target,ContactPanelState state) const;
};
}
