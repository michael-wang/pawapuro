#include "static_batter.hpp"
#include "pawapuro/batting/reference_scene.hpp"
#include <cmath>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace {
void require(bool ok,const char* why) { if (!ok) throw std::runtime_error(why); }
void check_near(DirectX::XMFLOAT3 a,DirectX::XMFLOAT3 b) {
    require(std::abs(a.x-b.x)<1e-5f && std::abs(a.y-b.y)<1e-5f && std::abs(a.z-b.z)<1e-5f,"position/color mismatch (1e-5)");
}
}
int main(int argc,char** argv)
{
    try {
        require(argc==4,"expected GLB, staging, temporary fixture directory");
        const std::filesystem::path source=argv[1], temporary=argv[3];
        const auto staging=pawapuro::load_batting_staging(argv[2]);
        const auto asset=engine::read_mesh_glb(source);
        require(asset.primitives.size()==2 && asset.nodes.size()==20 && asset.joints.size()==17,"shared asset contract");
        require(asset.sha256=="d5ec26c3cc2d607b8bd8b1b891aaa5cf1c92eb9a03585f6b40a113f7d8ba623d","accepted asset changed");
        const auto batter=pawapuro::load_static_batter(source,staging);
        require(batter.vertices.size()==14304,"combined expanded count");
        check_near(batter.local_min,{-.519f,0,-.795799673f}); check_near(batter.local_max,{.468f,2.182591915f,.698f});
        const auto p=staging.batter_blockout_position_m;
        check_near(batter.world_min,{p.x-.519f,p.y,p.z-.795799673f});
        check_near(batter.world_max,{p.x+.468f,p.y+2.182591915f,p.z+.698f});
        // Independent source rest construction: grip plus normalized bat axis times semantic distance.
        check_near(batter.grip_world,{p.x-.42f,p.y+1.04f,p.z-.36f});
        const float length=std::sqrt(.15f*.15f+.94f*.94f+.32f*.32f);
        const auto expected=[&](float d) -> DirectX::XMFLOAT3 {
            return {p.x-.42f+d*.15f/length,p.y+1.04f+d*.94f/length,p.z-.36f-d*.32f/length};
        };
        check_near(batter.barrel_world,expected(.86f)); check_near(batter.tip_world,expected(1.2f));
        require(batter.tip_world.z<batter.grip_world.z && batter.grip_world.x<p.x,"bat must remain on rear/plate side");
        std::size_t offset=0;
        for (const auto& primitive:asset.primitives) {
            const bool body=primitive.mesh_name=="BatterMesh";
            require(primitive.mesh_node_name==primitive.mesh_name && asset.nodes[primitive.mesh_node].name==primitive.mesh_name,"mesh node ownership");
            require(primitive.source_vertex_count==(body?2608:142) && primitive.indices.size()==(body?13464:840),"per primitive count");
            require(primitive.influences.size()==primitive.bind_vertices.size(),"influence count");
            for (std::size_t i=0;i<primitive.triangles.size();++i) {
                auto expected_vertex=primitive.triangles[i/3*3+(i%3==0?0:3-i%3)];
                const auto v=expected_vertex.position;
                check_near(batter.vertices[offset+i].position,{-v.x+p.x,v.y+p.y,v.z+p.z});
                check_near(batter.vertices[offset+i].color,expected_vertex.color);
            }
            offset+=primitive.triangles.size();
        }
        auto moved=staging; moved.batter_blockout_height_m*=2; moved.batter_blockout_position_m={p.x+1,p.y+.2f,p.z+2};
        const auto translated=pawapuro::load_static_batter(source,moved);
        for (std::size_t i=0;i<batter.vertices.size();++i) {
            const auto v=batter.vertices[i].position; check_near(translated.vertices[i].position,{v.x+1,v.y+.2f,v.z+2});
        }
        const auto empty=pawapuro::make_batting_reference(staging,{0,1,0},16.f/9,{});
        const auto scene=pawapuro::make_batting_reference(staging,{0,1,0},16.f/9,{},batter.vertices);
        require(scene.vertices.size()==empty.vertices.size()+batter.vertices.size()
            && scene.ball_vertex_start==empty.ball_vertex_start+batter.vertices.size()
            && scene.overlay_vertex_start==empty.overlay_vertex_start+batter.vertices.size(),"static range isolation");
        for (std::size_t i=empty.ball_vertex_start;i<empty.vertices.size();++i) {
            check_near(empty.vertices[i].position,scene.vertices[i+batter.vertices.size()].position);
            check_near(empty.vertices[i].color,scene.vertices[i+batter.vertices.size()].color);
        }
        std::filesystem::create_directories(temporary);
        const auto fails=[&](const std::filesystem::path& file,const char* why) {
            try { (void)pawapuro::load_static_batter(file,staging); }
            catch (const std::exception& e) {
                const std::string message=e.what(); require(message.find(file.string())!=std::string::npos
                    && message.find("owner=")!=std::string::npos && message.find(why)!=std::string::npos,"missing failure provenance"); return;
            }
            throw std::runtime_error("invalid asset accepted");
        };
        fails(temporary/"missing.glb","cannot open");
        std::ifstream input(source,std::ios::binary);const std::vector<char> original{std::istreambuf_iterator<char>(input),{}};
        const auto fixture=[&](const char* name,const std::vector<char>& bytes) {
            const auto f=temporary/name;std::ofstream output(f,std::ios::binary);output.write(bytes.data(),static_cast<std::streamsize>(bytes.size()));return f;
        };
        auto corrupt=original;corrupt[0]='!';fails(fixture("bad.glb",corrupt),"invalid GLB/JSON");
        std::uint32_t json_size;std::memcpy(&json_size,original.data()+12,4);const std::size_t bin=28+json_size;
        // Actual second-mesh accessor offsets, independently read from the accepted GLB.
        require(bin+138016+2<original.size(),"fixture BIN layout changed");
        corrupt=original;const std::uint32_t nan=0x7fc00000;std::memcpy(corrupt.data()+bin+132336,&nan,4);
        fails(fixture("bad-bat-position.glb",corrupt),"non-finite position");
        corrupt=original;const std::uint16_t bad_index=65535;std::memcpy(corrupt.data()+bin+138016,&bad_index,2);
        fails(fixture("bad-bat-index.glb",corrupt),"invalid accessors, indices or hierarchy");
        corrupt=original;corrupt[bin+135176]=static_cast<char>(255);
        fails(fixture("bad-bat-joint.glb",corrupt),"invalid joint index/weight");
        corrupt=original;const float half=.5f;std::memcpy(corrupt.data()+bin+135744,&half,4);
        fails(fixture("bad-bat-weight.glb",corrupt),"weight sum/nonzero influence count");
        corrupt=original;const std::string before="bat_grip",after="bad_grip";
        auto at=std::search(corrupt.begin()+20,corrupt.begin()+20+json_size,before.begin(),before.end());
        require(at!=corrupt.begin()+20+json_size,"semantic fixture not found");std::copy(after.begin(),after.end(),at);
        fails(fixture("missing-grip.glb",corrupt),"expected unique");
        // Exercise the new second-caller ownership path with a translated Bat node.
        const auto json_fixture=[&](const char* name,const std::string& before_text,const std::string& after_text) {
            std::string json(original.data()+20,json_size);
            const auto found=json.find(before_text);require(found!=std::string::npos,"JSON fixture target missing");
            json.replace(found,before_text.size(),after_text);
            while (json.size()%4) json+=' ';
            std::vector<char> bytes(original.begin(),original.begin()+20);
            bytes.insert(bytes.end(),json.begin(),json.end());
            bytes.insert(bytes.end(),original.begin()+20+json_size,original.end());
            const auto total=static_cast<std::uint32_t>(bytes.size()), chunk=static_cast<std::uint32_t>(json.size());
            std::memcpy(bytes.data()+8,&total,4);std::memcpy(bytes.data()+12,&chunk,4);
            return fixture(name,bytes);
        };
        const std::string bat_node="\"mesh\":1,\"name\":\"Bat\",\"skin\":0";
        const auto moved_asset=engine::read_mesh_glb(json_fixture("bat-node-translation.glb",bat_node,
            bat_node+",\"translation\":[1,2,3]"));
        for (std::size_t j=0;j<2;++j) for (std::size_t i=0;i<asset.primitives[j].triangles.size();++i) {
            const auto v=asset.primitives[j].triangles[i].position;
            const auto delta=asset.primitives[j].mesh_name=="Bat"?DirectX::XMFLOAT3{1,2,3}:DirectX::XMFLOAT3{};
            check_near(moved_asset.primitives[j].triangles[i].position,{v.x+delta.x,v.y+delta.y,v.z+delta.z});
        }
        fails(json_fixture("bat-missing-skin.glb",bat_node,"\"mesh\":1,\"name\":\"Bat\""),"shared skin");
        corrupt=original;corrupt[bin+135176]=0;
        fails(fixture("bat-wrong-attachment.glb",corrupt),"rigidly weighted");
        std::cout<<"Static batter PASS: shared hierarchy, both primitives, bind geometry, semantic attachment, basis/winding/colors, staging-only placement, grounding, static range isolation, second mesh transform, 9 failures.\n";
        return 0;
    } catch (const std::exception& e) { std::cerr<<e.what()<<'\n';return 1; }
}
