# M1 Step 0 — 開發環境

核對日期：2026-09-14。本文件記錄本機觀察結果與準備缺項；scope 與驗收標準由
[milestone](../milestones/01-batting-feel.md) 定義。Step 1 **尚未開始**。

以下至「Step 0 辨識出的後續 sample 需求」保留 Step 0 當時的核對結果：當時未安裝任何工具或依賴，也未建立 source、build 設定、shader 或資產。本次 SDL3 準備與驗證另記於末節，不改寫 Step 0 的範圍或結果。

## 已觀察到的工具

| 元件 | 核對結果 |
|---|---|
| OS | Windows 11 Home x64，build 10.0.26200。 |
| Visual Studio | Community 2022，17.14.11；安裝程式回報安裝完整且可啟動。 |
| MSVC | Toolset 14.44.35207；x64 compiler 可啟動，回報版本 19.44.35214。x64 linker、STL headers 與 runtime library 均存在。 |
| Windows SDK | 10.0.26100.0；Windows/UCRT headers 與 libraries、resource compiler 及 manifest tool 均存在。 |
| D3D12/DXGI | SDK headers `d3d12.h`、`d3d12sdklayers.h`、`dxgi1_6.h`；x64 `d3d12.lib`、`dxgi.lib`、`dxguid.lib` 均存在。系統 D3D12/DXGI runtime 與 `d3d12SDKLayers.dll` 均存在。 |
| HLSL 工具 | SDK DXC 可執行：dxcompiler 1.7.2308.16、dxil 1.7.2308.24。DXC headers/DLLs 與 FXC 均存在。未編譯任何 shader。 |
| 數學 | SDK `DirectXMath.h` 存在；此切片不需要額外的數學套件。 |
| CMake / Ninja | Visual Studio 隨附的 CMake 3.31.6-msvc6 與 Ninja 1.12.1 均可執行。 |
| Git | 2.50.1.windows.1；從 origin 正常 fetch 成功。未檢視憑證。 |
| 顯示卡 | NVIDIA GeForce RTX 5070 Ti（driver 32.0.16.1074）與 AMD Radeon Graphics（32.0.21042.62）。 |
| SDL3 | 在已檢查的位置未找到開發套件；當時尚未確認本專案可用。 |
| PIX | 在已檢查的標準安裝位置未找到；不是 Step 1 的必要條件。 |

一般 shell 的 PATH 可找到 Git，但找不到 MSVC、CMake、Ninja 或 DXC。後續建置應使用 x64 Visual Studio developer shell；不需要修改全域 PATH。安裝內容包含 `Common7/Tools/VsDevCmd.bat`。隨附的 CMake 與 Ninja 位於 `Common7/IDE/CommonExtensions/Microsoft/CMake/`；SDK 工具位於 Windows Kits 的 `10/bin/10.0.26100.0/x64/` 目錄。使用工具探索或 developer shell，不要提交特定機器的絕對路徑。

核對方式包含 Visual Studio 安裝資訊探索、SDK 登錄與檔案檢查、工具版本命令，以及 OS／顯示卡查詢。依賴檢查涵蓋 repository、PATH、標準 Program Files 目錄名稱、常見根目錄下的依賴目錄，以及 Visual Studio 的 vcpkg installed 目錄。當時未設定 `VCPKG_ROOT`，專案也沒有依賴或資產。這不是全磁碟搜尋；其他應用程式內附的 SDL runtime 本身不代表有可用的開發套件。

Step 0 未編譯或連結任何程式，也未建立 D3D12 device。列出顯示卡與確認 DLL 存在，不能驗證功能支援、debug layer 啟用、畫面呈現、frame timing 或 replay 行為。這些檢查屬於獲授權後的 Step 1 implementation。

## Step 1 的最低需求（Step 0 核對時）

- **已具備：**支援 C++20 的 x64 compiler/linker 與 standard library、包含 D3D12/DXGI 的 Windows SDK，以及 HLSL compiler。CMake/Ninja 可用於小型 build 設定；它們是建置便利工具，不是額外的 runtime 需求。
- **既定 SDL3 路線的待補項目：**尋找或取得一份 SDL3 開發套件，包含 headers 與 x64 link/runtime 設定。當時版本與來源尚待另行授權的依賴準備工作選定。不要默默改用 Win32 window/input，也不要引入套件管理框架。
- **不需要外部資產：**簡單的本壘／出手標記與球即可支援初始靜態場景，以及後續固定條件的飛行。鍵盤 pause、single-step 與重投已足夠；實際按鍵仍由 implementation 決定。Native fixture 必須明示來源。
- **尚不需要：**Lua、Dear ImGui、audio libraries、Blender/glTF import、physics packages、ECS、inference、Agility SDK 或 graphics abstraction package。不要只因長期計畫提及它們就加入。

## Step 0 辨識出的後續 sample 需求

以下是後續 milestone 步驟的資產取得需求，不是 Step 1 的阻礙：

- 一份最低可用的 rigged pitcher sample，pitch clip 涵蓋準備、抬腿／跨步、旋轉、出手與 follow-through；已知的手部／出手 attachment 與 timing 可用於對齊 simulation 與 pose。
- 一份最低可用的 rigged batter sample，包含 swing clip 與球棒 attachment/path 參考，以及空揮的 follow-through。
- 第一輪 feedback 所需的短揮棒破風聲與清楚的接觸音效；更多 sweet-spot/HR 音效變化應依實際聆聽需求增加。
- glTF/GLB samples 必須能核對尺度、座標軸、骨架與動畫匯出。保留可追查的來源／匯出版本及使用權利。尚未選定或取得 samples；不要在 Step 1 建立通用 importer。

Step 0 的盤點已完成；當時的下一步是解決 SDL3 可用性，並取得 coding 授權，才開始 Step 1 的第一個靜態場景交付。

## SDL3 依賴準備（2026-09-14）

### 固定版本與來源

採用官方穩定版 **SDL3 3.4.16**，release 日期為 2026-09-02，官方 release metadata 的 `prerelease` 為 `false`。

- [官方 release](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.16)
- [官方 Visual C 開發套件：SDL3-devel-3.4.16-VC.zip](https://github.com/libsdl-org/SDL/releases/download/release-3.4.16/SDL3-devel-3.4.16-VC.zip)
- SHA-256：`1a784cb2a5c64d56fe7a62090fe9d242d9865f235e4ea9678f1a6ba4e693e7de`。本機下載檔案的雜湊與官方 release asset metadata 一致；準備腳本也固定並驗證此雜湊。

### 最小整合方式

保留單一 [cmake/PrepareSDL3.cmake](../../cmake/PrepareSDL3.cmake)，只負責下載固定的官方 archive、驗證 SHA-256，並解壓縮到 repository 內的 `.deps/SDL3-3.4.16/`。不做全域安裝，不修改 PATH，也不建立通用依賴抽象。`.deps/` 由 `.gitignore` 忽略；保留本機開發套件、授權檔案與下載快取，不提交第三方 binary。

在 repository 根目錄、可執行 CMake 的 developer shell 執行：

```powershell
cmake -P cmake/PrepareSDL3.cmake
```

未來獲准建立 build 設定後，使用官方 CMake config：以 `.deps/SDL3-3.4.16/cmake/` 作為 `SDL3_DIR`，呼叫 `find_package(SDL3 3.4.16 EXACT CONFIG REQUIRED COMPONENTS SDL3-shared)`，並連結官方 `SDL3::SDL3` target。不建立自訂 SDL wrapper target。此為整合契約，目前沒有 Pawapuro 根目錄 `CMakeLists.txt` 或 app target。

x64 build 使用套件的 `lib/x64/SDL3.lib`，執行時需將對應的 `lib/x64/SDL3.dll` 放在 executable 旁邊；可由官方 target 的 `$<TARGET_FILE:SDL3::SDL3>` 取得 DLL 路徑。套件包含 `SDL3.pdb` 與 `LICENSE.txt`，日後發布時須保留適用的授權聲明。官方使用方式見 [SDL CMake 文件](https://wiki.libsdl.org/SDL3/README-cmake) 與 [Windows 文件](https://wiki.libsdl.org/SDL3/README-windows)。

選擇預編譯 Visual C 套件，是因目前只需要 Windows x64 的 SDL 公開 API；它省去 clean build 時編譯 SDL 的時間，也只需維護固定版本、雜湊與一個準備腳本。相較之下，FetchContent 或 Git submodule 搭配原始碼建置較適合需要修改 SDL、調整 SDL build options 或深入除錯其內部時；目前不承擔額外的原始碼建置成本。手動下載但未固定雜湊的全域安裝較難重現；vcpkg／Conan 則不是取得這一個套件所必需。

### 實際驗證

使用現有 `VsDevCmd.bat -arch=x64 -host_arch=x64`，在子行程設定工具環境，透過 CMake 3.31.6-msvc6、Ninja 1.12.1 與 MSVC 19.44.35214，分別 configure、compile、link 並執行暫存的 Debug／Release C++20 console program：

- CMake 使用 `-G Ninja`、`-DCMAKE_BUILD_TYPE=Debug` 或 `Release`，以及指向上述套件的 `-DSDL3_DIR:PATH=...`；`find_package` 要求 `3.4.16 EXACT` 與 `SDL3-shared`。
- 兩個組態均使用 `-std:c++20 /W4 /WX`；Debug 使用 `-MDd /Zi /Od /RTC1`，Release 使用 `-MD /O2 /DNDEBUG`；linker 使用 `/machine:x64`，連結 `lib/x64/SDL3.lib`。
- 程式 include `SDL3/SDL.h` 與 `SDL3/SDL_main.h`，以 `static_assert` 核對 headers 版本，呼叫 `SDL_Init(0)`、`SDL_GetVersion()` 與 `SDL_Quit()`。執行前將官方 x64 `SDL3.dll` 複製到 executable 旁。
- Debug 與 Release 均編譯／連結成功並以 exit code `0` 結束；兩者輸出均為 `headers=3004016 runtime=3004016`，確認 headers 與實際載入的 runtime 版本一致。
- 暫存程式、CMake 專案、兩組 build 目錄與複製的 DLL 均已移除。沒有加入 Pawapuro production／game／rendering source，也沒有開始 M1 Step 1。

### 限制與目前狀態

SDL3 的開發套件缺項已解決。首次準備仍需要連線取得官方 archive；本機快取不屬於 Git，另一台機器需執行準備命令。本次只驗證上述工具鏈與動態連結方式，未驗證其他平台、static linking 或無開發工具的乾淨 Windows 部署。

`SDL_Init(0)` 不啟用 video/audio 子系統；未建立視窗、處理真實輸入、建立 D3D12 device 或測試呈現。官方套件提供 PDB，但尚未驗證 debugger 載入 symbols／對應原始碼；預編譯 SDL 不代表能完整逐步追蹤其最佳化後的內部程式。這些未驗證項目不算 Step 1 已完成，後續仍須取得 implementation 授權。
