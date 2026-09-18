#include "contact_panel.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
using namespace pawapuro;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char**argv){try {
    require(argc==2,"expected batting directory");const std::filesystem::path d=argv[1];
    ManualSwingPreview p(d,load_batting_staging(d/"staging.toml"));
    require(contact_panel_state(p)==ContactPanelState::Ready&&p.next_tempo==SwingTempo::Compact,"Ready default B");
    for(auto state:{ContactPanelState::Ready,ContactPanelState::Waiting,ContactPanelState::Swinging})
        require(contact_panel_highlight(state)==-1,"Pending highlighted a final result");
    for(unsigned height:{1080u,1620u}) {
        ContactResultPanel panel(height*16/9,height);
        require(panel.vertex_count>0&&panel.vertex_count%3==0,"invalid panel capacity");
        std::vector<engine::Vertex> drawn;
        const auto check_annotation=[&](unsigned slot,unsigned expected){
            const auto offset=panel.base_vertex_count+slot*panel.annotation_vertex_count;
            require(std::memcmp(drawn.data()+offset,panel.annotations[expected].data(),panel.annotation_vertex_count*sizeof(engine::Vertex))==0,"wrong panel annotation");
        };
        p.reset();p.next_tempo=SwingTempo::Compact;panel.append(drawn,p);require(drawn.size()==panel.vertex_count,"append capacity");check_annotation(0,1);check_annotation(3,6);check_annotation(4,7);
        for(bool authorized:{true,false})for(auto geometry:{ManualGeometry::Pending,ManualGeometry::Contact,ManualGeometry::NoContactInWindow}) {
            p.authorization=HitAuthorizationDecision{authorized,{},{},{},authorized?0.f:2.f};p.geometry=geometry;
            drawn.clear();panel.append(drawn,p);check_annotation(4,authorized?8:9);
            require(contact_panel_highlight(contact_panel_state(p))==(geometry==ManualGeometry::Contact?1:geometry==ManualGeometry::NoContactInWindow?2:-1),"authorization altered geometry row");
        }
        p.reset();
        p.toggle_tempo();drawn.clear();panel.append(drawn,p);check_annotation(0,0);
        p.toggle_tempo();drawn.clear();panel.append(drawn,p);check_annotation(0,1);
        p.start();p.input_boundary(false,false,true,{});p.input_boundary(true,true,true,{});
        drawn.clear();panel.append(drawn,p);check_annotation(3,5);
        while(p.phase==PreviewPhase::Playing)p.advance(16'666'667);
        p.toggle_tempo();drawn.clear();panel.append(drawn,p);check_annotation(0,1);check_annotation(1,2);check_annotation(3,5);
        p.reset();drawn.clear();panel.append(drawn,p);check_annotation(0,0);check_annotation(3,6);

        for(unsigned index=0;index<7;++index) {
            const auto state=static_cast<ContactPanelState>(index);const auto& mesh=panel.variants[index];
            require(mesh.size()==panel.base_vertex_count,"variant capacity differs");
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
    p.next_tempo=SwingTempo::Original; // Retain original geometry fixtures.
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
