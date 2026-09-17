#include "contact_panel.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace pawapuro;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char**argv){try {
    require(argc==2,"expected batting directory");const std::filesystem::path d=argv[1];
    ManualSwingPreview p(d,load_batting_staging(d/"staging.toml"));
    require(contact_panel_state(p)==ContactPanelState::Ready,"Ready prompt");
    for(auto state:{ContactPanelState::Ready,ContactPanelState::Waiting,ContactPanelState::Swinging})
        require(contact_panel_highlight(state)==-1,"Pending highlighted a final result");
    for(unsigned height:{1080u,1620u}) {
        ContactResultPanel panel(height*16/9,height);
        require(panel.vertex_count>0&&panel.vertex_count%3==0,"invalid panel capacity");
        for(unsigned index=0;index<7;++index) {
            const auto state=static_cast<ContactPanelState>(index);const auto& mesh=panel.variants[index];
            require(mesh.size()==panel.vertex_count,"variant capacity differs");
            unsigned highlight_quads=0;
            for(size_t i=0;i<mesh.size();i+=3) {
                const auto& a=mesh[i];const auto& b=mesh[i+1];const auto& c=mesh[i+2];
                require(std::isfinite(a.position.x)&&std::isfinite(a.position.y),"nonfinite panel");
                // Only the selected row has a wide bright rectangle; glyph runs are narrow.
                if(a.color.x==1 && a.color.y==0.9f && std::abs(a.position.x-b.position.x)>0.4f
                    && std::abs(b.position.y-c.position.y)>0.03f)++highlight_quads;
            }
            require(highlight_quads==(contact_panel_highlight(state)>=0?1u:0u),"not exactly one selected row");
        }
    }
    for(std::uint64_t commit:{0ull,432ull,441ull,456ull}) {
        p.reset();p.start();require(contact_panel_state(p)==ContactPanelState::Waiting,"waiting prompt");
        if(commit)p.record_command(commit,{0,0.775f});
        require(contact_panel_state(p)==ContactPanelState::Waiting,"queued input treated as consumed");
        while(p.phase==PreviewPhase::Playing) {
            p.advance(4'166'667);
            const auto tick=p.tick;const auto pose=p.batter.pose.world;const auto ball=p.delivery.pitch.current;
            const auto state=contact_panel_state(p);
            require(tick==p.tick&&pose==p.batter.pose.world&&ball.position_m.z==p.delivery.pitch.current.position_m.z,"UI mutated simulation");
            if(p.committed&&p.geometry==ManualGeometry::Pending) {
                require(state==ContactPanelState::Swinging,"swinging prompt");
                p.toggle_pause();p.advance(100'000'000);
                require(p.tick==tick&&contact_panel_state(p)==state,"pause changed panel state or time");
                p.toggle_pause();
            }
        }
        const auto expected=commit==0?ContactPanelState::NoSwing:commit==432?ContactPanelState::Contact:
            commit==441?ContactPanelState::ExtendedContact:ContactPanelState::NoContact;
        require(contact_panel_state(p)==expected,"owner result mapping/analytic extension differs");
        const auto result=contact_panel_state(p);p.advance(9'000'000'000);p.toggle_pause();
        require(contact_panel_state(p)==result,"final result did not persist");
        require(contact_panel_highlight(result)==(commit==0?0:commit==456?2:1),"wrong result row");
        p.reset();require(contact_panel_state(p)==ContactPanelState::Ready&&!p.contact,"reset retained highlight");
    }
    std::cout<<"PASS panel: pending, three owner results, analytic extension, persistence/reset, 1080/1620 cached geometry\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
