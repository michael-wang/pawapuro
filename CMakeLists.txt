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

# This is a generated launch copy; the authored Data stays beside its Native caller.
configure_file(pawapuro/batting/staging.toml staging.toml COPYONLY)
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

add_executable(pawapuro WIN32 pawapuro/main.cpp pawapuro/batting/reference_scene.cpp
    engine/rendering/d3d12_view.cpp pawapuro/batting/staging.cpp
    pawapuro/batting/reference_pitch.cpp ${shader_headers})
target_compile_features(pawapuro PRIVATE cxx_std_20)
target_compile_options(pawapuro PRIVATE /W4 /WX /permissive- /utf-8)
target_compile_definitions(pawapuro PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_include_directories(pawapuro PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(pawapuro PRIVATE SDL3::SDL3 tomlplusplus::tomlplusplus d3d12 dxgi dxguid)
add_custom_command(TARGET pawapuro POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:pawapuro>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_DIR}/../LICENSE.txt" "$<TARGET_FILE_DIR:pawapuro>/SDL3-LICENSE.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/.deps/tomlplusplus-3.4.0/LICENSE" "$<TARGET_FILE_DIR:pawapuro>/tomlplusplus-LICENSE.txt")

include(CTest)
if(BUILD_TESTING)
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
