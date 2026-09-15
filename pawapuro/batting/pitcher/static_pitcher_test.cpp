#include "static_pitcher.hpp"
#include "pawapuro/batting/reference_scene.hpp"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace {
void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
void check_near(DirectX::XMFLOAT3 actual, DirectX::XMFLOAT3 expected)
{
    require(std::abs(actual.x-expected.x) < 0.00001f && std::abs(actual.y-expected.y) < 0.00001f
        && std::abs(actual.z-expected.z) < 0.00001f, "position/color mismatch (1e-5)");
}
void same_vertex(const engine::Vertex& a, const engine::Vertex& b)
{
    check_near(a.position,b.position); check_near(a.color,b.color);
}
}
int main(int argc, char** argv)
{
    try {
        require(argc == 4, "expected GLB, staging and temporary output directory");
        const std::filesystem::path source = argv[1], temporary = argv[3];
        const auto staging = pawapuro::load_batting_staging(argv[2]);
        const auto glb = engine::read_mesh_glb(source);
        require(glb.mesh_name == "PitcherMesh" && glb.mesh_node_name == "PitcherMesh", "mesh identity");
        require(glb.nodes.size() == 15 && glb.source_vertex_count == 2362 && glb.triangles.size() == 11808,
            "official sample node/vertex/triangle counts");
        const auto pitcher = pawapuro::load_static_pitcher(source,staging);
        require(pitcher.vertices.size()/3 == 3936 && pitcher.source_vertex_count == 2362, "expanded triangles");
        check_near(pitcher.local_min,{-1.263325f,0,-1.005480f});
        check_near(pitcher.local_max,{1.340500f,2.565150f,0.621810f});
        const auto p = staging.pitcher_blockout_position_m;
        check_near(pitcher.world_min,{p.x-1.263325f,p.y,p.z-1.005480f});
        check_near(pitcher.world_max,{p.x+1.340500f,p.y+2.565150f,p.z+0.621810f});
        check_near(pitcher.bind_grip_world,{p.x-1.12f,p.y+1.315f,p.z-0.35f});
        require(pitcher.bind_grip_world.x < p.x, "throwing hand must remain on game -X side in bind pose");
        bool left_hand = false;
        for (const auto& node : glb.nodes) if (node.name == "hand_L") {
            require(-node.world[12] > 0, "glove must remain on game +X side in bind pose");
            left_hand = true;
        }
        require(left_hand,"left hand missing");
        for (std::size_t i = 0; i < pitcher.vertices.size(); ++i) {
            // Reflection must reverse winding exactly once, without changing color association.
            const auto j = i/3*3 + (i%3 == 0 ? 0 : 3-i%3);
            auto expected = glb.triangles[j];
            expected.position = {-expected.position.x+p.x,expected.position.y+p.y,expected.position.z+p.z};
            same_vertex(pitcher.vertices[i],expected);
        }
        auto moved = staging;
        moved.pitcher_blockout_height_m *= 2;
        moved.pitcher_blockout_position_m = {p.x+1,p.y+0.2f,p.z+2};
        const auto translated = pawapuro::load_static_pitcher(source,moved);
        check_near(translated.local_min,pitcher.local_min); check_near(translated.local_max,pitcher.local_max);
        for (std::size_t i = 0; i < pitcher.vertices.size(); ++i) {
            const auto v = pitcher.vertices[i].position;
            check_near(translated.vertices[i].position,{v.x+1,v.y+0.2f,v.z+2});
        }
        const auto empty = pawapuro::make_batting_reference(staging,{0,1,0},16.0f/9.0f,{});
        const auto scene = pawapuro::make_batting_reference(staging,{0,1,0},16.0f/9.0f,pitcher.vertices);
        const auto added = pitcher.vertices.size();
        require(scene.vertices.size() == empty.vertices.size()+added
            && scene.ball_vertex_start == empty.ball_vertex_start+added
            && scene.overlay_vertex_start == empty.overlay_vertex_start+added, "world/ball/overlay range isolation");
        for (std::size_t i = empty.ball_vertex_start; i < empty.vertices.size(); ++i)
            same_vertex(empty.vertices[i],scene.vertices[i+added]);

        std::filesystem::create_directories(temporary);
        const auto fails = [&](const std::filesystem::path& path, const char* reason) {
            try { (void)pawapuro::load_static_pitcher(path,staging); }
            catch (const std::exception& error) {
                const std::string message = error.what();
                require(message.find(path.string()) != std::string::npos && message.find("owner=") != std::string::npos
                    && message.find(reason) != std::string::npos, "failure lacks source/owner/reason");
                return;
            }
            throw std::runtime_error("invalid asset unexpectedly accepted");
        };
        fails(temporary/"missing.glb","cannot open");
        std::ifstream input(source,std::ios::binary);
        const std::vector<char> original{std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
        const auto fixture = [&](const char* name, const std::vector<char>& bytes) {
            const auto path = temporary/name;
            std::ofstream output(path,std::ios::binary);
            output.write(bytes.data(),static_cast<std::streamsize>(bytes.size())); output.close();
            return path;
        };
        auto corrupt = original; corrupt[0] = '!';
        fails(fixture("bad-magic.glb",corrupt),"invalid GLB/JSON");
        corrupt.assign(original.begin(),original.begin()+16);
        fails(fixture("truncated.glb",corrupt),"invalid container size");
        // Sample-specific corruption fixtures, not a second GLB parser/conformance suite.
        std::uint32_t json_size = 0;
        std::memcpy(&json_size,original.data()+12,4);
        const std::size_t bin = 28+json_size;
        require(bin+94480+2 < original.size(),"sample BIN layout changed");
        corrupt = original;
        const std::uint32_t nan = 0x7fc00000;
        std::memcpy(corrupt.data()+bin,&nan,4);
        fails(fixture("nonfinite-position.glb",corrupt),"non-finite position");
        corrupt = original;
        const std::uint16_t invalid_index = 65535;
        std::memcpy(corrupt.data()+bin+94480,&invalid_index,2);
        fails(fixture("invalid-index.glb",corrupt),"invalid accessors, indices or hierarchy");
        require(bin+118928+8<original.size(),"sample skin/timeline BIN layout changed");
        corrupt=original; corrupt[bin+47240]=static_cast<char>(255);
        fails(fixture("invalid-joint.glb",corrupt),"invalid joint index/weight");
        corrupt=original; const float negative=-0.5f;
        std::memcpy(corrupt.data()+bin+56688,&negative,4);
        fails(fixture("negative-weight.glb",corrupt),"invalid joint index/weight");
        corrupt=original; const float half=0.5f;
        std::memcpy(corrupt.data()+bin+56688,&half,4);
        fails(fixture("bad-weight-sum.glb",corrupt),"weight sum/nonzero influence count");
        corrupt=original; std::memcpy(corrupt.data()+bin+118096,&nan,4);
        fails(fixture("nonfinite-inverse-bind.glb",corrupt),"invalid inverse bind matrix");
        corrupt=original; const float zero=0;
        std::memcpy(corrupt.data()+bin+118928+4,&zero,4);
        fails(fixture("unordered-time.glb",corrupt),"animation times must be finite and strictly increasing");
        std::cout << "Static pitcher: official subset, finite geometry/color, 3936 triangles, bind hierarchy, "
            "basis/winding, scale=1, staging placement/grounding, ball/overlay isolation and 10 failures passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
