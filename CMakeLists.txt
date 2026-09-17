cmake_minimum_required(VERSION 3.24)
project(Pawapuro LANGUAGES CXX)
if(NOT WIN32 OR NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This slice requires Windows x64 with MSVC.")
endif()

set(SDL3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.deps/SDL3-3.4.16/cmake")
if(NOT EXISTS "${SDL3_DIR}/SDL3Config.cmake")
    message(FATAL_ERROR "Prepare SDL first: cmake -P cmake/PrepareSDL3.cmake")
endif()
find_package(SDL3 3.4.16 EXACT CONFIG REQUIRED COMPONENTS SDL3-shared)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.deps/tomlplusplus-3.4.0/CMakeLists.txt")
    message(FATAL_ERROR "Prepare toml++ first: cmake -P cmake/PrepareToml.cmake")
endif()
add_subdirectory(.deps/tomlplusplus-3.4.0 EXCLUDE_FROM_ALL)
set(CGLTF_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.deps/cgltf-1.15")
if(NOT EXISTS "${CGLTF_DIR}/cgltf.h" OR NOT EXISTS "${CGLTF_DIR}/LICENSE")
    message(FATAL_ERROR "Prepare cgltf first: cmake -P cmake/PrepareCgltf.cmake")
endif()
add_library(mesh_glb STATIC engine/rendering/mesh_glb.cpp engine/rendering/glb_pose.cpp)
target_compile_features(mesh_glb PUBLIC cxx_std_20)
target_compile_options(mesh_glb PRIVATE /W4 /WX /permissive- /utf-8)
target_compile_definitions(mesh_glb PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX)
target_include_directories(mesh_glb PRIVATE "${CGLTF_DIR}")
target_link_libraries(mesh_glb PRIVATE bcrypt)

# This is a generated launch copy; the authored Data stays beside its Native caller.
configure_file(pawapuro/batting/batter/batter.toml batting/batter/batter.toml COPYONLY)
configure_file(pawapuro/batting/batter/batter.glb batting/batter/batter.glb COPYONLY)
configure_file(pawapuro/batting/staging.toml staging.toml COPYONLY)
configure_file(pawapuro/batting/pitcher/pitcher.glb batting/pitcher/pitcher.glb COPYONLY)
configure_file(pawapuro/batting/pitcher/pitcher.toml batting/pitcher/pitcher.toml COPYONLY)
foreach(asset IN ITEMS motion.glb motion.toml)
    configure_file(pawapuro/batting/batter/ingame_s0/${asset} batting/batter/ingame_s0/${asset} COPYONLY)
endforeach()
find_program(DXC dxc HINTS "$ENV{WindowsSdkVerBinPath}/x64" REQUIRED)

set(shader_headers)
foreach(stage IN ITEMS vs ps)
    if(stage STREQUAL "vs")
        set(entry VSMain)
    else()
        set(entry PSMain)
    endif()
    set(header "${CMAKE_CURRENT_BINARY_DIR}/generated/scene_${stage}.h")
    set(binary "${CMAKE_CURRENT_BINARY_DIR}/generated/scene_${stage}.cso")
    add_custom_command(OUTPUT "${header}" "${binary}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
        COMMAND "${DXC}" -T ${stage}_6_0 -E ${entry}
            "$<$<CONFIG:Debug>:-Od>" "$<$<CONFIG:Debug>:-Zi>" "$<$<CONFIG:Debug>:-Qembed_debug>"
            -Fh "${header}" -Vn scene_${stage} -Fo "${binary}"
            "${CMAKE_CURRENT_SOURCE_DIR}/engine/rendering/scene.hlsl"
        DEPENDS engine/rendering/scene.hlsl VERBATIM COMMAND_EXPAND_LISTS)
    list(APPEND shader_headers "${header}")
endforeach()

add_executable(pawapuro WIN32 pawapuro/batting/manual_swing.cpp pawapuro/batting/batter/ingame_s0/ingame_motion.cpp pawapuro/batting/player_aim.cpp pawapuro/batting/batter/batter_motion.cpp pawapuro/batting/batting_preview.cpp pawapuro/batting/bat_contact.cpp pawapuro/batting/contact_probe.cpp pawapuro/main.cpp pawapuro/batting/reference_scene.cpp
    engine/rendering/d3d12_view.cpp pawapuro/batting/staging.cpp
    pawapuro/batting/reference_pitch.cpp pawapuro/batting/pitcher/pitcher_motion.cpp pawapuro/batting/pitcher/pitch_delivery.cpp ${shader_headers})
target_compile_features(pawapuro PRIVATE cxx_std_20)
target_compile_options(pawapuro PRIVATE /W4 /WX /permissive- /utf-8)
target_compile_definitions(pawapuro PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_include_directories(pawapuro PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(pawapuro PRIVATE mesh_glb SDL3::SDL3 tomlplusplus::tomlplusplus d3d12 dxgi dxguid)
add_custom_command(TARGET pawapuro POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:pawapuro>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_DIR}/../LICENSE.txt" "$<TARGET_FILE_DIR:pawapuro>/SDL3-LICENSE.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/.deps/tomlplusplus-3.4.0/LICENSE" "$<TARGET_FILE_DIR:pawapuro>/tomlplusplus-LICENSE.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CGLTF_DIR}/LICENSE" "$<TARGET_FILE_DIR:pawapuro>/cgltf-LICENSE.txt")

include(CTest)
if(BUILD_TESTING)
    add_executable(manual_swing_test pawapuro/batting/manual_swing_test.cpp pawapuro/batting/manual_swing.cpp
        pawapuro/batting/batter/ingame_s0/ingame_motion.cpp pawapuro/batting/pitcher/pitch_delivery.cpp
        pawapuro/batting/pitcher/pitcher_motion.cpp pawapuro/batting/reference_pitch.cpp pawapuro/batting/staging.cpp)
    target_compile_features(manual_swing_test PRIVATE cxx_std_20)
    target_compile_options(manual_swing_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(manual_swing_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(manual_swing_test PRIVATE mesh_glb tomlplusplus::tomlplusplus)
    add_test(NAME manual_swing COMMAND manual_swing_test "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting")
    add_executable(ingame_motion_test pawapuro/batting/batter/ingame_s0/ingame_motion_test.cpp
        pawapuro/batting/batter/ingame_s0/ingame_motion.cpp pawapuro/batting/staging.cpp)
    target_compile_features(ingame_motion_test PRIVATE cxx_std_20)
    target_compile_options(ingame_motion_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(ingame_motion_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(ingame_motion_test PRIVATE mesh_glb tomlplusplus::tomlplusplus)
    add_test(NAME ingame_motion COMMAND ingame_motion_test "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting")
    add_executable(player_aim_test pawapuro/batting/player_aim_test.cpp pawapuro/batting/player_aim.cpp
        pawapuro/batting/contact_probe.cpp pawapuro/batting/batting_preview.cpp pawapuro/batting/bat_contact.cpp
        pawapuro/batting/batter/batter_motion.cpp pawapuro/batting/pitcher/pitch_delivery.cpp pawapuro/batting/pitcher/pitcher_motion.cpp
        pawapuro/batting/reference_pitch.cpp pawapuro/batting/reference_scene.cpp pawapuro/batting/staging.cpp)
    target_compile_features(player_aim_test PRIVATE cxx_std_20)
    target_compile_options(player_aim_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(player_aim_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(player_aim_test PRIVATE mesh_glb tomlplusplus::tomlplusplus)
    add_test(NAME player_aim COMMAND player_aim_test "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting" "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/staging.toml")

    add_executable(contact_phase_test pawapuro/batting/contact_phase_test.cpp pawapuro/batting/contact_probe.cpp
        pawapuro/batting/batting_preview.cpp pawapuro/batting/bat_contact.cpp pawapuro/batting/batter/batter_motion.cpp
        pawapuro/batting/pitcher/pitch_delivery.cpp pawapuro/batting/pitcher/pitcher_motion.cpp
        pawapuro/batting/reference_pitch.cpp pawapuro/batting/reference_scene.cpp pawapuro/batting/staging.cpp)
    target_compile_features(contact_phase_test PRIVATE cxx_std_20)
    target_compile_options(contact_phase_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(contact_phase_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(contact_phase_test PRIVATE mesh_glb tomlplusplus::tomlplusplus)
    add_test(NAME contact_phase COMMAND contact_phase_test
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting" "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/staging.toml")
    add_executable(bat_contact_test pawapuro/batting/bat_contact_test.cpp pawapuro/batting/contact_probe.cpp
        pawapuro/batting/batting_preview.cpp pawapuro/batting/bat_contact.cpp pawapuro/batting/batter/batter_motion.cpp
        pawapuro/batting/pitcher/pitch_delivery.cpp pawapuro/batting/pitcher/pitcher_motion.cpp
        pawapuro/batting/reference_pitch.cpp pawapuro/batting/reference_scene.cpp pawapuro/batting/staging.cpp)
    target_compile_features(bat_contact_test PRIVATE cxx_std_20)
    target_compile_options(bat_contact_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(bat_contact_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(bat_contact_test PRIVATE mesh_glb tomlplusplus::tomlplusplus)
    add_test(NAME bat_contact COMMAND bat_contact_test
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting" "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/staging.toml")
    add_executable(contact_probe_test pawapuro/batting/contact_probe_test.cpp pawapuro/batting/contact_probe.cpp
        pawapuro/batting/batting_preview.cpp pawapuro/batting/bat_contact.cpp pawapuro/batting/batter/batter_motion.cpp
        pawapuro/batting/pitcher/pitch_delivery.cpp pawapuro/batting/pitcher/pitcher_motion.cpp
        pawapuro/batting/reference_pitch.cpp pawapuro/batting/reference_scene.cpp pawapuro/batting/staging.cpp)
    target_compile_features(contact_probe_test PRIVATE cxx_std_20)
    target_compile_options(contact_probe_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(contact_probe_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(contact_probe_test PRIVATE mesh_glb tomlplusplus::tomlplusplus)
    add_test(NAME contact_probe COMMAND contact_probe_test
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting" "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/staging.toml")
    add_executable(batter_motion_test pawapuro/batting/batter/batter_motion_test.cpp
        pawapuro/batting/batter/batter_motion.cpp pawapuro/batting/staging.cpp)
    target_compile_features(batter_motion_test PRIVATE cxx_std_20)
    target_compile_options(batter_motion_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(batter_motion_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(batter_motion_test PRIVATE mesh_glb tomlplusplus::tomlplusplus)
    add_test(NAME batter_motion COMMAND batter_motion_test
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/batter/batter.glb"
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/batter/batter.toml"
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/staging.toml"
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/batter/motion_expected.txt")
    add_executable(batting_preview_test pawapuro/batting/batting_preview_test.cpp pawapuro/batting/contact_probe.cpp
        pawapuro/batting/batting_preview.cpp pawapuro/batting/bat_contact.cpp pawapuro/batting/batter/batter_motion.cpp
        pawapuro/batting/pitcher/pitch_delivery.cpp pawapuro/batting/pitcher/pitcher_motion.cpp
        pawapuro/batting/reference_pitch.cpp pawapuro/batting/staging.cpp)
    target_compile_features(batting_preview_test PRIVATE cxx_std_20)
    target_compile_options(batting_preview_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(batting_preview_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(batting_preview_test PRIVATE mesh_glb tomlplusplus::tomlplusplus)
    add_test(NAME batting_preview COMMAND batting_preview_test
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting"
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/staging.toml")
    add_executable(static_batter_test pawapuro/batting/batter/static_batter_test.cpp
        pawapuro/batting/batter/static_batter.cpp pawapuro/batting/staging.cpp pawapuro/batting/reference_scene.cpp)
    target_compile_features(static_batter_test PRIVATE cxx_std_20)
    target_compile_options(static_batter_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(static_batter_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(static_batter_test PRIVATE mesh_glb tomlplusplus::tomlplusplus)
    add_test(NAME static_batter COMMAND static_batter_test
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/batter/batter.glb"
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/staging.toml"
        "${CMAKE_CURRENT_BINARY_DIR}/static-batter-test-data")
    add_executable(pitch_delivery_test pawapuro/batting/pitcher/pitch_delivery_test.cpp
        pawapuro/batting/pitcher/pitch_delivery.cpp pawapuro/batting/pitcher/pitcher_motion.cpp
        pawapuro/batting/reference_pitch.cpp pawapuro/batting/reference_scene.cpp pawapuro/batting/staging.cpp)
    target_compile_features(pitch_delivery_test PRIVATE cxx_std_20)
    target_compile_options(pitch_delivery_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(pitch_delivery_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(pitch_delivery_test PRIVATE mesh_glb tomlplusplus::tomlplusplus)
    add_test(NAME pitch_delivery COMMAND pitch_delivery_test
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/pitcher/pitcher.glb"
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/pitcher/pitcher.toml"
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/staging.toml")
    add_executable(pitcher_motion_test pawapuro/batting/pitcher/pitcher_motion_test.cpp
        pawapuro/batting/pitcher/pitcher_motion.cpp pawapuro/batting/staging.cpp)
    target_compile_features(pitcher_motion_test PRIVATE cxx_std_20)
    target_compile_options(pitcher_motion_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(pitcher_motion_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(pitcher_motion_test PRIVATE mesh_glb tomlplusplus::tomlplusplus)
    add_test(NAME pitcher_motion COMMAND pitcher_motion_test
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/pitcher/pitcher.glb"
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/pitcher/pitcher.toml"
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/staging.toml"
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/pitcher/motion_expected.txt")
    add_executable(static_pitcher_test pawapuro/batting/pitcher/static_pitcher_test.cpp
        pawapuro/batting/pitcher/static_pitcher.cpp pawapuro/batting/staging.cpp
        pawapuro/batting/reference_scene.cpp)
    target_compile_features(static_pitcher_test PRIVATE cxx_std_20)
    target_compile_options(static_pitcher_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(static_pitcher_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(static_pitcher_test PRIVATE mesh_glb tomlplusplus::tomlplusplus)
    add_test(NAME static_pitcher COMMAND static_pitcher_test
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/pitcher/pitcher.glb"
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/staging.toml"
        "${CMAKE_CURRENT_BINARY_DIR}/static-pitcher-test-data")
    add_executable(reference_pitch_test pawapuro/batting/reference_pitch_test.cpp pawapuro/batting/reference_pitch.cpp
        pawapuro/batting/reference_scene.cpp pawapuro/batting/staging.cpp)
    target_compile_features(reference_pitch_test PRIVATE cxx_std_20)
    target_compile_options(reference_pitch_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_include_directories(reference_pitch_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_definitions(reference_pitch_test PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_link_libraries(reference_pitch_test PRIVATE tomlplusplus::tomlplusplus)
    add_test(NAME reference_pitch COMMAND reference_pitch_test "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/staging.toml")
    add_executable(staging_test pawapuro/batting/staging_test.cpp pawapuro/batting/staging.cpp)
    target_compile_features(staging_test PRIVATE cxx_std_20)
    target_compile_options(staging_test PRIVATE /W4 /WX /permissive- /utf-8)
    target_link_libraries(staging_test PRIVATE tomlplusplus::tomlplusplus)
    add_test(NAME batting_staging COMMAND staging_test
        "${CMAKE_CURRENT_SOURCE_DIR}/pawapuro/batting/staging.toml"
        "${CMAKE_CURRENT_BINARY_DIR}/staging-test-data")
endif()
