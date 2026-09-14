cmake_minimum_required(VERSION 3.24)

# Only the staging parser; keep its official source local and checksum-pinned.
set(toml_root "${CMAKE_CURRENT_LIST_DIR}/../.deps")
file(MAKE_DIRECTORY "${toml_root}")
file(DOWNLOAD
    "https://github.com/marzer/tomlplusplus/archive/refs/tags/v3.4.0.zip"
    "${toml_root}/tomlplusplus-3.4.0.zip"
    EXPECTED_HASH SHA256=ad2a4cd786e25305d802e7490ea65a2531195e5834bf6b4fa5a323421fd81f9b
    TLS_VERIFY ON
)
file(ARCHIVE_EXTRACT INPUT "${toml_root}/tomlplusplus-3.4.0.zip" DESTINATION "${toml_root}")
message(STATUS "toml++ 3.4.0 ready: ${toml_root}/tomlplusplus-3.4.0")
