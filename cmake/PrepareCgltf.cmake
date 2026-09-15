cmake_minimum_required(VERSION 3.24)

# Official v1.15, peeled commit; preparation is explicit, never part of configure.
set(cgltf_commit "360db1a95480fe102ae9c69b27c5d101167ff5ba")
set(cgltf_root "${CMAKE_CURRENT_LIST_DIR}/../.deps/cgltf-1.15")
file(MAKE_DIRECTORY "${cgltf_root}")
file(DOWNLOAD "https://raw.githubusercontent.com/jkuhlmann/cgltf/${cgltf_commit}/cgltf.h"
    "${cgltf_root}/cgltf.h"
    EXPECTED_HASH SHA256=e378a21c084bf1f288bb799de827bb26906efb024255f1ecf1705ea13f11c6ec
    TLS_VERIFY ON)
file(DOWNLOAD "https://raw.githubusercontent.com/jkuhlmann/cgltf/${cgltf_commit}/LICENSE"
    "${cgltf_root}/LICENSE"
    EXPECTED_HASH SHA256=f619925f80ef862497aaf8e8155ef218fa6a2190055129523ca3df9119a9ba95
    TLS_VERIFY ON)
message(STATUS "cgltf 1.15 (MIT), ${cgltf_commit}, ready: ${cgltf_root}")
