# M1 Step 0 — Development environment

Audited 2026-09-14. This records local observations and preparation gaps; the
[milestone](../milestones/01-batting-feel.md) owns scope and acceptance criteria.
Step 1 has **not** started. No tools or dependencies were installed, and no
source, build files, shaders or assets were created.

## Observed tools

| Component | Finding |
|---|---|
| OS | Windows 11 Home x64, build 10.0.26200. |
| Visual Studio | Community 2022, 17.14.11; installer reports a complete, launchable installation. |
| MSVC | Toolset 14.44.35207; x64 compiler launches and reports 19.44.35214. x64 linker, STL headers and runtime library are present. |
| Windows SDK | 10.0.26100.0; Windows/UCRT headers and libraries, resource compiler and manifest tool are present. |
| D3D12/DXGI | SDK headers `d3d12.h`, `d3d12sdklayers.h`, `dxgi1_6.h`; x64 `d3d12.lib`, `dxgi.lib`, `dxguid.lib` are present. System D3D12/DXGI runtime and `d3d12SDKLayers.dll` are present. |
| HLSL tools | SDK DXC runs: dxcompiler 1.7.2308.16, dxil 1.7.2308.24. DXC headers/DLLs and FXC are present. No shader was compiled. |
| Math | SDK `DirectXMath.h` is present; no separate math package is required for this slice. |
| CMake / Ninja | Visual Studio bundled CMake 3.31.6-msvc6 and Ninja 1.12.1 both run. |
| Git | 2.50.1.windows.1; normal fetch from origin succeeded. Credentials were not inspected. |
| Graphics adapters | NVIDIA GeForce RTX 5070 Ti (driver 32.0.16.1074) and AMD Radeon Graphics (32.0.21042.62). |
| SDL3 | No development package was found in the checked locations; not yet established as available for this project. |
| PIX | Not found in the checked standard installation locations; not a Step 1 prerequisite. |

The regular shell exposes Git but not MSVC, CMake, Ninja or DXC on PATH. Use an
x64 Visual Studio developer shell for future builds; no global PATH change is
needed. The installation includes `Common7/Tools/VsDevCmd.bat`. Bundled CMake and
Ninja live under `Common7/IDE/CommonExtensions/Microsoft/CMake/`; SDK tools live
under the Windows Kits `10/bin/10.0.26100.0/x64/` directory. Use discovery or a
developer shell instead of committing machine-specific absolute paths.

Checks used Visual Studio installer discovery, SDK registry/file inspection,
tool version commands and OS/adapter queries. Dependency checks covered the
repository, PATH, standard Program Files directory names, common root dependency
directories and Visual Studio's vcpkg installed directory. No `VCPKG_ROOT` was
configured and no project dependencies/assets were present. This was not an
exhaustive disk search; an SDL runtime bundled inside another app would not by
itself establish a usable development package.

No program was compiled or linked, and no D3D12 device was created. Adapter
enumeration and DLL presence do not verify feature support, debug-layer
activation, presentation, frame timing or replay behavior. Those checks belong
to the authorized Step 1 implementation.

## Minimum for Step 1

- **Already present:** a C++20-capable x64 compiler/linker and standard library,
  Windows SDK with D3D12/DXGI, and an HLSL compiler. CMake/Ninja are available for
  a small build setup; they are conveniences rather than additional runtime requirements.
- **Outstanding for the agreed SDL3 route:** locate or obtain one SDL3 development
  package with headers and an x64 link/runtime configuration. Version and source
  remain to be selected in a separately authorized dependency task. Do not silently
  substitute Win32 window/input or introduce a package-management framework.
- **No external assets required:** simple home-plate/release markers and a ball
  can supply the initial static scene and subsequent fixed-condition flight.
  Keyboard pause, single-step and rethrow are sufficient; final bindings remain
  an implementation choice. Native fixtures must be identified as such.
- **Not needed yet:** Lua, Dear ImGui, audio libraries, Blender/glTF import,
  physics packages, ECS, inference, Agility SDK or a graphics abstraction package.
  Do not add them merely because the longer-term plan discusses them.

## Later sample needs identified by Step 0

These are acquisition requirements for later milestone steps, not Step 1 blockers:

- One minimal rigged pitcher sample with a pitch clip covering preparation,
  leg lift/stride, rotation, release and follow-through; a known hand/release
  attachment and timing allow simulation/pose alignment.
- One minimal rigged batter sample with a swing clip and a bat attachment/path
  reference, including follow-through for a miss.
- A short swing whoosh and a clean contact sound for the first feedback pass;
  further sweet-spot/HR sound variants should follow actual listening needs.
- glTF/GLB samples must allow scale, axes, skeleton and animation export to be
  checked. Keep source/export versions and usage rights traceable. No samples
  have been selected or acquired; do not build a general importer in Step 1.

Step 0's inventory is complete. Resolve SDL3 availability and authorize coding
before starting Step 1's first, static-scene delivery.
