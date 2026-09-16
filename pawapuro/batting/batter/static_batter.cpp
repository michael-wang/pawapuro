#include "static_batter.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace pawapuro {
StaticBatter load_static_batter(const std::filesystem::path& path, const BattingStaging& staging)
{
    try {
        const auto asset=engine::read_mesh_glb(path);
        const auto require=[](bool ok,const char* reason) { if (!ok) throw std::runtime_error(reason); };
        require(asset.primitives.size()==2 && asset.skin_name=="BatterRig" && asset.joints.size()==17,
            "expected two mesh nodes sharing BatterRig skin (17 joints)");
        const auto node_index=[&](const char* name) {
            std::size_t index=0, count=0;
            for (std::size_t i=0;i<asset.nodes.size();++i) if (asset.nodes[i].name==name) { index=i; ++count; }
            require(count==1,"expected unique hand_R/bat_grip/bat_barrel/bat_tip nodes");
            return index;
        };
        const auto hand=node_index("hand_R"), grip=node_index("bat_grip");
        const auto barrel=node_index("bat_barrel"), tip=node_index("bat_tip");
        require(asset.nodes[grip].parent==static_cast<int>(hand)
            && asset.nodes[barrel].parent==static_cast<int>(grip) && asset.nodes[tip].parent==static_cast<int>(grip),
            "expected hand_R -> bat_grip -> bat_barrel/bat_tip hierarchy");
        const auto placement=staging.batter_blockout_position_m;
        require(std::isfinite(placement.x) && std::isfinite(placement.y) && std::isfinite(placement.z),
            "non-finite staging batter placement");
        const auto place=[&](DirectX::XMFLOAT3 p) -> DirectX::XMFLOAT3 {
            return {p.x+placement.x,p.y+placement.y,p.z+placement.z};
        };
        const auto semantic=[&](std::size_t index) {
            const auto& m=asset.nodes[index].world;
            return place({-m[12],m[13],m[14]});
        };
        StaticBatter out;
        out.local_min={1e9f,1e9f,1e9f}; out.local_max={-1e9f,-1e9f,-1e9f};
        bool body=false,bat=false;
        for (const auto& primitive:asset.primitives) {
            require(primitive.mesh_name==primitive.mesh_node_name,"mesh/node identity mismatch");
            if (primitive.mesh_name=="BatterMesh") { require(!body,"duplicate BatterMesh"); body=true; }
            else if (primitive.mesh_name=="Bat") {
                require(!bat,"duplicate Bat"); bat=true;
                for (const auto& influence:primitive.influences) for (std::size_t j=0;j<4;++j)
                    if (influence.weights[j]>0) require(asset.joints[influence.joints[j]]==grip,
                        "Bat must be rigidly weighted to bat_grip in the shared skin");
            } else require(false,"expected only BatterMesh and Bat");
            for (std::size_t i=0;i<primitive.triangles.size();++i) {
                // Reflect and reverse winding exactly once; preserve vertex/color association.
                const auto reversed=i/3*3+(i%3==0 ? 0 : 3-i%3);
                auto v=primitive.triangles[reversed]; auto& p=v.position; p.x=-p.x;
                out.local_min={std::min(out.local_min.x,p.x),std::min(out.local_min.y,p.y),std::min(out.local_min.z,p.z)};
                out.local_max={std::max(out.local_max.x,p.x),std::max(out.local_max.y,p.y),std::max(out.local_max.z,p.z)};
                p=place(p); out.vertices.push_back(v);
            }
            std::fprintf(stderr,"Static batter primitive: mesh=%s node=%s source_vertices=%zu triangles=%zu expanded=%zu\n",
                primitive.mesh_name.c_str(),primitive.mesh_node_name.c_str(),primitive.source_vertex_count,
                primitive.triangles.size()/3,primitive.triangles.size());
        }
        require(body && bat,"BatterMesh or Bat missing");
        out.world_min=place(out.local_min); out.world_max=place(out.local_max);
        out.grip_world=semantic(grip); out.barrel_world=semantic(barrel); out.tip_world=semantic(tip);
        std::fprintf(stderr,"Static batter: source=%s sha256=%s meshes=2 primitives=2 pose=bind scale=1 placement=[%.6f %.6f %.6f]\n"
            "Batter local bounds=[%.6f %.6f %.6f]..[%.6f %.6f %.6f] world bounds=[%.6f %.6f %.6f]..[%.6f %.6f %.6f]\n"
            "Batter bind bat_grip=[%.6f %.6f %.6f] bat_barrel=[%.6f %.6f %.6f] bat_tip=[%.6f %.6f %.6f]; animation=off\n",
            path.string().c_str(),asset.sha256.c_str(),placement.x,placement.y,placement.z,
            out.local_min.x,out.local_min.y,out.local_min.z,out.local_max.x,out.local_max.y,out.local_max.z,
            out.world_min.x,out.world_min.y,out.world_min.z,out.world_max.x,out.world_max.y,out.world_max.z,
            out.grip_world.x,out.grip_world.y,out.grip_world.z,out.barrel_world.x,out.barrel_world.y,out.barrel_world.z,
            out.tip_world.x,out.tip_world.y,out.tip_world.z);
        return out;
    } catch (const std::exception& error) {
        throw std::runtime_error("Pawapuro static batter owner=pawapuro/batting/batter/static_batter.cpp source='"
            +path.string()+"': "+error.what());
    }
}
}
