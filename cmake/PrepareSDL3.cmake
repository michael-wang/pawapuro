cmake_minimum_required(VERSION 3.24)

# Keep SDL local and pinned; no package manager or global installation is needed.
set(sdl_version "3.4.16")
set(sdl_archive "SDL3-devel-${sdl_version}-VC.zip")
set(sdl_root "${CMAKE_CURRENT_LIST_DIR}/../.deps")
file(MAKE_DIRECTORY "${sdl_root}")
file(DOWNLOAD
    "https://github.com/libsdl-org/SDL/releases/download/release-${sdl_version}/${sdl_archive}"
    "${sdl_root}/${sdl_archive}"
    EXPECTED_HASH SHA256=1a784cb2a5c64d56fe7a62090fe9d242d9865f235e4ea9678f1a6ba4e693e7de
    TLS_VERIFY ON
)
file(ARCHIVE_EXTRACT INPUT "${sdl_root}/${sdl_archive}" DESTINATION "${sdl_root}")
message(STATUS "SDL3 ${sdl_version} ready: ${sdl_root}/SDL3-${sdl_version}/cmake")
