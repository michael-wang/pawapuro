# M1 Step 0 — 開發環境

核對日期：2026-09-14。本文件記錄本機觀察結果與準備缺項；scope 與驗收標準由
[milestone](../milestones/01-batting-feel.md) 定義。已完成 **Step 1 交付 1 的靜態場景驗證**；Step 1 整體尚未完成，後續已加入 staging calibration、啟動 Data 與交付 2 的 reference flight；結果與建置方式見各節。

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

本節保留依賴準備當時的結果與限制；後續 app 實作及新增驗證見末節。

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

## M1 Step 1／交付 1：靜態參考場景（2026-09-14）

以下保留 delivery 1 當時的結果；本輪 calibration 的改動另記於末節。本次僅實作 native window 與靜態 3D 場景。沒有球的運動、simulation tick、pause／single-step、重投、動畫、Lua、Data reload、audio 或 asset import；不代表 Step 1 整體或 M1 已完成。

### 建置與啟動

在 **x64 Native Tools Command Prompt for VS 2022** 切換到 repository 根目錄，執行下列命令。`chcp 65001` 只設定此 console 的字碼頁；本機繁中 MSVC 的 `/showIncludes` 曾被 CMake/Ninja 錯誤解碼，造成 header dependencies 為零。以 UTF-8 configure／build 後已修正，無需安裝語言套件或增加 compiler wrapper。若曾在其他字碼頁 configure，第一次改用 UTF-8 時以 `cmake --fresh` 重新 configure。

```bat
chcp 65001
cmake -P cmake/PrepareSDL3.cmake
cmake -P cmake/PrepareToml.cmake
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
build\debug\pawapuro.exe
cmake -S . -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/release
build\release\pawapuro.exe
```

CMake 會使用已固定的 SDL3 3.4.16，透過 developer environment 尋找 SDK 的 `dxc`，編譯 HLSL 為內嵌 bytecode，並在 link 後複製 `SDL3.dll` 與 `SDL3-LICENSE.txt` 到 executable 旁。可直接啟動 build 目錄的 executable，不依賴目前工作目錄尋找 shader。沒有新增外部依賴；所有 build 產物與本機紀錄由 `/build/` 忽略。

Debug build 要求可用的 D3D12 debug layer，初始化時啟用 GPU-based validation。診斷寫入 `stderr`；需要保存時，可在 repository 根目錄的 PowerShell 執行：

```powershell
Start-Process -FilePath (Resolve-Path build/debug/pawapuro.exe) -RedirectStandardError build/debug/run.log -PassThru
```

關閉 app 後讀取 `build/debug/run.log`。一般啟動不會自動在 repository 建立 log；錯誤也會以 SDL message box 顯示。

### 場景與程式邊界

- `pawapuro/main.cpp`：SDL video/window 與事件迴圈，依 pixel size 處理 resize；minimize 時暫停提交畫面，正常 close 退出。沒有 gameplay input 或 simulation。
- `pawapuro/batting/reference_scene.cpp/.hpp`：唯一的棒球尺度、camera 與靜態 geometry fixture。單位為公尺，+Y 向上、+Z 朝投手；本壘寬 0.4318 m，投手板參考 z=18.44 m，release marker 位於 `(0.25, 1.8, 16.8)`。球 marker 半徑 **0.10 m，刻意放大以便辨識，並非實際球體物理尺寸**。Camera 從本壘後方稍偏打者側看向投手，vertical FOV 65°。
- `engine/rendering/d3d12_view.cpp/.hpp` 與 `scene.hlsl`：D3D12 直接初始化、單一 triangle buffer／pipeline、depth test、resize、frame submission 與 shutdown。Engine 不辨識本壘或棒球。

目前唯一的資源 owner `D3D12View` 集中管理同一視窗的 COM resources 與 fence event，解決 resize／exception／shutdown 時必須先等 GPU 再釋放資源的問題。`ComPtr` 與 SDL window 的 `unique_ptr` 處理既有 API 的釋放責任；`Vertex` 是這次 CPU geometry 與 shader 共用的 position/color layout。局部 `check`／`transition` 與 fixture 的 triangle／quad helper 只消除眼前重複的錯誤檢查、barrier 與頂點展開；沒有 RHI、scene graph、mesh framework 或其他預先泛化。

此切片刻意僅保留一個 frame in flight：每次 Present 後等待 fence，才重用 allocator／depth。1053 個靜態頂點保留在小型 upload buffer，一次 draw；尚未為這個數量加入 staging/default-heap 搬移路徑。這些是本次可追蹤 lifetime 的取捨，不是未來效能目標或已驗證的最佳實作。

### 已驗證結果與限制

- 使用前述 Windows 11、MSVC 19.44.35214、SDK 10.0.26100.0、CMake 3.31.6-msvc6、Ninja 1.12.1 與 DXC。Debug／Release 均以 C++20、`/W4 /WX` 成功編譯與連結。
- 兩個組態都在 NVIDIA GeForce RTX 5070 Ti 建立硬體 D3D12 device 並持續呈現。視窗擷取確認五角本壘、地面距離參考、青色 release marker 與中央米白色球 marker 可見；resize 後仍保留此關係。
- 暫存、指定 process 的檢查使用 Windows API resize、minimize、restore，再送正常 `WM_CLOSE`。Debug／Release 均通過，觀察到 client pixel size 1901×1266 與 2251×1441；最終兩次執行分別完成 1348／620 frames，皆以 exit code `0` 結束。這不是手動拖曳視窗邊框或點擊 X 按鈕的實測。
- 最終 Debug run 的 debug layer 與 GPU-based validation 均啟用；`ID3D12InfoQueue1` callback 記錄的 error/corruption 為 **0**。釋放所有 app-owned rendering resources 後，live-object report 只列出當時為了回報而保留的 `ID3D12Device`（id 274、Refcount 3，包含報告／info queue 介面）；未列出 live child resources，隨後釋放剩餘 device 介面。未見本次程式造成的明顯 resource leak；不宣稱做過通用記憶體 leak 分析。
- 已確認 Ninja 記錄 project/shader header dependencies；touch 共用 header 會排入三個 `.cpp` 重編，沒有修改時 Debug／Release 都顯示 `no work to do`。
- 原生 computer-use helper 在 sandbox 啟動失敗；改用暫存的 process-targeted Windows API 檢查與 `PrintWindow` 擷取，實際檢視擷取結果。檢查腳本未納入版本控制；本機 build/run logs 與擷取圖留在忽略的 `build/`。
- 未測 AMD adapter、多螢幕／DPI 切換、極端視窗比例、device removal recovery 或乾淨機器部署。球的大小只是可視 placeholder；無球路或打擊手感驗收。Present 使用同步間隔 1，未量測 frame-time 分位數或端到端 latency，不能據此宣告 T10／T11 通過。

## 靜態 staging calibration 與啟動 Data（2026-09-14）

### Dependency 與操作方式

新增且只新增 **toml++ 3.4.0**，使用 [官方 v3.4.0 release](https://github.com/marzer/tomlplusplus/releases/tag/v3.4.0) 的 [固定 source archive](https://github.com/marzer/tomlplusplus/archive/refs/tags/v3.4.0.zip)。下載實測 SHA-256：`ad2a4cd786e25305d802e7490ea65a2531195e5834bf6b4fa5a323421fd81f9b`；[PrepareToml.cmake](../../cmake/PrepareToml.cmake) 固定 URL／雜湊並驗證後解壓至 `.deps/tomlplusplus-3.4.0/`。沒有全域安裝或 PATH 變更。

選 TOML 是因本次需要人類可讀、帶註解的少量 staging 值；[toml++](https://marzer.github.io/tomlplusplus/v3.4.0/index.html) 已提供成熟的 TOML parser 與語法位置診斷，省去自行定義格式／parser。它支援 C++17 以上、沒有額外依賴；只在一個 production `.cpp` include，透過上游 `tomlplusplus::tomlplusplus` header-only target 整合，不建立 package manager 或自己的 dependency wrapper。Build 後複製其 MIT 授權至 `tomlplusplus-LICENSE.txt`。

先依上方 developer console 命令準備依賴與建置；新增的必要準備命令為：

```bat
cmake -P cmake/PrepareToml.cmake
```

開發調參時，在 repository 根目錄直接讀取 authored Data：

```bat
build\debug\pawapuro.exe --staging pawapuro/batting/staging.toml
```

修改 TOML 後關閉並重開即可，不需要編譯、link 或 CMake refresh。`--staging` 只指定本次啟動資料來源。未提供參數時讀取 executable 旁的 `staging.toml`；那是 CMake `configure_file(COPYONLY)` 產生的 launch copy，更新 authored TOML 後 build 會更新它，不應把它當成另一份維護來源。已從不同工作目錄啟動 Debug／Release，確認 default path 不依賴 CWD。

### 首輪 calibration 的比較與驗證（歷史紀錄）

以下是前一輪 1280×720／右打側的結果；目前基準已由下一節取代，保留這些數字作比較證據。

- 固定 1280×720 後比較了 40° 與 36° 兩個 camera candidate；比較只修改 TOML 並重開同一 executable。採用 36°，本壘仍完整且更扁平，release／球 marker 較醒目。與 delivery 1 的擷取圖相比，投手後方不再緊接短草地邊界，而可見延伸草地與漸密的距離色帶。這是靜態構圖檢視，不宣稱完成玩家手感驗收。
- 選定的右打構圖、Data／Native 分工與固定視窗政策見 [設計契約](../design/batting-feel.md)；當前所有 tuning 值只維護在 authored TOML。視窗標題與啟動 log 都明示 `right_handed`，log 也列出 Data path、Native owner、camera position／target／FOV。
- 最終 Debug／Release 均成功建置與執行於既有 RTX 5070 Ti；Windows API 檢查確認 client area **1280×720**，`WS_THICKFRAME` 與 `WS_MAXIMIZEBOX` 都未設定，minimize／restore 後尺寸仍相同，正常 `WM_CLOSE` 均以 exit code 0 結束。最後兩次執行各完成 3040／1106 frames，靜態 buffer 為 1557 vertices。
- 兩組態擷取確認五角本壘、低矮投手丘、投手板旁的金色高度標尺、青色 release 支柱／圓環及球 marker 可辨識。真正尺寸的投手板在低角度下仍很薄，標尺提供高度與站位參照，沒有以移近投手板或放大丘高度解決問題。
- 最終 Debug 的 D3D12 debug layer／GPU-based validation error/corruption 為 **0**；shutdown report 只列出用於回報的 device，未新增 live child resource。Engine rendering source 與 HLSL 均未修改。
- CTest 的 `batting_staging` 在 Debug／Release 皆通過：authored 檔可載入、空檔採明示 defaults、合法整數覆寫、缺檔，以及 18 組語法／型別／NaN／Infinity／越界／未知 key／錯誤 preset 案例。測試不要求 authored tuning 值與 Native fallback 永遠相同，避免調 Data 反而被迫重新改 C++。
- 另外對真正的 Debug app 實測缺檔、語法錯誤與 FOV=500：三者皆顯示錯誤對話框與來源診斷，關閉訊息後 exit code 1；未進入 D3D12 初始化，沒有 silent fallback 或 crash。

```bat
ctest --test-dir build/debug --output-on-failure
ctest --test-dir build/release --output-on-failure
```

原生 computer-use helper 仍在 sandbox 啟動失敗，因此沿用 process-targeted 檢查及 `PrintWindow` 擷取並實際檢視。暫存檢查腳本已移除；run logs／擷取圖保留於忽略的 `build/`。尚未測跨螢幕／DPI 變更、其他 GPU 或全部合法參數組合的構圖品質。仍沒有 runtime reload、左右打切換、投手模型／動畫、球移動或 simulation；delivery 2 未開始。


### 右投 vs 左打 staging 校正（2026-09-14）

沿用上述依賴、建置命令與啟動 Data 路徑，未新增 library／build 設定。固定顯示與左右側定義由 [設計契約](../design/batting-feel.md) 維護；以下是本次實測的 [authored TOML](../../pawapuro/batting/staging.toml) 快照，未來 tuning 仍以該檔為唯一來源。

| 項目 | 本次實測設定 |
|---|---|
| Preset | `right_handed_pitcher_vs_left_handed_batter`；視窗標題與 log 明示右投手 vs 左打者 |
| 視窗 | 固定 windowed 1920×1080、16:9；不能任意 resize／maximize |
| Camera | position `(0.75, 1.25, -5.0)` m；target `(0, 1.30, 16.8)` m；vertical FOV **36° 未變** |
| Release reference | `(-0.65, 2.05, 16.8)` m；X 改到捕手視角左側，與右投手基準相符並與中央標尺分開；球 marker radius 0.10 m 未變 |
| 平面紅土 | 本壘中心 `(X,Z)=(0,0)`、radius 2.8 m；投手丘外圍中心 `(0,17.9832)`、visual radius 5.5 m |
| Raised mound | 底部 radius 2.75 m、平頂 radius 0.9 m、高度 0.254 m 均未變；投手板距離仍為 18.4404 m |
| 中央標尺 | X=0、Z=18.5166 m（投手板長度中點），底端 Y=0.259 m（丘高加原有 0.005 m render offset）；高 2 m、每 0.5 m 一刻度 |
| 外野草地 | X=−85～85 m、Z=−12～140 m 與既有距離色帶未變 |

本輪只新增 `field.home_dirt_radius_m`（1.5～4 m）及 `mound.visual_dirt_radius_m`（3.5～7 m）兩個 Data 數值，沿用現有 defaults／finite number／範圍檢查。Camera、preset、release 是既有 Data 的修改。標尺位置是「投手板中心」的固定關係，留在 Native 推導；圓盤 tessellation／顏色／微小 depth offset 也留 Native，沒有新的編輯需求。局部 `dirt_disk` lambda 只共用本壘與丘外圍兩個當前 caller 的頂點展開，沒有建立 primitive／terrain API。

實際執行與檢視：

- Debug／Release 均以既有 MSVC x64、C++20、`/W4 /WX` 成功 build；兩組態 CTest 皆通過。測試涵蓋新紅土值確實覆寫、defaults、authored 檔、缺檔及 23 組不合法 Data，包含新增紅土範圍與錯側 release。
- 兩組態皆在 RTX 5070 Ti 開啟並持續呈現；實測 client area 為 **1920×1080**，沒有 `WS_THICKFRAME`／`WS_MAXIMIZEBOX`，minimize／restore 後尺寸仍相同。正常 `WM_CLOSE` 均 exit code 0，Debug／Release 分別完成 291／302 frames；靜態 buffer 為 1875 vertices。
- 實際擷取畫面確認本壘完整且更扁平，中央標尺與其左側 release／球 marker 分開可辨；較大的平面紅土包圍較小的隆起丘面，與本壘紅土之間是大片草地。外野色帶仍延伸至遠處。這是靜態視覺檢查，壓迫感與打擊體驗仍待使用者 review，不宣稱通過玩法驗收。
- Debug 的 debug layer／GPU-based validation 保持啟用，error/corruption **0**；shutdown 僅列出仍為報告保留的 device，沒有 live child resource。Engine rendering source／HLSL 未變。

本輪 computer-use helper 在 sandbox 初始化失敗，沿用只針對本次 app process 的 Windows API／`PrintWindow` 檢查；不是手動拖曳或點擊 X 的測試。暫存檢查腳本移除，畫面與 logs 保留於忽略的 `build/left-staging-debug.*`、`build/left-staging-release.*`。尚未測其他 GPU／DPI 情境與全部合法 Data 組合的構圖；沒有人物、animation、hot reload、runtime 左右打切換或球運動，delivery 2 仍未開始。


### Camera 對側修正（2026-09-14）

本次只將 authored camera position X 由 +0.75 改為 −0.75 m：最終 position `(-0.75, 1.25, -5.0)`、target `(0, 1.30, 16.8)`，FOV 仍為 36°；上述其餘 staging 設定不變。原 Native validation 只接受正 X，因此必要地將範圍改為 −1.5～1.5 m，並同步修改 fallback 與既有測試；沒有修改 geometry、Engine、shader 或依賴。對側構圖與暗沉感的後續處理見設計文件。

Debug／Release build 與 CTest 均通過；實際 client area 1920×1080、固定 windowed，正常關閉皆 exit code 0，分別完成 230／237 frames。Debug GPU-based validation error/corruption 為 0，shutdown 沒有 live child resource。沿用僅針對 app process 的 Windows API／PrintWindow 擷取，檢視本壘、中央標尺、release 與右側 foreground 空間；兩組態 client 畫面一致。尚無模型，不能據此承諾實際打者／揮棒不遮擋。暫存檢查腳本已移除，logs／畫面留在忽略的 `build/opposite-camera-debug.*`、`build/opposite-camera-release.*`；沒有開始 delivery 2。


## M1 Step 1／交付 2：Reference pitch（2026-09-14）

沿用既有 Debug／Release build 命令與兩個已固定依賴，未新增 library。操作與時間／Data／rendering 契約由 [設計文件](../design/batting-feel.md) 維護；執行後 Space release、P pause/resume、. single-step、Esc 退出，不能 rethrow。

| 實測 reference fixture | 結果 |
|---|---|
| 初始位置 | 沿用 staging release `(-0.65, 2.05, 16.8)` m |
| 唯一新增初始條件 Data | `reference_pitch.initial_velocity_mps = [1.655, -0.6, -41.667]`；向量長度約 150.14 km/h，不另存 speed／target |
| Native 時間／重力 | 240 Hz；dt=1/240 s；Y acceleration −9.80665 m/s²；最多 16 ticks/frame，保留欠帳 |
| Arrival | tick **95**；simulation time **0.395833333 s**；plane Z=0.4318 m |
| Arrival position | `(0.005105, 1.044226, 0.306804)` m；previous Z=0.480416 m |
| Arrival velocity | `(1.655000, −4.481804, −41.667000)` m/s |
| 離散判定限制 | 停在跨越後位置，本例 overshoot 約 0.125 m，Z 每 tick 約 0.174 m；不做 sub-tick 接觸判定 |

已驗證：

- Debug／Release 均以既有 MSVC x64 C++20、`/W4 /WX` build 成功。兩組態 CTest **2/2 通過**：既有 Data 測試（新增 velocity 型別／範圍案例，26 個不合法 fixture），以及新的 `reference_pitch` 測試。
- Simulation 測試對同一 Native reference 初始 state 重複 20 次固定 48 ticks，逐欄位精確相等；30／60／120 FPS 的奈秒分段在 0.2 s 得到相同 48-tick state，最後均 arrival tick 95。另測重力解析式容差、250 ms 長 frame 的 cap／欠帳補完、pause 保留 fractional credit、單步一 tick、Ready／Complete 不推進、arrival 恰好一次（包含 paused 單步抵達）。這不要求未來 authored Data 必須永遠等於 Native defaults。
- 實際 app 使用 process-targeted Windows key messages 經 SDL event loop 執行 Space → P → . → P：Debug paused tick **16** → step **17**，Release **20** → **21**；等待期間 title state 不變且 paused client 截圖逐像素相同，單步後畫面確實改變。恢復後兩者都停在 tick 95，Complete 再按 Space／. 不改 state。
- 實測固定 client area **1920×1080**、無 resize/maximize style；Debug 以 Esc、Release 以正常 WM_CLOSE 退出，exit code 皆 0。兩者 stderr 各只有一筆 release 與 arrival；最後執行各完成 **84／89 render frames**。
- 檢視 Ready、Paused、單步、InFlight 與 Complete 的擷取：球從圓環離開、向本壘靠近並停在本壘上方；release 圓環與場地保持固定。球中心僅來自 current simulation state，沒有 render-only trajectory。Camera／FOV／場地 Data 值保持前一 commit 基準，場景 vertex buffer 仍為 1875 vertices，改成固定區／平移區兩次 draw。
- Debug D3D12 debug layer／GPU-based validation error/corruption **0**；shutdown 只列出報告當下保留的 device，沒有 live child resource。沒有新增 GPU resource，原單一 frame-in-flight fence ownership 沿用。

Native UI helper 仍無法啟動；上述是真正 app 的 SDL 按鍵路徑與 PrintWindow 檢查，不是人類手動鍵盤測試。暫存檢查腳本移除，logs／擷取留在忽略的 `build/flight-*`。本例飛行約 0.4 秒與畫面可見性已核對，最終速度感／打擊體驗仍需使用者 review。無 drag／spin／Magnus、投手動畫、collision／CCD、rethrow／replay 或 hot reload；240 Hz 不構成未來 bat-ball contact 精度保證。其他 GPU、全部合法初始向量與低 FPS 的視覺平順度尚未驗證。


## 進壘 framing／同球重投驗證（2026-09-14）

前節保留 delivery 2 當時的單球結果；本節記錄目前新增的驗證畫面與操作迴圈。沒有新增依賴，Engine／HLSL／球路積分未改。固定 1920×1080、16:9 windowed、不可任意 resize；preset 仍是右投手 vs 左打者。

| 本次設定 | 實際採用值與作用 |
|---|---|
| Camera | position **(-0.75, 1.25, -5.0)** m 未變；target **(2.260052, 1.30, 16.8)** m，只改 X；vertical FOV **36°** 未變 |
| 水平對齊 | target X 由 `-0.75 + 0.75 × (16.8 + 5) / (0.4318 + 5)` 選定，使水平視線通過 arrival plane 的 X=0；沒有 camera tracking 系統 |
| 新增 Data | `strike_zone_reference.width_m=0.4318`、`bottom_m=0.5`、`top_m=1.3`；高 0.8 m、中心高 0.9 m，只是 provisional reference，需待打者模型／站姿校正 |
| Native reference | 框中心 X=0、Z=0.4318 m 共用 arrival plane；四條薄幾何的線寬 0.01 m 留在 Native，沒有目前的調參需求 |
| 保留值 | 其餘既有 TOML 值逐項比較均未變；initial velocity、release、gravity、240 Hz、投手丘／紅土／草地與顏色均未改 |

驗證：

- 沿用前述 x64 developer environment／UTF-8 命令，`cmake --build build/debug`、`cmake --build build/release` 成功；兩者的 `ctest --test-dir build/<configuration> --output-on-failure` 均 **2/2 通過**。Staging 測試涵蓋三值 override／default、錯誤型別／範圍／非有限值及上下界反轉（共 30 個非法案例）。
- 投球測試在同一個物件重投 **20 次**，每次檢查 initial state／tick／pending／pause 重置及新 fractional credit，逐 tick 與相同初始 fixture 精確比較，arrival 每球只發生一次；原 30／60／120 FPS chunking、catch-up、pause／single-step 測試保留並通過。這是同 build／平台比較，不是跨平台 bit identity 保證。
- 原生 computer-use helper 仍因 sandbox setup 失敗而無法啟動；改用只針對本次 app process 的 Windows key messages 經 SDL event loop 操作及 PrintWindow 擷取。Debug／Release client 實測皆 **1920×1080**，resize／maximize style 關閉；兩者各完成初投加 **20 次 Space 重投**，21 筆 release 與 21 筆 arrival 各自一致，不需重啟。
- Ready 截圖量測：藍框 X 範圍 **893～1025**，外框 bounding-box 中心 **959 px**；白色本壘 X 範圍 **895～1035**，bounding-box 中心 **965 px**；release 標記 X 範圍 **717～759**，中心 **738 px**。兩種 build 結果相同。框的幾何中心投影目標為 X=960；bbox 中心受透視及 rasterization 影響。本壘位於地面且 Z 範圍不同，因此容許此數個 pixel 差異。
- 實際檢視 Ready／Complete 擷取：參考框清楚置中，投手丘中央標尺與出手標記偏左，右側保留 foreground 空間，外野與草地仍延伸。Complete 的球位於框內中上部、水平接近中央；沒有以此實作好壞球規則。
- Debug pause **19→20 tick**、Release **15→16 tick** 的 single-step 成功；暫停等待期間 state 與 client 畫面逐像素相同，單步後畫面改變，P 恢復後抵達 Complete。
- 每球 arrival 仍為 **tick 95、0.395833333 s**；球心 **(0.005105, 1.044226, 0.306804)** m，速度 **(1.655000, -4.481804, -41.667000)** m/s。相對框中心約向 +X **0.51 cm**、高 **14.42 cm**，即水平近中、垂直中上；這是跨平面後的離散狀態，並非精確交平面位置。既有 overshoot 限制仍成立。
- Debug layer／GPU-based validation 啟用，**0 validation errors**；釋放後只列仍供報告使用的 Live ID3D12Device，沒有 live child resource。兩種 build 正常退出 **0**，各完成 572 frames。

操作以設計文件為準：Space 在 Ready 出手、Complete 立即重投；P 暫停／恢復、. 暫停單步、Esc 退出。沒有新增抽象：只有既有具體 staging 欄位、四條框幾何與 initial state 快照；rendering 仍直接讀權威球位置。

以上是實際 app 的自動按鍵／擷取驗證，不是人類手動試玩。真實打者 silhouette 的遮擋、最終好球帶上下界與速度感仍待後續 review；沒有模型、動畫、揮棒或新物理。暗沉感仍延後至人物／lighting 階段。暫存驗證腳本移除，畫面與 logs 留在忽略的 `build/framing-*`。


## Authoritative gameplay strike zone 驗證（2026-09-15）

本次依 [gameplay-first／好球帶設計決策](../design/batting-feel.md#gameplay-first-原則)，將 `[strike_zone_reference]` 改為唯一的 `[strike_zone]`；width=**0.8636 m**、bottom=**0.5 m**、top=**1.3 m**。前節的 provisional 用語與窄框數值是歷史紀錄，不是另一份 runtime 規則。Native fallback 同步更新；width validation 採 0.25～1.5 m，可試 0.75／0.86／0.95 等候選。下限使用可精確表示的 0.25，避免原 0.2f 與 TOML double 的邊界比較差異；未改共用數值 parser。

- Debug／Release build 均成功，CTest 各 **2/2**；新增 section／0.8636 載入、可調寬度及上下限測試通過，**32 個非法案例**包含舊 section 與超出寬度上限。既有 20 次 deterministic rethrow、30／60／120 FPS chunking、pause／single-step、arrival once 均通過，simulation 測試與 source 未改。
- 兩種 app 的實際 **1920×1080** client 擷取均量到藍框外緣：**left=827、right=1089、top=527、bottom=774 px**；以含兩端 pixel 計算為 **263×248 px**，bounding-box center X=**958**，接近 960。前版為 133×247 px，寬度約 **1.98 倍**、比例由狹長變為近方形。世界高度仍為 0.8 m；外框高度多 1 pixel 是寬度延伸後透視／rasterization 的結果，沒有調整上下界。
- Ready 圖與前版逐像素比較，所有差異都位於新／舊藍框 pixel；本壘、球場、相機構圖、release marker 完全不變。本壘 physical width 仍為 **0.4318 m**。視覺檢視 Ready／Complete：投手偏左、右側仍有空間，球在唯一好球帶中水平近中央、垂直中上，未加第二個內框。
- Debug／Release 均操作 Space → P → . → P，再連續 **20 次重投**；pause tick **19→20**，等待暫停時畫面逐像素一致，單步後畫面改變。每種 build 的 21 筆 arrival 與前版 log 完全一致：**tick 95、0.395833333 s**，position **(0.005105, 1.044226, 0.306804)** m、velocity **(1.655000, −4.481804, −41.667000)** m/s；仍是跨 arrival plane 後的離散狀態。
- Debug layer／GPU-based validation **0 errors**，沒有 corruption 或 live child resource；報告只保留供報告使用的 device。Debug／Release 正常退出 0，分別完成 614／628 frames。
- 原生 UI helper 仍因 sandbox setup 失敗，沿用 process-targeted Windows key messages 經 SDL event loop 與 PrintWindow 擷取；這是實際 app 自動操作，不是人類手動試玩。暫存腳本已移除，擷取與 logs 留在忽略的 `build/gameplay-zone-*`。最終遊戲性與模型遮擋仍待人類 review。

本輪沒有新增 dependency、abstraction、判定、aiming、模型或動畫；camera／FOV／window、release／velocity／gravity／240 Hz、arrival plane、重投邏輯皆未改。


## 本壘／evaluation plane／overlay 一致性驗證（2026-09-15）

本次規則取代前節「本壘維持真實寬度」的歷史決策；唯一 gameplay width 同時驅動本壘與好球帶，設計契約見 [Batting Feel](../design/batting-feel.md)。

| 本次設定 | 實際值 |
|---|---|
| Strike-zone Data | width=0.8636、bottom=0.5、top=1.45 m；只改 top，為首個稍高 candidate |
| Home plate | width=depth=0.8636 m；X=±0.4318、tip Z=0、肩點 Z=0.4318、front Z=0.8636；保留原五角形等比例放大 |
| Evaluation plane | depth/2=0.4318 m，穿過本壘前後中央。與舊 Z 相等是本次尺寸巧合，不再使用舊前緣常數 |
| Camera／window | position=(-0.75,1.25,-5)、target=(2.260052,1.30,16.8)、FOV=36°、1920×1080，皆未改 |
| 其他保留值 | TOML 逐項比較除 top 外均相同；release／velocity、gravity／240 Hz／積分、場地與 marker size 未變 |

畫面與 prediction：

- Debug／Release 實際 client 均為 1920×1080。藍框外緣 **left=828、right=1088、top=481、bottom=774 px**（含 stroke）；含兩端尺寸 **261×294 px**、center X=**958**、寬／高 **0.888**，高／寬 **1.126**。幾何中心線的 projected bounds 為 left=828.902、right=1088.303、top=482.503、bottom=773.643。Screenshot 可見四邊完全水平／垂直，無 depth 遮蔽；本壘明顯加寬且五角形仍可辨，投手偏左、右側空間保留。
- Prediction 跑相同固定 tick 積分，再對 crossing tick 作相同線性 evaluation。預測 world sample 與 actual evaluation 皆為 **(0.000140, 1.057610, 0.431800)** m、time **0.392833450 s**、velocity **(1.655000, −4.452385, −41.667000)** m/s。兩者 screen **(960.041992, 602.092529)** px；**ΔX=0、ΔY=0、Euclidean error=0 px**（同 build/platform）。
- 橘色空心環的 screenshot 外緣為 X=913～1006、Y=555～648，外框中心 (959.5,601.5)；stroke／rasterization 的 bbox 中心不是 world sample 的精確投影。環中央不填色，Complete 球仍清楚可見。
- 完整 tick 的 authoritative current state 仍在 **tick 95、0.395833333 s**：position **(0.005105,1.044226,0.306804)** m、velocity **(1.655000,−4.481804,−41.667000)** m/s，與前版相同。Rendering 保留此 state，不吸附 plane；球心 screen **(966.869324,607.582703)**，相對預測環中心 **Δ=(6.827332,5.490174) px**，距離約 **8.761 px**。這是完整 tick 與 crossing sample 的差異，不是 prediction 跑另一套物理。沒有宣稱 CCD 已解決。
- Overlay 隨啟動 camera 投影；本輪無 runtime camera 調整。視覺驗證只涵蓋當前 candidate，正式遊戲是否顯示 prediction／如何顯示與人物遮擋仍待 review。

建置與回歸：

- 沿既有 x64 developer environment 執行 Debug／Release build 成功，CTest 各 **2/2**。既有 Data validation、20 次 deterministic rethrow、30／60／120 FPS chunking、pause／single-step、arrival once 保留。
- 新測試檢查實際場景 plate vertices 的 width／depth 與 Data 一致；width=1.5 時 plane=0.75，arrival 改為 tick 93，證明 crossing 不再依賴舊常數。Baseline 與改寬案例的 prediction／actual evaluation 相同，baseline projected screen coordinate 也相同。
- 實際兩種 app 均 Space → P → . → P，pause tick 19→20；等待暫停時 client 畫面逐像素相同。各完成初投加 20 次重投，每球 raw arrival／plane evaluation log 一致，正常 exit 0。Debug GPU-based validation **0 errors／corruption**，shutdown 沒有 live child resource；只保留供報告的 device。
- 原生 UI helper 再次啟動失敗，沿用只針對 Pawapuro process 的 Windows key messages／SDL event loop 與 PrintWindow；這是實際 app 自動操作，不是人類手動試玩。暫存驗證腳本移除，擷取與 logs 保留於忽略的 `build/overlay-*`。
- Renderer 只增加 depth-disabled PSO 與第三段 NDC draw，共用原 vertex buffer、shader 與 fence ownership；沒有新增 dependency、UI／camera／physics framework 或新 HLSL。Prediction 與 geometry 推導均留在 Pawapuro。


## 球尺寸與靜態人物 blockout 驗證（2026-09-15）

設計目的與 Data 邊界見 [Batting Feel](../design/batting-feel.md)。本次沒有修改 camera、好球帶、本壘、release、初速、gravity、240 Hz 或 evaluation／rethrow 邏輯，也沒有修改 renderer／shader 或新增 dependency。

| 設定 | 本次候選（公尺） |
|---|---|
| Ball visual radius | 0.085，較 0.10 縮小 15%；prediction 環不另存半徑 |
| 右投手腳底原點／總高 | (0, 0.259, 18.5166)／1.85；站在投手板中央，頭寬約 0.851、頭高約 0.814 |
| 左打者腳底原點／總高 | (1.25, 0.008, 0)／1.70；頭寬約 0.782、頭高約 0.748 |
| Bat 靜態端點 | grip=(0.944, 1.062, −0.204)、tip=(1.454, 2.014, −0.578)，長約 1.143；端點由打者位置／高度及 Native 比例推導 |

- Debug／Release build 成功，CTest 各 **2/2**。新增角色位置／高度 override、default 與非法值檢查；staging 共 **36 個非法案例**通過。既有 20 次 deterministic rethrow、30／60／120 FPS chunking、pause／single-step、arrival once 測試全部保留。
- 兩種 app 實際 client 均 **1920×1080**，各操作初投加 **20 次重投**；pause 19→20 tick 的 single-step 成功，暫停等待時畫面逐像素相同。Ready、暫停在 tick 48 的 mid-flight、Complete 擷取留於忽略的 `build/blockout-debug-*.png` 與 `build/blockout-release-*.png`，logs 同前綴。暫存驗證腳本已移除。
- Ready 橘環 raster bounds 為 **X=934～985、Y=576～627**，外徑 **52×52 px**；Complete 球可見 bounds 為 **X=941～992、Y=581～633**，**52×53 px**。兩種 build 相同；球的 faceting、rasterization 及完整 tick 深度造成約 1 px 尺寸差，沒有另一份 marker radius。
- 實際檢視三張畫面：投手站在 mound，雙手準備姿勢與大頭短肢 silhouette 可辨；release 在人物左上方清楚可見。左打者與斜向後上的球棒在畫面右側，與置中的好球帶保留間隔，未遮住主要球路。Mid-flight 球接近投手帽頂的投影位置，仍可見；這是目前靜態構圖觀察，不是完整動作遮擋驗收。好球帶與 Q 版頭身比例、人物造型仍需人類實玩 review。
- 每種 build 的 21 筆 raw arrival／plane evaluation 與前版 log 完全一致：raw tick **95**、time **0.395833333 s**、position **(0.005105,1.044226,0.306804)**；plane sample **(0.000140,1.057610,0.431800)**。Prediction／actual plane sample 的 pixel error 仍為 **0**；Complete raw 球心與環中心約 **8.761 px** 的既有差距仍保留，原因見前節。
- Debug layer／GPU-based validation **0 errors**，未見 corruption 或 live child resource；shutdown 報告只保留供報告的 device。Debug／Release 正常 exit 0，分別完成 **603／599 frames**。
- 原生 computer-use helper 本次啟動失敗；沿用只針對 Pawapuro process 的 Windows key messages 經 SDL event loop 與 PrintWindow。以上是實際 app 自動操作／畫面檢視，不是人類手動試玩；沒有開始正式人物 pipeline、揮棒或新物理。


## Field readability／presence pass 驗證（2026-09-15）

本次設計契約見 [Batting Feel](../design/batting-feel.md)，以下為最終 candidate 與實測；沒有開始 animation pipeline。

| 項目 | 最終設定 |
|---|---|
| 右投手 | 腳底 (0, 0.655, 18.5166) m；總高 2.45 m（原 1.85），等比例放大；頭寬約 1.127、頭高 1.078 m，既有準備姿勢不變 |
| Raised mound | 底部 radius=3.5、平頂 radius=1.5、height=0.65 m；原為 2.75／0.9／0.254。外圍平面紅土 radius=5.5 m 不變，丘中心／投手板 X、Z 不變 |
| Camera | position=(-0.75,1.25,-5) 不變；target=(2.260052,2.10,16.8)，只將 Y 由 1.30 提高 0.80 m；vertical FOV=36°、1920×1080 不變 |
| 打擊區白線 | 兩側各寬 1.2、深 2 m，內緣離 plate side 0.18 m，Z=−0.6～1.4；白線寬 0.05 m |
| 一／三壘 | 中心 X=±27.432/√2、Z=27.432/√2（約 ±19.397／19.397 m），邊長 0.46、頂高 0.10 m；保留 90° diamond 方向，位於本視角外 |
| 界外線／牆 | 界外線沿 X=±Z；可見 chalk 從 (±2,2) m 起，以免穿過打擊區。外野牆為本壘尖端半徑 110 m、±45° 弧段，高 3.5 m，頂緣色帶高 0.12 m；無碰撞或 HR rule |

- Data 只新增 `mound.height_m`；既有 mound radius／top radius、pitcher position／height、camera target 調整。其餘 authored release、初速、球半徑、strike zone、草地／紅土與 batter Data 逐項比較相同；simulation／prediction source、renderer、shader 未改。Native fallback 保留原來的安全 camera／角色候選，新增 mound height fallback=0.254 m；省略欄位會明確記錄 default。
- Debug／Release build 成功，CTest 各 **2/2**。Staging 新增丘高 override／非有限值／零高度拒絕，擴大平頂 override 通過，共 **38 個非法案例**；既有 deterministic 20 rethrows、30／60／120 FPS chunking、pause／single-step、arrival once 通過。
- 兩種 app 實際 client 均 **1920×1080**；各完成初投加 20 次重投，pause **11→12 tick** 單步，暫停等待畫面逐像素相同。Ready／tick 48 暫停的 mid-flight／Complete 擷取位於忽略的 `build/presence-debug-*.png` 與 `build/presence-release-*.png`，logs 同前綴；暫存操作腳本已移除。
- 實際三狀態畫面可見：投手更厚實、raised mound 斜坡／平頂清楚，兩側打擊區與界外線可辨，遠端牆提供外野邊界。投手帽頂至鞋底的色塊 pixel 範圍由 **Y=483～614（132 px）** 增為 **472～646（175 px）**；打者鞋底由 **Y=963** 移至 **1028**，本壘尖端到 **Y=1015**。左側打擊區靠後白線部分被畫面裁切；這是保留近景臨場感的當前 candidate，不代表完整球場俯視驗收。一／三壘未強迫入鏡，依使用者選擇以界外線表達方位。
- 好球帶外框 **X=827～1089、Y=542～836**，含兩端 **263×295 px**、center X=**958**。Prediction 環外緣 **X=934～985、Y=637～687**，**52×51 px**；Complete 球 **X=941～992、Y=642～694**，**52×53 px**，尺寸仍接近。預測／actual plane sample 投影皆 **(960.042114,662.669617)**，pixel error=**0**。Raw tick 球心 **(966.884094,668.182678)**，既有離散 crossing gap 約 **8.787 px**；沒有改 physics 來吸附 marker。
- 各 build 的 21 筆 raw arrival 與前版 log 完全相同：tick **95**、t=**0.395833333 s**、position **(0.005105,1.044226,0.306804)** m、velocity **(1.655,−4.481804,−41.667)** m/s。Plane evaluation world/time/velocity 也完全相同。Mid-flight 球投影至投手身前仍可見；打者與 bat 未遮住好球帶／主要球路，正式出手或揮棒遮擋仍未驗證。
- Debug GPU-based validation **0 errors**，未見 corruption；shutdown 無 live child resource，報告只留供報告的 device。兩種 app 正常 exit 0，各完成 **619 frames**。
- Computer-use helper 因 sandbox setup 失敗，沿用只針對 app process 的 Windows key messages／SDL event loop／PrintWindow；量測程式採 DPI-aware client pixel。這是實際 app 自動操作與畫面檢視，人物存在感與最終玩法仍待人類 review。沒有新增 dependency 或 renderer／character／stadium abstraction。


## Pre-animation staging correction 驗證（2026-09-15）

使用者實玩指出 0.65 m mound 像高台、好球帶在 Q 版打者旁偏高。設計決策與單一靜態 release pose 的界線見 [Batting Feel](../design/batting-feel.md)；本次沒有正式 rig／skeleton／animation。

- 實際以同一 camera、zone 與初版 release pose 擷取 **0.30／0.35／0.40 m** 三個丘高（1920×1080，`build/release-pose-0.xx-ready.png`）。0.30 m 較平、0.40 m 丘面略突出，選 **0.35 m** 作為仍可辨斜坡且不再像高台的中間候選。底部 radius **3.5 m**、平頂 radius **1.5 m**、visual dirt radius **5.5 m** 均未變；選定後只再澄清兩腳的橫向 silhouette，沒有進行動畫。
- Camera 完全不改：position **(−0.75,1.25,−5)**、target **(2.260052,2.10,16.8)**、FOV **36°**。前次 target Y 提高所保留的本壘／腳底近景仍成立，不因降低丘高再調整。左右打擊區、界外線、壘包、外野牆及打者 Data／幾何皆保留。
- Authoritative zone 為 **width=0.8636、bottom=0.30、top=1.25 m**，高度仍 **0.95 m**；Data 只下移上下界，不改 width／本壘／evaluation plane。截圖外框 **X=827～1089、Y=603～898**，**263×296 px**、center X=**958**、高／寬約 **1.125**；前版為 263×295 px、Y=542～836，1 px 高差來自投影／rasterization。
- 畫面顯示下移後頂邊仍與打者大頭下半部同高。使用者已選擇保留 **0.30～1.25 m** 候選，頭身比例關係留待 review；不宣稱頂邊已完全低於頭部，也不判定本球為好／壞球。右側 X≥1150 的 Ready 畫面與前版逐像素相同，打者與 bat 未被偷偷改動。

Release fixture 的 world landmarks（公尺，腳底點為鞋底中央，其餘為幾何中心）：

| Landmark | (X, Y, Z) |
|---|---|
| 原點／站立比例 scale | (0, 0.355, 18.5166)／2.45 m；原點 X/Z 不改，Y 隨降低的丘面調整 |
| 後腳鞋底中央 | (−0.1715, 0.355, 18.5166)，仍與投手板相交 |
| 前腳鞋底中央 | (0.3675, 0.355, 16.8506)，向本壘跨出 1.666 m；兩腳均在平頂範圍 |
| Pelvis／chest | (0, 1.188, 17.9531)／(−0.049, 1.7515, 17.5366)；胸口比 pelvis 再前移 0.4165 m |
| 頭中心 | (0.049, 2.1925, 17.5366)，保留原大頭比例 |
| 右肩／右肘 | (−0.392, 1.7515, 17.5366)／(−0.735, 1.972, 17.1936) |
| 右手中心 | (−0.65, 2.015, 16.93) |
| Release（不變） | (−0.65, 2.05, 16.8)，距右手中心 **0.134629 m** |

手中心與球心分開，手位於球後方／略下，Ready 可見手與球接近；Mid-flight 球離開手部且可追蹤，Complete 關係不變。主視角會縮短前跨與胸口前傾的 Z 深度；本次只證明端點能連到現有 release 幾何，不能以靜態圖冒充動態 body mechanics、連貫動作或投球節奏驗收。所有 pose 比例留 Native，沒有新增 Data schema；只有 mound height、pitcher origin Y、zone bottom／top 使用既有 Data 調整。

- Debug／Release build 成功，CTest 各 **2/2**。保留全部 staging validation、20 次 deterministic rethrow、30／60／120 FPS chunking、pause／single-step、arrival once、prediction consistency；新增獨立 release/body 端點重合時拒絕建立幾何的測試，避免 normalize 零向量。
- 最終兩種 app 各完成初投加 **20 次重投**，pause **12→13 tick** 的單步成功，暫停等待期間畫面逐像素相同。Ready／暫停 tick 48 的 Mid-flight／Complete 皆為 1920×1080，留於忽略的 `build/release-pose-debug-*.png` 與 `build/release-pose-release-*.png`。
- 各 build 的 **21 筆 raw arrival 與 plane evaluation 完整 log 與前版逐行相同**：tick **95**、time **0.395833333 s**、position **(0.005105,1.044226,0.306804)**、velocity **(1.655,−4.481804,−41.667)**。Plane sample **(0.000140,1.057610,0.431800)**、time **0.392833450 s**；prediction／actual plane pixel **(960.042114,662.669617)**、error **0 px**。Raw 球心仍為 **(966.884094,668.182678)**。Camera／球半徑／prediction logic 未改，環與球的尺寸關係也未改。
- Debug GPU-based validation **0 errors**，未見 corruption，shutdown 無 live child resource（僅供報告使用的 device）。Debug／Release 正常 exit 0，分別完成 **613／616 frames**。
- Computer-use helper 因 sandbox setup 失敗，沿用只針對 Pawapuro process 的 Windows key messages／SDL event loop／PrintWindow；這是實際 app 自動操作及畫面檢視，不是人類手動試玩。暫存操作腳本移除，候選 Data／截圖／logs 留在忽略的 `build/`。沒有新增 dependency、renderer 或 character／animation framework。


## Off-axis batting projection 驗證（2026-09-15）

本次只修改 camera/projection，設計契約見 [Batting Feel](../design/batting-feel.md)。Camera position **(−0.75,1.25,−5)**、target **(−0.75,2.10,16.8)**、vertical FOV **36°**；只改 target X，水平 viewing direction 為 **+Z（yaw=0）**，vertical pitch 約 **2.233°**。人物／球場／好球帶 Data 與 geometry、physics、prediction／控制邏輯均未改。

- 共用 `batting_view_projection()` 改用 `XMMatrixPerspectiveOffCenterLH`，lens shift 從 view-space gameplay focus 推導，沒有新增 authored Data。此候選近面 Z=0.1 m，near-plane horizontal offset 約 **+0.01386535 m**，約為水平半寬的 **0.240036**；正向 frustum offset 將影像往左移，使偏右的 gameplay focus 置中。Native camera fallback 的 target X 同步改為 −0.75，其餘 fallback 不動。
- Debug／Release build 成功，CTest 各 **2/2**。新增 camera X=−1.25／−0.75／0、aspect=16:9／4:3 的 projection 契約測試：focus 置中、overlay bbox 中心在 1 px 內、兩側打擊區前後 X 向橫線 ΔY<1e−6 NDC。最初把 bbox 中心當成幾何中心的過嚴 tolerance 揭露 vertical pitch 深度差，已改為明確 pixel tolerance；沒有修改幾何或另補 overlay offset。
- 原有 20 次 deterministic rethrow、30／60／120 FPS chunking、pause／single-step、arrival once、prediction consistency 與 Data／退化 fixture tests 均通過。兩種實際 app 各初投加 **20 次重投**，pause **11→12 tick**，暫停等待畫面逐像素相同。

實際 **1920×1080** Ready 截圖量測（Debug／Release 結果相同）：

| 橫線 | 左端 pixel | 右端 pixel | ΔY |
|---|---|---|---|
| 畫面左側打擊區後線 | (323,1076) | (781,1076) | **0 px** |
| 畫面右側打擊區後線 | (1251,1076) | (1709,1076) | **0 px** |

以上採後線中心掃描列 Y=1076 的可見白色筆畫範圍，包含轉角接合的 rasterization；整條後線的主要厚度在 Y=1073～1078，未被畫面下緣裁切。相同 world endpoints 的解析投影交叉核對：左側約 **(323.772,1075.998)→(782.382,1075.998)**，右側約 **(1250.011,1075.998)→(1708.621,1075.998)**；解析值與實際筆畫端點差約 1 px，沒有把計算值冒充 screenshot pixel。前方橫線也水平，左側解析端點約 **(451.541,927.635)→(765.752,927.635)**，截圖可見水平白線；右前方部分被既有打者遮住。

- 好球帶外框 screenshot bounds **left=826、right=1094、top=604、bottom=898**，**269×295 px**、center X=**960**。維持 axis-aligned、略高於寬，與物理場景使用同一投影來源；沒有將 overlay 變回 world-space draw。
- Prediction／actual plane evaluation pixel 均 **(959.574707,663.836792)**，**ΔX=0、ΔY=0、Euclidean error=0 px**。Raw tick 球心為 **(966.581604,669.439392)**，既有完整 tick 與 plane sample 的差異仍保留，不吸附 marker。預測環 **52×52 px**，Complete 球 **53×53 px**；差異來自 raw state 深度／faceting／rasterization。
- 每種 build 的 **21 筆 authoritative raw arrival 與 evaluation world/time/velocity** 與前版相同：tick **95**、time **0.395833333 s**、position **(0.005105,1.044226,0.306804)**、velocity **(1.655,−4.481804,−41.667)**；plane sample **(0.000140,1.057610,0.431800)**、time **0.392833450 s**。只有 projected screen coordinates 改變。
- Ready／mid-flight（暫停 tick 48）／Complete 圖可見投手仍偏左、打者與 bat 在右側，主要球路／release 不受遮擋，本壘／腳底仍靠近下緣。本次 projection 消除橫線傾斜，但沒有消除正常透視或改人物比例；正式動畫尚未開始。
- Debug GPU-based validation **0 errors**，未見 corruption；shutdown 無 live child resource（僅保留供報告的 device）。Debug／Release 均正常 exit 0，各完成 **616 frames**。
- 原生 computer-use helper 本次啟動失敗；沿用 process-targeted Windows key messages／SDL event loop／DPI-aware PrintWindow。這是實際 app 自動操作／畫面檢視，非人類手動試玩。截圖／logs 留在忽略的 `build/offaxis-debug-*`、`build/offaxis-release-*`；暫存驗證腳本已移除。沒有新增 dependency、renderer abstraction 或 cinematic-camera framework。


## Character Style v1 靜態校正（2026-09-15）

長期造型規則見 [Batting Feel](../design/batting-feel.md)。本次只校正兩個既有 blockout，沒有正式 animation pipeline。

- 共用 `[character_style]` 無單位倍率：head **1.08**、foot **1.6**、hand **1.3**、bat thickness **1.25**；允許範圍分別 0.8～1.25、1～2、0.75～1.75、0.75～1.75。只有啟動載入，沿用 defaults／finite／型別／範圍診斷。
- Pitcher 原點 **(0,0.355,18.5166)** m、比例 scale **2.45 m**；頭完整 XYZ 尺寸約 **1.217×1.164×1.111 m**，球形手直徑 **0.287 m**，單鞋 **0.666×0.353×0.784 m**。Torso 兩端直徑 0.784／0.735 m，使用較緊湊軀幹與短褲。後／前腳 XZ、胸口、肩肘、手中心與 release 關係不變，鞋放大時底面維持原高度。
- Batter 原點 **(1.25,0.008,0)** m、比例 scale **1.7 m**；頭 **0.845×0.808×0.771 m**，球形手直徑 **0.199 m**，單鞋 **0.462×0.245×0.653 m**，torso **0.578×0.544×0.476 m**。頭頂含帽約 Y=1.739 m；height 欄位是比例 scale，不宣稱它仍是精確總高。兩個球形手沿 bat 分開，讓雙手握棒可辨；bat grip／tip 保留 **(0.944,1.062,−0.204)／(1.454,2.014,−0.578)** m，長約 **1.143 m**，直徑由握端 **0.0765 m** 漸增至 **0.14875 m**。
- Native 保留軀幹／短褲、帽簷、臉部、手腳位置與固定 pose 比例；移除細腿連接，短褲與鞋的垂直空隙約投手 **0.162 m**／打者 **0.112 m**。這是刻意的簡化身腳關係，沒有加入 hierarchy 或 character abstraction。
- Debug／Release build 成功，CTest 各 **2/2**；staging 測試涵蓋四倍率載入及非法值，非法案例共 **42**。既有 20 次 deterministic rethrow、30／60／120 FPS chunking、pause／single-step、arrival once、prediction／projection 契約均通過。
- 兩種實際 app 各完成初投加 **20 次重投**；pause **11→12 tick** 單步，暫停等待畫面逐像素相同，正常 exit 0。Ready／暫停 tick 48 的 Mid-flight／Complete 擷取均為 **1920×1080**，留在忽略的 `build/style-v1-debug-*.png` 與 `build/style-v1-release-*.png`，log 同前綴。
- 實際圖可見兩者共用大頭／大鞋／簡單手與緊湊軀幹語言；打者雙手、粗 bat 與身腳間距清楚，仍在右側，沒有遮住 zone、release 或主要球路。投手跨步 Z 深度仍受正面透視壓縮；這是靜態 silhouette review，不能代替未來動作驗收。Zone 與打者大頭的高度關係保留待 review，未為配合人物修改規則。
- Camera／field／zone／ball Data 未改，simulation、prediction 與 renderer source 未改。兩種 build 各 21 筆 release／arrival／evaluation 紀錄與前版完全一致：arrival tick **95**、t=**0.395833333 s**、position **(0.005105,1.044226,0.306804)**、velocity **(1.655,−4.481804,−41.667)**；plane prediction／actual pixel **(959.574707,663.836792)**，error **0 px**。
- Debug GPU-based validation **0 errors**，未見 corruption；shutdown 無 live child resource（僅供報告使用的 device）。Debug／Release 分別完成 **609／608 frames**。原生 computer-use helper 啟動失敗，沿用僅針對 app process 的 Windows key messages／SDL event loop／DPI-aware PrintWindow，非人類手動試玩。暫存操作腳本已移除；沒有新增 dependency、renderer／character／animation architecture。


## 扁腳／帽冠／單段手臂與紅中 fixture（2026-09-15）

本次規則見 [Character Style v1](../design/batting-feel.md)。沒有 rig／skeleton／animation 或新投球物理。

- 實際 1920×1080 比較 foot planar／height **1.9／0.9** 與 **2.1／0.7**，擷取於 `build/calibration-feet-a-ready.png`／`calibration-feet-b-ready.png`。選 **1.9／0.9**：X/Z 各比前版增大 **18.75%**、Y 減少 **43.75%**；第二組近景更像薄片，第一組仍保有鞋的體積。投手單鞋 XYZ 約 **0.79135×0.19845×0.931 m**，打者 **0.5491×0.1377×0.7752 m**，鞋底 Y 與角色原點不變。
- `[character_style]` 以 `foot_planar_scale`（1～2.5）、`foot_height_scale`（0.5～1.5）取代單一 `foot_scale`；舊 key 拒絕，不建相容層。Head=1.08、hand=1.3、bat thickness=1.25 均未改。只新增一個必要自由度，仍沿用 startup load／default／finite／range validation。
- 帽子沿用橢球：crown radii 為 **(0.255,0.155,0.235)×head_scale×角色 scale**，中心高於頭中心 **0.12×head_scale×角色 scale**，與頭上半部相交；帽簷 radii **(0.23,0.022,0.16)×head_scale×角色 scale**，高度在頭中心上方 0.075 倍、向前偏 0.22 倍。投手朝 −Z、打者朝 +Z，沿用其既有小幅 lateral offset。Crown／brim 比例留 Native，未新增帽子 Data 或 accessory API。投手 crown 完整 XYZ 約 **1.349×0.820×1.244 m**，打者 **0.936×0.569×0.863 m**。
- 所有手臂改為 shoulder-to-hand／glove 的單一 tapered segment，去除肘端點與兩段接合。Throwing arm／batter arm 半徑為角色 scale 的 **0.055→0.04**，glove arm **0.055→0.045**；手與 bat 端點不變。這是 rubber-like 視覺規則的靜態近似，尚無 bending、curve 或 deformation。

紅中初速推導（只算一次寫回既有 velocity Data，不加入 runtime solver）：

- 目標 **(0,0.775,0.4318) m**；release **(−0.65,2.05,16.8) m**，保留 `vz=−41.667 m/s`、`g=−9.80665 m/s²`、`dt=1/240 s`。
- `t=(target_z-release_z)/vz=0.3928336573307414 s`；`a=fract(t/dt)=0.28007775937793156`。現有 evaluation 在相鄰 constant-acceleration ticks 間做線性 interpolation，因此重力位移為 `0.5*g*(t²+a*(1-a)*dt²)`；保留這個小項，而非假裝 evaluation 已是精確連續拋物線。
- `vx=(target_x-release_x)/t`，`vy=(target_y-release_y-重力位移)/t`。寫入 **(1.654644371,−1.319413788,−41.667) m/s**，總速 **150.194554 km/h**。Runtime 只有這份 velocity，沒有第二份 target／speed Data；TOML 浮點與逐 tick float rounding 的殘差交由實際 simulation 測試。
- CTest 現在讀正式 staging TOML；中心誤差容許每軸 **0.1 mm**、投影中心誤差每軸 **0.05 px**，不是測死某組 velocity。保留全部 20 rethrows、30／60／120 FPS chunking、pause／single-step、arrival once、prediction／projection tests；退化 arm 測試改檢查 shoulder／hand 重合。Debug／Release build 成功，CTest 各 **2/2**；staging 的 **44 個非法案例**通過。

實際與預測結果（兩種 build 一致）：

| 項目 | 結果 |
|---|---|
| Prediction／actual interpolated evaluation XYZ | **(−0.000000450,0.775000632,0.431800008) m**，兩者相同 |
| 對設計中心 world error | 約 **0.00000078 m**；prediction vs actual error=0 |
| Evaluation time／velocity | **0.392833450 s**／**(1.654644,−5.171800,−41.667) m/s** |
| Raw Complete state | tick **95**、t=**0.395833333 s**、p=**(0.004963,0.759459,0.306804)**、v=**(1.654644,−5.201219,−41.667)** |
| Gameplay centre 投影 | **(959.999939,750.853821) px** |
| Prediction／actual evaluation 投影 | **(959.999817,750.853638) px**，兩者 ΔX=ΔY=distance=**0 px** |
| Prediction 對 gameplay centre 投影誤差 | 約 **0.000220 px** |
| Screenshot zone bounds／centre | **X=826～1094、Y=604～898**／**(960,751) px**，與前版相同 |
| Screenshot 橘環 bounds | **X=934～985、Y=725～776**；raster 外緣中心 (959.5,750.5)，與解析球心的 subpixel／筆畫取樣差小於 1 px |

Complete 畫面仍畫完整 crossing tick 的球，球心 **(967.034790,759.215210) px**，不是 interpolated evaluation sample。它比紅中環約前移一小段；未修改 rendering 或 physics 使其吸附。此差異與真正 evaluation error 分開回報。

- Debug／Release 實際 app 各完成初投加 **20 次重投**，每次 release／arrival／evaluation 紀錄各自相同。Pause 等待畫面逐像素不變，single-step **11→12 tick**，正常 exit 0，各完成 **608 frames**。Ready／暫停 tick 48 的 Mid-flight／Complete 三張 1920×1080 擷取在 `build/calibration-debug-*.png`／`calibration-release-*.png`，log 同前綴。
- 實際圖可見更扁的鞋底、包住頭頂的帽冠與單段手臂；兩手握棒、release 球及主要球路仍可辨。帽冠與手臂仍是低細節 static fixture，不宣稱完成動態可讀性。Camera／projection、好球帶、場地、人物原點與 ball radius Data 逐項比對未改；reference_pitch.cpp/.hpp、renderer、main loop 均未改。
- Debug GPU-based validation **0 errors**，未見 corruption；shutdown 無 live child resource（僅供報告的 device）。原生 computer-use helper 啟動失敗，沿用 process-targeted Windows key messages／SDL event loop／DPI-aware PrintWindow；這是實際 app 自動操作與畫面檢視。候選／截圖／logs 留在忽略的 build，暫存驗證腳本移除。沒有新增 dependency 或 animation architecture。


## Final Pre-Rig Character／Vertical Composition（2026-09-15）

起始 main／origin/main 均為 `da2d6bdf1c1699352f26db60e6ff8510c2d62d99`，workspace 乾淨。依序擷取未修改 baseline、原 camera＋衣襬、衣襬＋新 camera；不是同時改完再推測原因。設計與 human review gate 見 [Batting Feel](../design/batting-feel.md)。

- 衣襬只改 `reference_scene.cpp` 的兩個既有 fixture。投手保留原 chest rim，向下接到 Y=角色原點＋0.30×scale 的水平橢圓衣襬，半徑 X/Z=0.175／0.155×scale；第一次沿傾斜軸延伸仍露出碎片狀褲緣，因此改成水平截面。打者保留原上半橢球，從 equator 接至 Y=0.31×scale、半徑 X/Z=0.16／0.135×scale 的衣襬，取代下半收尖。沒有另一顆 pelvis、裙襬擴張或逐部位 Data。
- 最終 camera 採人類提出的第一候選：position **(−0.75,1.55,−5)**、target **(−0.75,1.0713,16.8)**、FOV **36°**。只修改 TOML 兩個 Y；parser／validation／fallback、projection 實作皆未改。沒有另測相鄰 camera，因第一候選已符合本輪數值方向，球路仍可辨；後續靜態構圖 review 已接受目前 camera，接受範圍見設計文件。

下表為**實際生成的 geometry vertices 經既有 project_batting_point 投影**，不是離線重寫角色 landmarks。帽冠 top 用角色帽色上部 vertices，沒有以 bat tip 代替；鞋／plate／bat 同樣從現有幾何選取。暫存量測跳過 camera 後方草地 vertices（原函式會拒絕投影），沒有修改 production projection。CSV 保留在 build。

| 指標（px，除另註） | Before | After |
|---|---:|---:|
| 投手帽冠 top Y | 481.8046 | 402.2809 |
| 打者帽冠 top Y | 415.2633 | 412.9772 |
| 投手鞋底最下緣 Y | 673.0867 | 594.4993 |
| 投手帽冠至鞋底 projected height | 191.2821 | 192.2184（+0.49%） |
| Gameplay focus X／Y | 959.9999／750.8538 | 959.9999／740.0032 |
| Overlay left／right | 826.9975／1093.9155 | 828.0790／1092.4269 |
| Overlay top／bottom | 604.8008／897.9097 | 595.2288／884.2245 |
| Overlay width／height | 266.9180／293.1089 | 264.3478／288.9957（−0.96%／−1.40%） |
| 中外野牆頂 Y／Y÷1080 | 572.2598／0.52987 | 475.3008／0.44009 |
| 打者鞋底最下緣 Y | 1064.8175 | 1067.6201 |
| 本壘 Y 範圍 | 959.1437～1020.9424 | 937.1412～1011.5363 |
| Bat Y 範圍 | 306.0336～677.8037 | 314.6493～680.3135 |

- 實際 1920×1080 screenshot 藍框外緣由 **X=826～1094、Y=604～898** 改為 **X=827～1092、Y=594～884**；解析 bounds 與筆畫外緣的差異來自線寬／rasterization。左右 world-X 白線仍水平：後線外緣生成 vertices 的 ΔY=0，before Y=1078.7207、after Y=1081.3107。**後線最外側約 1.3 px 超過 client 下緣**，其餘筆畫仍可見；沒有新增鞋底、bat 或重要本壘裁切，保留此 chalk 邊緣量測，不因本次靜態接受而宣稱它已修正。
- 原 camera 的 torso-only 圖顯示兩件衣襬較平、褲子下半仍可見，沒有肉眼可見 z-fighting；褲子到鞋的空間保留。生成的非 shirt 色 vertices／colors multiset 與 baseline 相同；角色 origin／scale、頭帽、手腳、bat、場地與 simulation Data 均未改。投手 projected height 僅 +0.49%，存在感主要來自位置而非巨大化。
- 實際檢視 Ready／release tick 0、ticks **1、3、6、12、24、36、48、60、72、84、94** 與 Complete。Release 球在牆頂上方，約 tick 24 短暫跨越黃色牆頂與手／臉背景，tick 36 後離開色帶並往胸前飛來；球本體仍可辨，沒有觀察到長時間沿牆頂相切。Mid／late flight、近壘均清楚，打者與修過的 torso 未擋主要球路。膚色背景對比與短暫邊緣交會是否足夠易追蹤，仍需 Michael＋Julia review，不能以球心座標在畫面內代替判讀驗收。
- Debug／Release build 成功，CTest 各 **2/2**；未放寬既有 centre／prediction tolerance。44 個非法 Data 案例、20 deterministic rethrows、30／60／120 FPS chunking、pause／single-step、arrival once、prediction／evaluation 一致性皆通過。沒有為局部造型新增測試 framework。
- 兩種實際 app 各初投＋**20 次重投**；首球在 tick 0 pause，逐步到 tick 94 後 resume，等待 pause 的兩張圖逐像素相同；正常 exit 0。Debug／Release 分別完成 **698／703 frames**。每次 release／arrival 與前版逐行相同，evaluation 的 world/time/velocity 也相同：raw tick **95**，p **(0.004963,0.759459,0.306804)**；plane p **(−0.000000450,0.775000632,0.431800008)**。本次 prediction／actual plane pixel **(959.999817,740.002991)**，error **0 px**；raw Complete 球心 **(966.906860,750.393799)**，未吸附。
- Debug GPU-based validation **0 errors**，未見 corruption；shutdown 無 live child resource，僅供報告使用的 device。原生 computer-use helper 啟動失敗，沿用僅針對 Pawapuro process 的 Windows key messages／SDL event loop／DPI-aware PrintWindow。這是實際 app 自動操作與 capture，**當時尚未執行 human review，也未驗證任何動畫**；後續 Michael＋Julia 的接受限於靜態造型／構圖，不擴張上述技術驗證範圍。

證據位於忽略的 `build/`（不提交自動產物）：

- `pre-rig-baseline-ready.png`、`pre-rig-torso-ready.png`、`pre-rig-final-debug-ready.png`：同 client 尺寸、相同 debug 元素的 full-frame 分階段比較。
- `pre-rig-pitcher-hem-comparison.png`／`pre-rig-batter-hem-comparison.png`：左 before、右 torso-only，兩側採完全相同 crop 座標與倍率；投手 4×、打者 2× nearest-neighbour，僅供衣襬 review。
- `pre-rig-final-debug-tick-48.png`／`pre-rig-final-debug-complete.png`：正式 mid-flight／Complete；`pre-rig-final-debug-tick-*.png` 與 `pre-rig-flight-inspection.png` 為額外診斷。Release 有同名三狀態與 tick captures。
- `pre-rig-*.log`、`pre-rig-baseline-geometry.csv`／`pre-rig-final-geometry.csv`：runtime／GPU 紀錄及生成幾何量測。暫存操作／量測 source 與 executable 已移除。

Michael＋Julia 已完成靜態 review，本次 pass 收尾；[設計文件](../design/batting-feel.md) 分開記錄接受項目與暫緩問題。此處既有 build／capture／量測結果不變，本次僅更新文件狀態，未重跑 build／tests 或新增驗證。下一步在新對話規劃 Rig／Animation Pipeline；early-flight 背景對比與動畫遮擋仍待後續人類檢查，不視為動態球路、動畫、正式資產或 M1 驗收。


## Right-handed Pitcher S0：Authoring Sample／Export Contract（2026-09-15）

開始時實際 checkout 為 `C:/astra-dev/pawapuro`、branch `main`、HEAD `b296f9dec43d92423df8ae6ac37b7e6a7d501872`，工作樹乾淨。Michael 在 Julia review 後正式授權 S0 的工具準備、最低角色／動畫資產與 export contract；沒有授權 C++ runtime 接入。

### 實際工具準備

| 工具 | 本次結果 |
|---|---|
| Blender | 官方 Windows x64 ZIP，**4.5.13 LTS**；build `daeeeca98fb0`，build date 2026-08-25、commit date 2026-08-24。`--version` 與 headless 啟動／製作／render 均成功。 |
| Blender 路徑 | `C:/astra-dev/tools/blender-4.5.13-windows-x64/blender.exe`。ZIP／checksum 留在同層 `downloads/`；未改全域 PATH、檔案關聯或既有使用者設定。 |
| 官方 SHA-256 | ZIP 實際雜湊與官方 `blender-4.5.13.sha256` 一致：`b5fdf800ce65fa2f209e8f68d02667e4d720fa1c42f247c72d1882ab04decba6`。驗證後才解壓／執行。 |
| Khronos Validator | 官方 Windows binary **2.0.0-dev.3.10**，位於 `C:/astra-dev/tools/gltf-validator-2.0.0-dev.3.10/gltf_validator.exe`，含 LICENSE／NOTICES。僅用於 authoring validation，未加入 runtime dependency。 |
| Validator SHA-256 | 本機下載 ZIP 雜湊 `c5068f51205deedc28acc3529ee7e11ee60e853454f673093398eba80142202c`；官方 release API 未提供 digest，**這是本機記錄，不是獨立官方 checksum 比對**。 |
| 圖片／影片 | Contact sheets 使用本機既有 Python／Pillow 12.3.0；影片使用 Blender 隨附的 H.264／MP4 encoder，沒有另裝 FFmpeg 或第三方外掛。 |

官方來源：[Blender 4.5 LTS](https://www.blender.org/releases/4-5/)、[Windows ZIP](https://download.blender.org/release/Blender4.5/blender-4.5.13-windows-x64.zip)、[官方 checksum](https://download.blender.org/release/Blender4.5/blender-4.5.13.sha256)、[Khronos release](https://github.com/KhronosGroup/glTF-Validator/releases/tag/2.0.0-dev.3.10)。格式依據：[glTF 2.0 規格](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html)。實際 exporter 參數另用此版本的 Blender RNA 查證，不把其他版本的 UI 選項當成已確認結果。

### 資產與量測結果

本輪只有 `pawapuro/batting/pitcher/` 的 authoring 資產／腳本，以及相關文件變更。細部空間／時間／格式契約與實際開啟、匯出、驗證、preview 命令由 [pitcher README](../../pawapuro/batting/pitcher/README.md) 維護。

| 檢查 | 實際結果 |
|---|---|
| 小型資產 | `.blend` **2,356,699 bytes**；GLB **208,312 bytes**；2362 vertices／3936 triangles、13 skin joints、15 nodes、單一 mesh／skin／clip |
| Clip／marker | `pitch_R`，39 TRS channels、全部 LINEAR；60 fps、fps_base=1、source frames 1–151 → GLB 0–2.5 s；唯一 release marker frame 91 → 1.5 s → 240 Hz tick 360 |
| 尺度 | Rest mesh 高約 **2.56515 m**；2.45 是既有比例 scale，未重複作 runtime scaling。Placement 保留 staging 的 `(0, 0.355, 18.5166)`。 |
| Official validator | **0 errors／0 warnings／0 infos／0 hints**，資源內容驗證開啟 |
| Source grip world | `[-0.6499997973442078, 2.0500002908706665, 16.799999105072022]` m；使用 source pose 與既定座標／placement 換算，並非 Native sampling |
| GLB grip world | `[-0.6499996781349182, 2.0500002908706665, 16.79999970111847]` m |
| GLB − authored reference | `[3.2186508180931384e-07, 2.9087066666377837e-07, -2.988815310800419e-07]` m；距離 **5.26814052e-07 m**，約 **0.000527 mm** |
| GLB 自行取樣 vs source | 十個重要 frames（含 90／91／92）雙向 vertex-position 最近點最大差 **9.13721919e-07 m**；沒有把兩種不同幾何順序直接以 index 相減 |
| 空白 Blender scene round-trip | 重新匯入 GLB；十個姿勢 mesh 最大差 **1.19952028e-06 m**、grip 最大差 **9.25312918e-07 m**。不是 C++ importer 驗證。 |
| Weights／inverse binds | Weight sum 最大誤差 **2.98023224e-08**；每 vertex 最多四個槽位／兩個非零 influences。Rest joint × inverse bind 與 identity 最大元素誤差 **3.08863861e-07**。 |
| 平塗顏色 | 五色確實在 `COLOR_0`；匯出最大 channel 誤差 **7.62951095e-06**。Blender reimport 轉為 byte color 後最大差 **0.0035380435**，明確保留量化限制。 |
| Planted feet | 後腳 frames 1–73、前腳 frames 73–151 的 world transforms 最大差皆 **0**。這不等於真實受力／重心已驗收。 |
| Source 保護 | 生成腳本對已存在 `.blend` 拒絕覆寫；實測保留原檔 SHA-256。日常匯出不保存 source，同 source 重複匯出的 GLB byte-identical。 |

TOML 包含 source／GLB hashes；詳細 raw figures 在 `build/pitcher-s0/sample-validation.json`。Release comparison 使用 staging 原始十進位數值作參考，不先轉成 float32 使差值被隱藏。本輪沒有改 `staging.toml` 的 release、velocity、camera、mound、角色 scale 或好球帶。未來 Native 的取樣／矩陣實作仍須另行量測，不宣稱目前已可無差異交接 simulation。

### 實際視覺證據與限制

- 原始 source 的 151 frames 逐張離線渲染；正常斜側面／batting 影片解碼確認 **60 fps、151 frames、2.516667 s**。Source clip 是 2.5 s，影片包含首尾兩個端點，因此多一個顯示 frame。慢速固定每 frame 重複四次，**60 fps、604 frames、10.066667 s、0.25×**。沒有拿掉幀的 viewport capture 代表正常節奏。
- 已檢視七姿勢、release 前後、source／GLB 比較與影片解碼 frame。局部修正肩部連接高度與抬腳時的前臂路徑，避免明顯深入大頭；保留連續手臂與衣襬覆褲子。手套／帽／臉仍是粗略形體。
- 另對七姿勢作頭部橢球與右臂表面的局部粗查；排除靠身體的前兩個 attachment rings 後，沒有 normalized squared distance <0.9 的右臂點。這只是針對觀察到問題的輔助診斷，**不是整段 self-collision 或全角色無穿插保證**。
- 部分加速姿勢在特定視角仍有手臂／大頭／帽簷的投影重疊；腳的平塗對比、蓄力／跨步重量感與收勢節奏，交 Michael＋Julia 判斷。未把數值通過當成人類動作接受。
- Authoring batting camera 使用現有 position／target／36° vertical FOV，推導 Blender horizontal shift 約 **0.21186446**，gameplay focus NDC X 約 **0.5**；release 投影約 **(738.816, 465.350) px**（1920×1080）。只有 camera 對照，沒有重建球場、打者或 app camera framework，不能驗收真實背景遮擋／early-flight contrast。
- 輔助球在 frame 92 隱藏；preview 未模擬 release 後飛行。Source、GLB comparison 與 MP4 都有 authoring-only 標示。

Evidence：`build/pitcher-s0/pitcher-normal.mp4`、`pitcher-slow.mp4`、`pitcher-batting.mp4`；`seven-key-poses.png`、`release-grip-reference.png`、`batting-camera-release.png`、`roundtrip-comparison.png`；validator／sample-validation／export／render／encode logs 與逐 frame PNGs 皆在忽略的 `build/pitcher-s0/`，不 commit。工具及 archives 留 repo 外。

**沒有重跑 C++ build／CTest／app／GPU validation**：本輪未改 production C++、HLSL、CMake 或 staging Data；先前通過結果不算本輪新驗證。S0 candidate 停在 Michael＋Julia review gate，未開始 S1／S2／S3，沒有宣告動態 gameplay 或 M1 通過。


## S0.1 Motion reblocking（2026-09-15）

開始時 cwd `C:/astra-dev/pawapuro`，branch `main`，HEAD／main／origin/main 皆 `fb97f4614792c5e90ac9d3996083b0dbd0302bf8`，工作樹乾淨；`git ls-remote origin refs/heads/main` 也確認遠端同 commit。沿用 Blender 4.5.13 LTS 與 Khronos validator 2.0.0-dev.3.10，未安裝新 dependencies。原 `build/pitcher-s0/` 預覽未覆寫；source／GLB／TOML 另存 `build/pitcher-s01/before/`。本輪研究與 motion brief 見 [design](../design/batting-feel.md#s01-motion-reblocking)。

### 實際 source 根因核對

讀取保存的 S0 `.blend`，逐格 evaluated 151 frames，並檢視其 source-derived 圖像，而非只檢查 generator：

- Pelvis／chest／head world yaw 的確是同曲線振幅縮放；rest／parent space 轉換未帶來不同發動時間。原負 yaw coil 使右肩較靠本壘，不能用加大原 yaw 解決本輪要求的右肩後留。
- 全段右上臂關節距離 **0.0341～0.9296 m**、前臂 **0.2370～0.6913 m**，證實原先獨立移動 elbow／hand 的 FK bake 仍會伸縮。Weights 及 rest hierarchy 本身不需重建。
- 分段 smoothstep 的確讓多個單調姿勢前後反覆慢下來；S0 整段 grip 最大速度反而在 frame 49，release 當格約 5.93 m/s。這是離散影格差分的 authoring 數據，不是 Native 球速。
- 原版已有 post-release keys、軀幹前折與右腳前移。問題是右腳 frame 151 才回地面，沒有之後重新支撐的時間，不能描述成完全沒做 follow-through。

### 最終候選的實測

| 檢查 | S0.1 結果 |
|---|---|
| 不變項 fingerprint | Source rest mesh／topology／weights／colors、骨架 rest／parent hierarchy、兩台 cameras、placement／比例 scale 的前後 hashes 一致；無骨架或 weights 修改。 |
| 時間 | 60 fps、source 1–205、clip 3.4 s；唯一 marker 97 → 1.6 s → 未來 tick 384。 |
| Opening／前甩順序 | Evaluated world yaw 開轉速度峰值：pelvis frame 83、chest 91；grip 速度峰值 95 約 23.024 m/s；release 97 約 12.709 m/s。前腳接觸前 grip 峰值約 9.850 m/s。這些是候選曲線結果，不是影片測得的人體時間。 |
| 固定骨段 | 全 205 frames 右上臂約 0.387803 m、前臂約 0.474552 m，誤差 <1e-5 m；grip local binding 全段固定 <1e-6 m。 |
| 腳底 | 四個 source 支撐區間皆落地且 world transform 差 <1e-6；其餘對應離地 frames 的 mesh 最低點 >1e-6 m。整段無低於地面的腳底，右腳 171 後固定、身體繼續回穩到 205。 |
| 方向／局部穿入 | Marker 前後 grip 持續朝本壘（local −Z）移動；全段右臂表面對頭部橢球 proxy 未檢出侵入。最初粗稿 coil 有侵入，降低折臂後排除。Proxy 排除肩部前兩圈，未涵蓋帽子／完整角色碰撞，不能宣稱完全無穿插。 |
| Khronos validator | 0 errors／warnings／infos／hints，含資源驗證。 |
| Release alignment | GLB 對原十進位 staging reference 誤差 **1.25296422e-6 m**（約 **0.001253 mm**）；原 0.1 mm 容差未變。 |
| 全段 source ↔ GLB | 205 個 frames，雙向 mesh 最近點最大差 **1.59491162e-6 m**。 |
| 空白 Blender round-trip | 全 205 frames，mesh 最大差 **1.56795340e-6 m**、grip 最大差 **1.07765894e-6 m**。Weights、bind、COLOR_0 與原 subset 檢查保留，未放寬容差。 |

實際腳本 `inspect_motion.py` 檢查 source evaluated mesh／pose，`verify_sample.py` 保留獨立 GLB TRS／skinning 計算及空白 scene reimport；source samples 擴為整段，accessor 解碼只 cache 唯讀資料。Export 不呼叫 reblock／generator，也不保存 `.blend`；metadata 由 source marker／contact properties 產生。

### 影像與尚未完成的觀看

`build/pitcher-s01/` 保存 before／after 正常速度斜側面與 batting-view、after 0.25×，以及 key poses、整段 overview、release 周圍與收勢後半連續影格、GLB round-trip 和單張診斷圖。全部使用既有 camera；乾淨影片隱藏固定 reference 環。圖表由 saved source 的 poses／marker／contacts 決定標籤。

正常影片 decoder 實測 60 fps：before 151 frames／2.516667 s；after 205 frames／3.416667 s。慢速 820 frames／13.666667 s，固定四次重複、0.25×。不同 clip 長度沒有被拉伸成相同時間；JSON sidecar 記錄 FPS、frames、速度與 authoring-only 身分。

**不能宣稱正常速度自看完成**：瀏覽器／播放器工具在啟動時退出。初次讀取亦曾遇到 automatic approval review 的模型容量不足，之後本機操作重試成功；無繞過審核。已實際檢視 source-derived 全段 overview、release／recovery 連續影格及 batting／diagnostic 圖，但不是影片播放。外部 reference video 也沒有實際觀看；來源候選與 FPS／視角未確認項列在 design。附件只有文字，沒有可檢視的遊戲參考圖。

從影格可見的限制：出手前斜側面仍有手／大頭／帽簷的投影重疊；大鞋與地面的深色對比使接觸不如診斷圖清楚。Release 後手臂速度仍有次級起伏，粗 motion 的重量感與節奏不能由速度峰值代替人類判斷。保留 camera、比例、顏色及 lighting，不用調構圖掩蓋。

本輪未執行 C++ build／CTest／app／GPU validation，未修改 production code／staging；先前結果不列成本輪驗證。交付停在 Michael＋Julia review gate，不宣告 S0 motion／dynamic gameplay 通過、不進入 S1。

## S0.2A Closed Ready／Coordinated Leg Lift（2026-09-15）

開始時實際 cwd `C:/astra-dev/pawapuro`，main／HEAD／origin/main 與 live remote main 均為 `ed3d8be8840a45491b821964677d2295952905cd`，工作樹乾淨。Blender 4.5.13 LTS／Khronos validator 2.0.0-dev.3.10 沿用，沒有安裝 dependencies。先複製 source／GLB／TOML 到忽略的 `build/pitcher-s02a/before/`，未重跑 S0 generator 或 S0.1 reblock。

### Frame 79 右臂診斷

先開實際 S0.1 `.blend`，檢視 frames 77／79／81 的乾淨 mesh、evaluated wireframe、肩→肘→手骨架投影；包含既有斜側面與另一臨時 authoring angle。`diagnose_arm.py` 留存可重製圖像與量測，未保存臨時 camera／wire objects。

- 凹折在兩個角度仍存在，排除「只是遮擋」；骨架連接連續，upper-arm 相鄰旋轉在 frames 78／79／80 約 5.45°／3.85°／2.94°，無 frame 79 突然 roll 跳變，scale／determinant 近 1。
- Skinning deformation matrix 是 pose × inverse(rest)。Frame 79 upper／fore 相對旋轉約 136.22°；fore／hand 約 177.72°。各取 0.5 的 linear blend，其最小 singular value 約 0.37284／0.01992；前臂／手部混合區幾乎壓成平面。實際管面 ring 9 最小半徑約 0.00645 m，而 rest 約 0.1072 m。
- 既有全段 tube 漸變 weights 與此姿勢的相反 skin rotations 共同造成局部壓扁；不是單靠固定骨長檢查可發現。未證實是單一 bone roll 設定錯誤；最小 B 修法尚未驗證，不先改 weights 或重建骨架。
- 原 frame 49 fore／hand 相對旋轉約 47.08°、上述 singular value 約 0.91677，顯示嚴重度隨姿勢改變。沒有證據表明此 B 問題阻止 A 的三個動作關係；A 只協調自身新姿勢的 pose roll，B 完整保留。

### 局部候選與回歸

實際修改腳本 `pawapuro/batting/pitcher/revise_ready_lift.py` 讀取 S0.1、局部寫入 1–71 的 FK keys，另存 candidate；確認後以相同 bytes 更新正式 source。更新前再核對正式 source SHA256，防止覆蓋其他編輯。Final `.blend` SHA256 `18894d26d6b2bc29aa2ff2e2a58c3479cf17eb9e1361890f8c8b2e3c2dedadcb`，預設 frame 1。Export 未修改此 source。

| 檢查 | 實測 |
|---|---|
| Protected 動作 | `check_ready_lift.py` 開 before／after saved source，在 frames 72–205 每格比較所有 13 bones 的 world matrices 與所有 2362 evaluated vertices；matrix max abs = **0**、mesh max distance = **0 m**，容差各 1e-6。Protected keys 逐值相同。 |
| 不變項 | Rest mesh／faces／weights／colors、骨骼 rest／parents／lengths、object transforms、兩個 cameras、placement／scale、contacts／timeline、球 mesh／scale 逐值相同。 |
| Ready 方向 | Evaluated chest front game 約 `(-0.99999994, 0, 0.000000134)`，即 −X。頭的 world orientation 保留。 |
| 合手藏球 | Frames 1–49 每格取 288 個球面點，轉回實際手套 deformation space；橢球 normalized squared distance 最大 **0.416656**（表面為 1）。這是幾何輔助，另檢視兩個視角的實際 render，沒有外露白點。 |
| 接回連續性 | 檢視 64–76 連續影格。Grip 在到達 frames 70／71／72／73 的差分速度約 1.305／1.956／2.331／2.735 m/s；左腳約 5.254／5.183／5.454／5.612 m/s，未在 72 歸零停住。不是正常速度觀看證據。 |
| Existing source checks | 全 205 frames 固定骨長、grip binding、腳底接觸／離地、頭部 proxy、release 方向與 opening 次序 PASS；revision S0.2A 仍執行原 assert，容差未放寬。 |
| 局部時序影響 | 全段 pelvis 最大開轉速度現在落在 frame 67 的接回區段，chest 仍 91、grip 峰值仍 95；並非宣稱 S0.1 全部前半段速度保留。50–71 展開偏快仍待 review。 |
| Khronos validator | 0 errors／warnings／infos／hints。 |
| Source ↔ GLB／round-trip | 全 205 frames，GLB mesh 最大差 **1.59491162e-6 m**；空白 Blender reimport mesh **1.56795340e-6 m**、grip **1.07765894e-6 m**。原 0.1 mm 容差未變。 |
| Release／支撐 | 原 world release 誤差 **1.25296422e-6 m**；GLB 接觸區間 transform 最大 drift **2.93140912e-9**。唯一 release 97、60 fps、frames 1–205、四段接觸區間不變。 |

### 證據與限制

六支 MP4 均以 Blender 隨附 decoder 核對 60 fps／1×，首 source frame 是 1。兩視角 before／after 局部比較各 84 frames／1.4 s；兩視角完整 after 各 205 frames／3.416667 s。Before 是本次 ed3d8be S0.1，hash `562f1237a3066c89eb0ccb88423e0b227c7c02861b1b014ed9f222be30a11be0`；不是 S0。Manifest／影片 JSON sidecar 保存 source hash、首尾 frame、FPS／倍率。

已檢視兩視角 contact sheets、Ready／coil 放大、分手／接點連續影格及右臂診斷。正常速度播放自看未完成，不宣稱看過影片；Michael 參考影片／圖片未在可讀附件找到。本輪沒有新 reference video 觀看／逐格觀察，也不使用 0.25× 秒數推估 timing。

從影格可見，展開區段較緊、深色雙鞋部分重疊，簡化手臂的凹形輪廓仍粗糙；B 的 frame 79 壓扁和 C 的後腳路徑保留。數值與影格證據不能替代重量感／正常速度可讀性的人類判斷。未執行 C++ build／CTest／app／GPU 測試，沒有 production C++／HLSL／renderer／staging Data 變更。交付停在 Michael＋Julia review，不進入 B、C 或 S1。

## S0.2B Arm Deformation／Glove Direction（2026-09-15）

起始 cwd `C:/astra-dev/pawapuro`，main／HEAD／origin/main／live remote 均為 `45305e36b2efbe4901b4b1be7480bb01285db9c3`，工作樹乾淨。保存三檔 S0.2A baseline；其 source hash `18894d26d6b2bc29aa2ff2e2a58c3479cf17eb9e1361890f8c8b2e3c2dedadcb` 與既有 after-side／after-batting manifests 一致，可重用 before renders。沒有安裝工具、重做廣泛研究或重跑過往 generator。

先讀既有 diagnostic JSON／圖，只補 f77／79／81、Ready／coil、release／收勢樣本。兩策略隔離比較：roll-only 把 f79 ring 9 最小半徑從 0.006453 m 改善至 0.042044 m，仍收窄；相同 roll 加局部 weights 達 **0.107187 m**，接近原 rest 半徑。兩個既有診斷角度都恢復連續管狀輪廓，沒有修改 mesh 半徑／topology。混合集中於肘附近四圈，管端跟 forearm 收入原球形 hand，避免原 forearm／hand 相反旋轉在整段前臂互相抵消。僅右臂 tube 144 vertices 的 weights 改變。

`revise_arm_glove.py` 另存兩個 arm candidates；選定後才由 `point_glove.py` 讀選定來源、修改左臂 50–96。手套 f79 的肩→中心水平偏角由 −57.171° 改為 **+0.501°**，水平 home cosine **0.999962**；保留彎曲，並非只轉手套表面。Frames 1–49 與 97–205 左臂完整保留。接回到達 95／96／97／98 的 glove 差分速度約 3.66／2.78／1.97／1.74 m/s，沒有在 97 新增 hold；分手展開偏快仍交人類判斷。

| 整段 205 frames 檢查 | 實測／容差 |
|---|---|
| Body／head／feet evaluated matrices | 最大差 **0**，保護容差 1e-6。 |
| 左臂授權區間外／右臂 roll 區間外 | 左臂 1–49、97–205 及右臂 1–49、110–205 matrices 最大差皆 **0**。 |
| 右臂關節／右手與 grip | 關節位置最大差 **3.191e-6 m**；右手／grip matrix max abs **7.629e-6**；grip local matrix 差 **3.949e-7**。子節點 TRS 補償的 float32 誤差，以原骨長 guard 同級 1e-5 檢查，不宣稱逐 byte 相同。 |
| 非修改 mesh 區域 | 排除右 tube 全段與左臂 50–96 後，最大差 **4.068e-6 m**，包含被補償的右手球形 mesh，容差 1e-5；rest vertices／topology／配色／其他 weights 不變。 |
| 允許的右 tube deformation | 全段最大 vertex 位移 **0.327242 m**，與關節軌跡差分分開報告；所有 sampled rings 最小半徑／rest 半徑 ≥ **0.347050**，最低在 f174。不是強迫全 mesh 0 差值。 |
| Ready／coil | 左腳與身體軌跡未改；兩姿勢影格回歸已檢視。球面對手套橢球 q 最大 **0.416656**，原 binding／顯示規則保留，沒有新增消失開關。 |
| Motion checks | 固定骨長、grip、接觸、release 方向、opening 次序通過；S0.2B revision 不跳過 assert。左臂新姿勢沒有新增頭部橢球 proxy 侵入；不是完整碰撞保證。 |
| Khronos validator | 0 errors／warnings／infos／hints。 |
| Source → GLB → round-trip | GLB mesh 最大差 **1.283923e-6 m**；round-trip mesh **1.567953e-6 m**、grip **1.187614e-6 m**。原 0.1 mm 容差不變。 |
| Release | 原 staging world reference 誤差 **5.454740e-7 m**，原 0.1 mm 容差不變；97／60 fps／1–205／contacts 不變。 |

最終只渲染一版完整 side（205 frames）與 batting 45–105（61 frames）。MP4 decoder 核對 60 fps／1×；完整片 3.416667 s、短片 1.016667 s。Before 完整片沿用 `build/pitcher-s02a/after-full-side.mp4`，before batting 短片從正確原 renders 擷取，沒有重渲整段或另做慢速版本。局部圖、Ready／coil、分手／接回連續影格與整段 overview 已檢視；正常速度播放自看未完成，不反覆嘗試已知失敗播放器，也未宣稱參考影片觀看。

簡單命令／script 計時（不含工具審核等待與人工判讀）：兩個 arm edits 約 0.134／0.142 s，glove edit 約 0.067 s；三組局部 renders 約 5.11／4.53／5.19 s，glove probe 2.52 s；scope comparison 1.95 s，export 1.65 s，round-trip verification 2.91 s，motion checks 0.56 s；最終 side 20.65 s、batting 10.73 s、補充 diagnostic 4.64 s；encode＋decoder side 2.78 s、兩支 batting 合計 3.99 s。這不是效能 benchmark 或總工作耗時。

正式替換的自動審核拒絕覆寫未經 human review 的 source，建議隔離候選。其後發現正式三檔已與候選 bytes 相同，與拒絕回報不一致；逐檔確認候選 hash 與備份 HEAD blob 後，已恢復原三檔，沒有 reset／改歷史。最終交付 `review/s02b/pitcher.blend`／GLB／TOML，正式資產仍為 S0.2A。Candidate source hash `b3dcccd38ad7c216b18ccb69b4a664240051de0b988618f449737611748dd366`，export 沒有保存／重建 source。升為正式資產仍待 review 後確認。

限制：低面數彎肘仍帶稜角，部分收勢手臂被身體遮擋；全段 ring／proxy 數值不保證所有 self-intersections 都已排除。沒有改 C、camera／深色鞋／lighting；沒有 production C++／HLSL／CMake／staging 或 runtime／GPU 測試。停在 Michael＋Julia review，未宣告整支投球通過。


### S0.2B Promotion to Official Pitcher Asset（2026-09-15）

Michael＋Julia 已接受 A 的 Closed Ready、藏球與左腳／蓄力連動，以及 B 的右臂 deformation 修正與跨步手套朝本壘。正式三檔已由接受的 review/s02b 原樣複製升至 S0.2B。C 的 release 後軀幹續轉／前折、右腳跟進並落在比左腳更靠本壘的位置、完整 follow-through／recovery 仍待處理及 review。S1 未開始，整支 pitch motion 與 M1 尚未通過。

基準 HEAD／main／origin/main 與 live remote main 均為 `4dc9569de51a7b7b099f6259dbafd05310c62352`，工作樹乾淨。核對接受候選 Git blobs 後，將 review/s02b 三檔明確複製至正式路徑，保留歷史 artifact；未執行 motion edit。

先對忽略目錄 `build/pitcher-s02b-promotion/export-check/` 的精確來源副本執行既有 exporter，取得全 205 格 source samples；重匯出的 GLB／TOML 與接受候選逐 byte 相同。正式三檔未經 Blender 重新保存或匯出。Promotion 後執行既有 validator、`verify_sample.py --asset-dir`、`inspect_motion.py`、`check_ready_lift.py --scope arm-glove`，全部 exit 0／PASS，未改腳本、assert 或容差。

- Validator：0 errors／warnings／infos／hints。
- 全 205 格 source／GLB mesh 最大差 1.283923e-6 m；round-trip mesh 1.567953e-6 m；release alignment 5.454740e-7 m，均在原 0.1 mm 容差內。
- 固定骨長、grip binding、四段 contacts、motion checks 通過；60 fps／1–205／release 97 不變。
- A／B 回歸以 S0.2A before（18894d26…）比較：body／head／feet 全段矩陣差 0；左臂 50–96 外、右臂 50–109 外矩陣差 0。Ready／coil 藏球 q 最大 0.416656 < 原 0.8，預設 frame 1；全段右 tube 半徑比最小 0.347050 > 原 0.25；f79 手套朝本壘 cosine 0.999962 > 原 0.98。既有 B scope 保護 A 的身體／腳／合手，不套用 A 的全 mesh 不變規則。
- 驗證後正式三檔與接受候選逐 byte 相同，evaluated animation、weights、marker、metadata 均未改變。SHA256：
  - `pitcher.blend`：`b3dcccd38ad7c216b18ccb69b4a664240051de0b988618f449737611748dd366`
  - `pitcher.glb`：`1ce3f915a5f869d85f6e267b44b0534449c8f9f48e95e82bea2d2db7b8b9eb1f`
  - `pitcher.toml`：`f5206448621d7bcd5cb1e43c2327ff23bbc4815801eb2789e93cbaf4712aeaf0`

證據 JSON／logs 留在忽略的 `build/pitcher-s02b-promotion/`。本次未新增影片觀看、正常速度播放自看、runtime／GPU／C++ 測試，不把此前未完成項目補寫為完成。沒有 production C++／renderer／staging 變更。完成後停止，等待下一個授權。


### S0.2C Follow-through／Rotation／Rear-Foot Recovery（2026-09-15）

實際 cwd `C:/astra-dev/pawapuro`；起始 branch main、HEAD／main／origin/main／live remote 均為 `55489a9c46b947903da79e6435d586ecad471a02`，工作樹乾淨。Before 是正式三檔的 byte copy，保存在忽略的 `build/pitcher-s02c/before/`；沒有從 review/s02b 重新開始或執行舊 revision scripts。`revise_follow_through.py` 僅對 source S0.2B hash 作一次局部 98–205 編輯。先渲染一版候選的兩視角十格局部 pose，再選定相同 bytes 到 review/s02c，才完整匯出及渲染最終影片；沒有反覆重渲整包。

| 實際驗證 | 結果 |
|---|---|
| 1–97 全 13 bones／2362 vertices、protected keys | 矩陣／mesh 最大差 0，keys exact；原 1e-6 容差。 |
| 左腳全段、mesh／weights／rest／hierarchy／camera／timeline／contacts | 左腳矩陣差 0；常數 exact。正式三檔與 before bytes 相同。 |
| A／B 回歸 | Closed Ready front −X、default frame 1；clasp q 0.416656 < 0.8；f79 glove cosine 0.999962 > 0.98。 |
| B 右臂 deformation | 全段 ring 半徑最小比例 0.347049 > 原 0.25；每格每圈與 baseline 半徑最大差 4.767e-7 m < 1e-6，weights 未改。 |
| Grip／骨長／motion | 固定 binding 矩陣最大差 5.734e-7；既有骨長、head proxy、opening 順序與 release 方向 assertions 全通過。C revision 沒有跳過原 checks。 |
| 右腳 world/game landing | f171：R Z 16.476601 m、L Z 16.850599 m，差 −0.373999 m。R X −0.410 m、L X 0.3675 m。 |
| Contact support | R 86–170 離地底部最小 0.001550 m > 1e-6；171–205 底部誤差 0、world translation slide 0。原左腳 contact 不變。 |
| Chest 續轉＋前折 | yaw f97 −25° → f144 −108.965° → f205 −65°；前折 f97 28° → f127 56.322° → f205 8°。Pelvis／head 不同時序。 |
| Release 接點 | Grip 97→98→99 速度 12.709／7.809／8.740 m/s；baseline 12.709／7.737／8.551，既有 release 速度對比保留，沒有新增 hold。 |
| Khronos validator | 0 errors／warnings／infos／hints。 |
| Source／GLB／round-trip | GLB mesh 最大差 1.283923e-6 m；round-trip mesh 1.744900e-6 m、grip 1.660393e-6 m；release alignment 5.454740e-7 m。全部在原 0.1 mm 容差內。 |

`check_ready_lift.py --scope follow-through` 增加 sample-specific C 分支，原 A／B assertions 不刪改；舊 B scope 亦實際重跑 PASS。`inspect_motion.py` 新增 lean 觀測值並讓 C 執行原 assertions。沒有放寬 tolerance、新增 dependency 或 framework。Candidate export 讀取已存檔 source，未重新保存；來源與匯出 hashes 由 TOML／validator 核對。

Candidate SHA256：

- blend：`8b76ae4a40377fa07021ebdf18c98b3bd73dddf132154961fbcf7131c63c3bca`
- GLB：`c151f41798432c777464beea590b9ea5241e019cfb88bff6a99b9b2b1092d038`
- TOML：`9c982e1486642cc54d7071aa832bd4a0f720d75c113f56b7ecdc9f31cc55cf21`

Evidence 在 `build/pitcher-s02c/`，影片／renders／logs 不提交。四支 MP4 以既有 Blender decoder 核對 60 fps、repeat=1／playback=1×：完整 side／batting 為 205 顯示 frames、3.416667 s、首格 1；before／after 收勢各 109 frames、1.816667 s、97–205。沒有拉伸時間或新增 0.25×。已檢視兩視角局部／最終 contact sheets、release 97–108 與 landing 164–176 連續格、全段抽樣與等比例 world top diagnostic；正常速度播放自看未完成，不把 decoder 檢查寫成觀看。

視覺限制：斜側面中段約 115–160 的右臂部分被 torso 遮擋，深色雙鞋近接時分離度有限；batting-view 原畫面人物小，sheet 只作固定 crop 放大。數值／proxy 不保證完整 self-collision 或人類重量感。沒有調 camera、比例、lighting、場地或 Native pitch 初始條件；沒有 production C++／HLSL／renderer／staging 變更，沒有 runtime／GPU／C++ build 測試。C 尚待 Michael＋Julia review，正式 S0.2B 不覆寫，停止於 gate、不進入 S1。


### S0.2C Promotion to Official Pitcher Asset（2026-09-15）

Michael＋Julia 已完成 A／B／C human review；正式 pitcher 三檔原樣升至 S0.2C，Right-handed Pitcher S0 的單一 pitch clip authoring motion baseline 通過。這只代表 Blender／GLB authoring baseline；app runtime animation、release integration、dynamic occlusion 與 early-flight readability 尚未驗證，正式遊戲品質與 M1 尚未完成，S1 未開始。

Michael 的實際正常速度觀看 feedback：「右腳前踩、軀幹延伸都不錯，有投球的味道。」接受包括 release 後軀幹旋轉／前折、throwing arm 延續、右腳朝本壘跟進並落在左腳前方，以及 follow-through 到 recovery 的目前節奏與整體觀感。此為人類觀看結果，並非補稱 Codex 已完成正常速度自看。

起始 cwd `C:/astra-dev/pawapuro`，branch main；HEAD／main／origin/main／live remote 均為 `d3de8a99e1072f4ff2b940a6dd6dab07a8c3e7a3`，工作樹乾淨。候選三檔核對 Git blobs、交付 SHA256 與 TOML provenance 後，明確從 review/s02c 複製至正式路徑；未保存 .blend、未修改 keys／weights／marker／metadata。歷史 review artifacts 保留，未整理或刪除。

從正式 .blend 的 byte-identical 隔離副本，用既有 exporter 取得全 205 格 source samples；隔離重匯出 GLB／TOML 亦與接受候選逐 byte 相同，沒有重寫正式三檔。再以正式 asset 執行既有 validator、verify_sample、inspect_motion、check_ready_lift 的 follow-through scope；全部 PASS／exit 0，腳本、assert、tolerance 未改：

- Khronos validator：0 errors／warnings／infos／hints。
- Source／GLB mesh 最大差 1.283923e-6 m；round-trip mesh 1.744900e-6 m、grip 1.660393e-6 m；release alignment 5.454740e-7 m，均在原 0.1 mm 容差內。
- 固定骨長、grip binding、contacts、motion checks 通過。C scope 同時執行 A 的 Ready／coil 藏球、B 的 ring deformation／glove direction 及 C 的續轉／rear-foot checks；沒有誤用歷史 B 的全段 body 不變規則。
- 1–97 bones／mesh 與 C 原始 S0.2B before 差 0，左腳全段差 0；weights 等常數 exact。右腳最終比左腳更靠本壘 0.373999 m，171–205 接地無滑移。
- 驗證後正式三檔與接受 candidate 逐 byte 相同。正式 SHA256：
  - `pitcher.blend`：`8b76ae4a40377fa07021ebdf18c98b3bd73dddf132154961fbcf7131c63c3bca`
  - `pitcher.glb`：`c151f41798432c777464beea590b9ea5241e019cfb88bff6a99b9b2b1092d038`
  - `pitcher.toml`：`9c982e1486642cc54d7071aa832bd4a0f720d75c113f56b7ecdc9f31cc55cf21`

JSON／logs 在忽略的 `build/pitcher-s02c-promotion/`。未重製影片或 screenshots，未執行 runtime／GPU／C++ build 測試；沒有 production C++／HLSL／renderer／CMake／staging 或 dependency 變更。完成後停止，S1 尚未開始。


## S1 Static Pitcher GLB Runtime Import（2026-09-16）

最新 human review：Michael＋Julia 已接受 S1 static import；接受邊界與延後的 Character Style debt 見 [Character Motion Rules](../design/character-motion.md#s1-acceptance-與延後的-polish)。以下保留 S1 技術交付當時的紀錄；S2 本輪實測另列於末節，不將舊的球路 app 測試當成本輪重跑結果。

基準 cwd `C:/astra-dev/pawapuro`，branch main；HEAD／main／origin/main／live remote main 均為 `83408e4c8ba3188b2bb15a7fe185ec10cd4bff0c`，起始 workspace 乾淨，origin 為 canonical `michael-wang/pawapuro`。本輪完成 static GLB transport，待 Michael＋Julia review；S0 的 A／B／C authoring motion 已接受。未改正式三檔、motion、staging、simulation、D3D12View 或 HLSL。

### 依賴與建置

唯一新增 dependency 為 [cgltf v1.15](https://github.com/jkuhlmann/cgltf/releases/tag/v1.15)，固定 peeled commit `360db1a95480fe102ae9c69b27c5d101167ff5ba`；[官方 header／API](https://github.com/jkuhlmann/cgltf/tree/360db1a95480fe102ae9c69b27c5d101167ff5ba) 與 [MIT license](https://github.com/jkuhlmann/cgltf/blob/360db1a95480fe102ae9c69b27c5d101167ff5ba/LICENSE) 已核對。`cmake/PrepareCgltf.cmake` 明確下載至忽略的 `.deps/cgltf-1.15/`，以 SHA256 固定 header `e378a21c084bf1f288bb799de827bb26906efb024255f1ecf1705ea13f11c6ec`、license `f619925f80ef862497aaf8e8155ef218fa6a2190055129523ca3df9119a9ba95`。Prepare 已成功執行；正常 configure 只檢查本機檔案，不連網。Build 沿既有慣例複製 `cgltf-LICENSE.txt`；未加 package manager、其他 dependency 或 global PATH。

在既有 x64 VS developer shell（本機 MSVC 14.44／SDK 10.0.26100.0），repository root 執行：

```powershell
cmake -P cmake/PrepareCgltf.cmake
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
ctest --test-dir build/debug --output-on-failure
cmake -S . -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/release
ctest --test-dir build/release --output-on-failure
& ./build/debug/pawapuro.exe
& ./build/release/pawapuro.exe
```

本機 build script 在 child developer shell 使用 `chcp 65001`，避免既有本地化 MSVC include tracking 問題。建置輸出讀取 launch-relative `batting/pitcher/pitcher.glb`，與同層 `staging.toml` 同採 configure copy；正式 GLB 有變化會觸發重新 configure。Parser 只在 startup 使用；Engine／Pawapuro／GPU ownership 見設計文件，subset 見 pitcher README。

### 實測

| 檢查 | 結果 |
|---|---|
| Debug／Release build＋CTest | 各 3/3 通過：新增 static_pitcher、既有 reference_pitch／batting_staging。/W4／WX 保留。 |
| GLB transport | 2362 source vertices，3936 triangles，11808 expanded vertices；完整 scene immutable buffer 22563 vertices。 |
| Game local bounds，m | min (−1.263325, 0, −1.005480)，max (1.340500, 2.565150, 0.621810)。 |
| World bounds，m | min (−1.263325, 0.355000, 17.511120)，max (1.340500, 2.920150, 19.138411)。 |
| Bind grip world，m | (−1.120000, 1.670000, 18.166599)，位於 game −X；手套 hand_L 在 +X。不是 animated／release grip。 |
| Scale／grounding | scale=1；測試將 height_m 加倍，local bounds／頂點尺寸不變；只修改 staging placement 時所有頂點同量平移。Mesh 最低點等於 placement Y=0.355，為 mound top 0.35 上方 5 mm，沒有再乘 2.45。 |
| Static contract tests | 正式 subset／count、全部 position/color finite／opaque、合法 indices、hierarchy grip、basis／一次 winding、placement／bounds（1e-5 容差）、world／ball／overlay range isolation；缺檔、壞 magic、截短、非有限 POSITION、非法 index 的 source／owner／reason 全部通過。 |
| Existing regressions | 保留 20 deterministic rethrows、30／60／120 FPS chunking、pause／single-step、arrival once、prediction／projection、44 個 staging 非法案例。舊 procedural shoulder/release 重合拒絕測試改為允許獨立 release，因該投手 geometry 已移除；未刪除 simulation assertions。 |
| 實際 app | Debug／Release client 均 1920×1080，各初投＋20 次重投，pause 0→1 tick single-step；暫停等待兩張圖逐像素相同。完整球路仍由原 Native owner 更新。 |
| 與本輪保留 baseline app 比較 | 兩種舊 exe 各實際完成一球。新版每次 release／raw arrival／plane evaluation（含 pixel）逐行相同，camera／overlay log 也相同。Raw Complete tick 95，t=0.395833333 s，p=(0.004963,0.759459,0.306804)；plane t=0.392833450 s，p=(−0.000000,0.775001,0.431800)，prediction delta=(0,0) px。 |
| 畫面差分 | Debug／Release startup 逐像素相同；與各自舊版相比，僅 13056 pixels 改變，全部位於 [695,402]–[880,594) 的投手 rectangle（右／下 exclusive），rectangle 外逐像素一致。 |
| GPU／shutdown | RTX 5070 Ti。Debug layer＋GPU-based validation 0 errors／corruption；shutdown report 無 live child resources，只有供報告的 device。Debug／Release exit 0，完成 641／642 frames。Release 依既有 compile contract 未啟用 debug layer，不把它標成另一次 GPU validation。 |

正式 assets SHA256 與本輪 baseline 完全相同：

- `.blend`：`8b76ae4a40377fa07021ebdf18c98b3bd73dddf132154961fbcf7131c63c3bca`
- `.glb`：`c151f41798432c777464beea590b9ea5241e019cfb88bff6a99b9b2b1092d038`
- `.toml`：`9c982e1486642cc54d7071aa832bd4a0f720d75c113f56b7ecdc9f31cc55cf21`

兩種 build output 的 GLB 與正式檔 bytes／hash 相同；cgltf license copies 亦相同。本次未重新匯出、保存 Blender 或重跑 authoring motion validation，因其資料未改。

### Review 證據與限制

忽略目錄 `build/pitcher-s1/`：先開 `debug-startup.png`／`release-startup.png`（1920×1080）及 `debug-pitcher-crop.png`／`release-pitcher-crop.png`（固定原圖 [675,390]–[905,602] 區域 3× nearest 放大，camera 不變）。`*-midflight.png` 為 tick 48，`*-complete.png` 保留到壘；`debug.log`／`release.log`、`*-smoke.json`、`regression-summary.json` 與 `build-test.log` 記錄以上實測。`baseline-debug/`／`baseline-release/` 保留本輪前的 executable，baseline 截圖是舊 procedural app，不是動畫 before。

Computer Use 初始化及 reset 重試均失敗（trusted Node kernel exited／Windows sandbox helper setup refresh errors），沿既有 process-targeted Windows key messages → SDL event loop、DPI-aware PrintWindow 方式操作本次啟動的 app。這是實際程序／畫面檢視，非人類手動試玩；暫存 smoke script 留在忽略目錄，沒有新增 runtime automation framework。

已檢視 startup 全圖及投手放大：帽／臉朝本壘、球形右手在 game −X／手套在 +X、shirt／pants 配色與 detached feet 可見，未見 mesh 缺面；深色鞋並排時輪廓接近，原 reference release 指示圈／球位於臉旁。球沒有接手，此圖不證明 animated handoff、dynamic occlusion 或 early-flight readability。S1 保持 bind pose，即使 app simulation state 顯示 Ready 也不是 `.blend` frame 1。未重新製作 Blender comparison render，沒有 animation playback／skinning／release integration；停在 Michael＋Julia review，S2／S3 尚未開始、M1 未完成。


## S2 Runtime Pitcher Animation Playback／CPU Skinning（2026-09-16）

基準 cwd `C:/astra-dev/pawapuro`，main／HEAD／origin/main／live remote main 均為 `9eeb15147b18e54602f55745a512187b63f8a898`，origin `https://github.com/michael-wang/pawapuro.git`，起始 worktree 乾淨。S1 與 Character Motion Rules v0.1 已獲 Michael＋Julia 接受；本輪 S2 是 implemented candidate，待 human review，S3 未開始、M1 未完成。

### 建置與重現

沿用 S1 的 MSVC 14.44／Windows SDK 10.0.26100.0、SDL3／toml++／cgltf 1.15 與 RTX 5070 Ti。沒有新增 dependency 套件；GLB provenance hash 使用已安裝 Windows SDK 的 BCrypt API／`bcrypt.lib`。建置前曾沿既有 script 重跑同版 cgltf prepare，版本與 hash 未改。最終使用 `build/pitcher-s2/build-test.cmd`，不再 prepare／下載 dependency，依序 configure／build／CTest Debug 與 Release；命令與 S1 相同（略去 prepare）。`/W4 /WX` 保留。啟動與 controls／review ticks 見 [pitcher README](../../pawapuro/batting/pitcher/README.md#s2-runtime-animation-preview)。

所有暫時 logs／PNG／sheet／smoke scripts 留在忽略的 `build/pitcher-s2/`，沒有 runtime automation framework。`app_smoke.py` 對自己啟動的 app 送 Windows key messages，由 SDL 正常 event loop 處理：先 Space＋P 停在 tick 0，再逐一 single-step 816 ticks 擷取 review poses；測 final hold、Space 1× 重播；再次 replay 測 minimize／restore／P 繼續，最後 Esc 正常退出。

### 技術結果

| 檢查 | 本輪結果 |
|---|---|
| Debug／Release build＋CTest | 各 4/4 通過：pitcher_motion、static_pitcher、reference_pitch、batting_staging。 |
| Loader／asset subset | 正式 skin／clip／13 個 joint-parent 與每 joint 的 TRS channels 通過；合法 influences、finite inverse binds、ordered finite times；10 個拒絕案例包含既有 5 個，另加非法 joint、負 weight、錯 weight sum、NaN inverse bind、未遞增 timeline。 |
| Pose／timing | 八個 review ticks 重複 evaluate triangles byte-identical／world matrices 相同；30／60／120 FPS 的整數 nanosecond chunking 均到同 tick；pause、單步、backlog、end tick 816 clamp／hold、replay initial pose 通過。支援同一 Windows build 內 replay，不承諾跨 compiler／platform bit identity。 |
| Skin／basis | Bind skin 與 rest geometry 在 0.1 mm 內；改 mesh-node translation 不重複套到 skin；antipodal quaternion keys 不改變 shortest-path 結果。整段 817 ticks vertices finite、數量／colors 不變。四段 arm bone lengths 差 <1e-5 m；既有 planted contact matrices 差 <1e-6。 |
| Contact sample intervals | foot_R ticks 0–336、680–816；foot_L 0–64、312–816，依未改的 source contacts 對應。這是 sample-specific regression，沒有另建 authoring truth。 |
| Source → runtime samples | 八格 grip／mesh bounds／96 個 source 點最大距離 **1.35952e-6 m**；原 tolerance 0.0001 m 不變。Sample ID／抽法見 pitcher README，並非全 mesh／全 subframe 與 Blender 的完整誤差證明。 |
| 直接重讀 saved source | Blender 4.5.13 LTS（daeeeca98fb0）background 只讀 evaluated animation，未保存／匯出；八格 fixture 與 saved source 最大十進位截斷差 6.31123e-9 m（檢查 <1e-6）。`source-fixture-check.json`／`.log` 保留。 |
| Runtime release diagnostic | metadata authoring frame 97 → tick 384 → 1.6 s；grip world 約 (−0.65,2.05,16.8) m，與 staging release reference 距離 **4.9151248e-7 m**，小於原 0.0001 m。未 snap／改 Data，未觸發 ball event。其他 review ticks 的 full transform finite，grip positions 記於測試輸出與 app title。 |
| 實際 app 操作 | Debug／Release 各 1920×1080；start／replay、pause、817 次 single-step、minimize／restore、Complete tick 816／3.4 s、Esc exit 0 全部通過。Pause 及 Complete 等待前後逐像素相同；minimize／restore 停在 tick 39，背景時間未補入。 |
| 正常速度執行 | 完整 1× replay 沒有逐格擷取／step，Debug wall time 3.4341 s、Release 3.4051 s 到 Complete；含視窗輸入／present／poll latency，clip authoritative time 均 3.4 s。不把此 wall time 當拍攝 FPS 或正常速度視覺接受。 |
| Runtime 畫面差分 | 八張 Debug／Release 同 tick PNG 逐像素相同，記於 `render-comparison.json`。 |
| GPU／shutdown | Debug layer＋GPU-based validation 0 errors／corruption；釋放後只有報告用 device，無 live child resources。兩版各完成 1379 frames、exit 0；Release 按既有 compile contract 沒啟用 debug layer，不宣稱兩次 GPU validation。 |
| Simulation regression | 原 ReferencePitch source／prediction 與 staging Data 未改，既有 20 deterministic rethrows、30／60／120 chunking、pause／step、arrival／prediction／projection、44 staging 拒絕案例通過。S2 app 沒有重新發球，不把 unit regression 寫成舊 flight app smoke。 |

### CPU 成本觀察

既有 GPU 同步／present 與 app smoke 操作保持不變，使用 `steady_clock` 累計平均；同一輪各 2451 次 motion evaluations、1379 次 uploads，數字不是 performance target，也不是可移植 benchmark。

| 平均成本，µs | Debug | Release |
|---|---:|---:|
| Pose sample／hierarchy／skin matrices | 26.242 | 3.054 |
| CPU skin＋triangle expansion＋basis／grip | 523.374 | 40.952 |
| Dynamic upload 的 CPU memcpy | 9.676 | 9.743 |

2362 unique skinned vertices → 3936 triangles／11808 expanded vertices；每次 draw copy 283392 bytes 至 exact-capacity upload buffer。Upload 數字只計 CPU memcpy，不含 GPU 讀取／渲染或 fence wait；skin 數字包含 expansion／basis 的合併成本。沒有為小 sample 加入平行工作、GPU skinning 或 streaming allocator。

### 資產、證據與 human review 限制

正式 `.blend`／GLB／TOML SHA256 與 S1 末列三個 hash 完全一致；Debug／Release launch GLB／TOML bytes 均與正式檔相同。沒有 motion／weights／marker／camera／placement／scale／球半徑／physics／HLSL 變更，沒有重新 export 或跑 motion-edit scripts。只有 read-only Blender fixture 重核；沒有重跑完整 authoring validation，未把既有通過紀錄列為本輪重跑。

先開 `build/release/pawapuro.exe` 以 Space 正常速度觀看，再看 `release-motion-sheet.jpg`／`debug-motion-sheet.jpg` 與八張 `*-tick-*.png`。Sheet 使用原圖固定 rectangle (645,385)–(925,645) 2× nearest 放大，並標 runtime tick／clip time；沒有調 runtime camera。`build-test.log`、`debug.log`／`release.log`、`*-smoke.json` 保存操作與成本結果。

Computer Use 的 trusted Node 初始化與 reset 重試因 sandbox helper setup refresh errors 失敗；沿 S1 已用的 process-targeted key messages／DPI-aware PrintWindow 檢查本次 app。Codex 已檢視實際 startup 全圖與 Debug／Release review-tick sheets，**未以影片連續觀看正常速度 motion**；1× wall-time 執行與截圖觀察分開回報，由 Michael 在 app 補完整 motion review。

截圖觀察：closed Ready／coil 抬腳集中、stride／前腳接觸與收勢姿態有變化；部分 Ready／late poses 的手臂仍讀得出硬折角，feet footprint 相對大頭／torso 偏窄，鞋重疊時支撐分離度有限。Stride／contact 的投球臂部分被頭／帽遮擋，手套靠近臉；獨立 reference ball／release ring 仍在臉與出手區附近干擾觀察。這些保留為 Character Motion／Style debt，不在本輪修改資產或球 owner，不替 Michael＋Julia 判定動態可讀性通過。


## Style Polish v1A Larger Grounded Feet（2026-09-16）

起始 main／HEAD／origin/main／live remote 均為 `a1a2b01a5c6ee33c8e1d7ec4a431edfdb0532097`，workspace 乾淨。Michael＋Julia 已接受 S2 技術與 runtime human review，並確認 footprint 不足／hard-elbow debt 存在於 animated view。本輪只交一個 1.25× planar feet candidate，未 promotion，未改 production code／HLSL／CMake／staging／physics／正式三檔，未做 rubber arm 或 S3。

### 來源與保護

`revise_footprint.py` 直接讀正式 saved `.blend`，利用兩組各 220 vertices 的 rigid foot weights，以 bind/rest anchor 放大 Blender X／Y，保留 Z；另存 `review/style-feet-v1a/pitcher.blend` 後重新開啟並比較。沒有重跑角色 generator／motion-edit scripts。保存後全 205 格 bones／grip matrices、全部 keys／handles／interpolation、rest skeleton、topology／colors／weights、camera／其他 mesh、1922 個非腳部 source／evaluated vertices 完全相同。

Candidate GLB 的所有非 POSITION accessor bytes 完全相同，包含 animation、inverse binds、JOINTS／WEIGHTS、colors 與 indices；JSON 結構只允許 POSITION bounds 改變。所有 440 個 foot vertices 都符合固定 planar edit，其中 398 個座標 bytes 改變、42 個在中心軸不移動。比對先排除了錯誤的「440 個都必須改變」假設，並按 Blender → GLB 轉換保留 −0.0；沒有放寬 geometry 容差。TOML 只變 source／GLB hashes，clip／marker／contacts／space 語意不變。

`motion_expected.txt` 的 candidate 副本只更新 7 格受影響 bounds、每格 source IDs 1300／1500 共 16 個腳部點，以及 hash／candidate 標示；既有 grip／非腳部點與正式 fixture 保留原文。資料來自本次 saved candidate 的既有 export source samples，不是 runtime 計算反填 expected values。

### 本輪實測

| 檢查 | 結果 |
|---|---|
| Khronos validator | 0 errors／warnings／infos／hints。 |
| 既有 source／GLB／round-trip | PASS；GLB → source 全格 mesh 最大 1.283924e-6 m；Blender round-trip mesh 1.744900e-6 m、grip 1.660393e-6 m，原 0.1 mm 容差不變。 |
| 既有 inspect_motion | PASS，保留 S0.2C revision 與原固定骨長／grip／contact／motion assertions。 |
| Source foot shape／ground | Rest 寬 0.791350→0.989187 m、長 0.931000→1.163750 m，厚度 0.198450 m 不變。Planted world bottom Y 0.355 m 不變（比較 <1e-7 m）；全 205 frames 未穿過支撐平面。Airborne 右腳傾斜時最低點最大變化 0.0907805 m，詳見 pitcher README；不是承諾所有 airborne world-Y bounds 不變。 |
| Debug／Release build、CTest | 各 4/4 PASS，ninja 無 code 工作；targets／正式 fixtures 原樣保留。 |
| Candidate static regression | 兩版未修改的 static_pitcher_test 直接讀 candidate，全 subset／bounds／basis／grounding／10 個拒絕案例通過。 |
| Candidate S2 motion test | 兩版既有 executable 直接讀 candidate＋小型 candidate fixture，全 ticks／counts／colors／bones／contacts／determinism／chunking／pause／replay／Complete 通過；source samples 最大 1.35952e-6 m。 |
| Release diagnostic | Runtime tick 384，grip alignment **4.9151248e-7 m**，與 S2 原結果相同；容差仍 0.0001 m，沒有 snap／發球。 |
| 實際 app | 本輪重新執行正式 Release baseline、candidate Debug／Release。各完成 start／play／pause／817 次 step／replay／minimize／restore／Complete／Esc exit 0。兩種 candidate 的正常速度 wall-time 約 3.4317／3.4101 s，clip time 均 3.4 s。 |
| GPU／shutdown | Candidate Debug layer＋GPU-based validation 0 errors／corruption，無 live child resources（報告用 device 保留）；Debug／Release 完成 1378／2466 frames。Release 未啟用 debug layer，不列為另一次 GPU validation。 |
| Raster comparison | 兩個 candidate build 的八張同 tick 圖逐像素相同。六個 baseline/candidate review pairs 的變動只在鞋部 raster 區域；raw PNG 同為 1920×1080，camera／crop／倍率未因 candidate 改變。 |
| 1× 影片 | `candidate-runtime-1x.mp4`：actual runtime tick captures 0..816，每 4 ticks 一格，60 fps，解碼 205 frames／3.416667 s；多出的 1/60 s 是最後一格顯示時間，沒有 retime clip。這是 tick-sampled 重組，不是 wall-clock screen recording；另外執行不擷取的完整 1× app playback。 |

### Review 與 launch 狀態

證據留在忽略的 `build/style-feet-v1a/`。先看 `baseline-vs-125-runtime.jpg`、`candidate-runtime-1x.mp4`、`ready-contact-landing-feet.jpg`；`source-comparison.json`、`export-comparison.json`、`sample-validation.json`、`motion-audit.json`、`foot-world-bounds.json`、`*-smoke.json`、`build-test.log` 保存具體數值。一次性的 raster／encoder／export 比對 scripts 留在該目錄，不建立新 framework。

Computer Use 初始化／reset 重試仍因 Windows sandbox helper setup refresh errors 失敗；本輪沿既有 process-targeted key messages → SDL loop／PrintWindow 擷取，沒有操作其他 app。Codex 實際檢視六組 paired poses、full-scene startup 與跨完整 clip 的 14 格 sequence；未連續觀看正常速度影片，不代替 Michael＋Julia style review。

觀察：鞋部 footprint 更大，但 Ready／front-contact／final 深色雙鞋更容易連成較寬的一片，frame 65 展開姿勢的鞋面投影與衣襬重疊。未見六組 review 圖新增 screen-edge／打者遮擋；沒有宣稱完整 self-collision 驗證，沒有為 overlap 修改 pose／mesh 其他部位。是否更站得住、落地是否更有重量、1.25× 是否合適仍待 human review。

Runtime 使用 `candidate-debug/`／`candidate-release/` isolated launch copies：exe／DLL／staging 從本次 build 原樣複製並核對；只在 isolated copies 放入 candidate GLB／TOML。一般 `build/debug`／`build/release` 的正式 launch GLB／TOML 從未替換，結束後 bytes／hash 仍等於正式資產；無需恢復。正式三檔 SHA256 仍為上節 S2 baseline 值。

Candidate SHA256：

- `.blend`：`3fbd587e767c4a154a702bdeeca742d871b7ed16a1647e283c76746e00d1fb5a`
- `.glb`：`c5582de513ebb4163566a41225507bf39829bea58f003446a80871e82ca6c291`
- `.toml`：`f65220513d2f3727b225f380c693e7926e61a8f606e9ddc89023b1cb76f5a3d4`


## Style Feet v1A Promotion（2026-09-16）

起始 HEAD／main／origin/main／live remote 均為 `701a780888aa58f6d12c3684c1f2707b4bbd5e8e`，workspace 乾淨。先核對接受 candidate 與該 commit 的三檔 bytes、上節 SHA256、TOML source／GLB provenance，再直接複製至正式路徑。正式三檔 hashes 即上節 candidate hashes，promotion 與全部檢查後逐 byte 相同；未重新保存／匯出 `.blend`、未執行 revision script。`review/style-feet-v1a/` 保留。正式 S2 fixture 採已驗證 candidate 數值（只改 acceptance 註解），不改測試規則。

| 本輪回歸 | 結果 |
|---|---|
| Khronos GLB validator | 0 errors／warnings／infos／hints。 |
| Source → GLB → Blender round-trip | 唯讀重取正式 source 全 205 格，再跑既有 verify_sample：PASS，原 0.1 mm 容差不變。 |
| Motion／fixed bones／grip／contacts | 既有 inspect_motion PASS，revision／marker／接觸區間不改；正式與接受 candidate 的全 205 格 evaluated mesh／grip samples 完全相同。 |
| A／B／C 行為門檻 | Ready front X −0.99999994；clasp q max 0.416655605 <0.8；f79 glove home cosine 0.999961792 >0.98；right tube minimum ring ratio 0.347049298 >0.25；後腳落地 slide 0、落點較前腳靠本壘，chest yaw release −25° → minimum −108.964935°，保留原門檻。 |
| Promotion 幾何保護 | 與忽略目錄保存的升版前正式檔比較：1922 non-foot vertices、animation／inverse binds／skeleton／weights／colors／indices exact；440 foot vertices 符合原 1.25× planar edit，398 個實際座標改變，vertical rest coordinates 不變。接地／airborne 行為由既有 motion checks 與 candidate 全格一致性確認，沒有再次放大。 |
| Debug／Release build＋CTest | 各 4/4 PASS，production code 無 build 工作；正式 GLB／TOML 已由 configure 複製至兩種正常 launch 目錄並核對 bytes。 |
| S2 runtime diagnostic | Release tick 384 grip error 4.9151248e-7 m；八格 source sample 最大 1.35952e-6 m，與接受版相同；全部 tick、骨長、contacts、pause／step／replay tests 通過。 |

歷史 `check_ready_lift.py --scope ready-lift` 因其硬編碼的 pre-A baseline hash 拒絕 v1A 輸入，未執行該歷史 edit-comparison；B／C modes 也各鎖定當年的 pre-edit source，故不誤用。沒有刪 assert 或更換它的預期 hash。Promotion 另以忽略目錄的唯讀 `check_abc.py` 重用原 A／B／C absolute 門檻，並比較接受 candidate／正式來源全格一致性；這是 promotion audit，不新增 framework。歷史 checker 拒絕與替代檢查結果均保留在 `build/style-feet-v1a-promotion/`。

本次沒有重製影片／screenshots，沒有重跑 app 視覺／GPU smoke；不把上一輪 GPU 通過寫成本輪重跑。接受限於 1.25× support footprint 與原厚度；Foot Shape、material／highlight、articulation／sole／cleats、rubber-arm 仍待獨立 task，S3 未開始。舊正式備份／logs／JSON 留在忽略目錄，不提交。

## Style Polish v1B: Shoe-like Foot Shape（2026-09-16）

基準 HEAD／main／origin/main／live remote 均為 `b102e02e6d752c5f2c347659d46124a555c0b1ff`，開始時 clean。正式 v1A 三檔及正式 fixture 未改；`review/style-feet-v1b/` 是待 human review 的 geometry-only candidate，非 promotion。沿用 Blender 4.5.13 LTS／既有 Khronos validator／S2 runtime，未加依賴或改 C++／HLSL／CMake／staging。

| Candidate | SHA256 |
|---|---|
| pitcher.blend | `f54437abc9ddcf75474962c1b34d6c30e689144dd30adb806b7f03df1faa943b` |
| pitcher.glb | `d6361c53ad3caf005e19e50f61991508cff856f068d1639b3f29bcc50e06c8fa` |
| pitcher.toml | `25d17be36fcb6426030358a7450c9024f5bf4ecf9353812fe54d0ab535095f1b` |

- Source：保存前、重開保存後，各比較 205 格；所有 bones／grip matrices、keys／handles、rest／hierarchy、weights、colors、indices、camera、contacts 與非腳部 mesh exact。仍為 2362 vertices／3936 triangles，每腳 220 vertices。只改 438 個 foot positions；rest envelope 最大差 2.98e-8 m，planted bottom 差 0（原 1e-7 m guard）、landing slide 0。空中最小 bottom 0.000852719 m，原 noncontact >1e-6／不穿地 guard 不變。
- GLB：全部非 POSITION accessors byte-identical（含 animation times／TRS／IBMs／JOINTS／WEIGHTS／COLOR／indices）；1922 個非腳部 POSITION bytes exact，structure 除允許的 POSITION min/max 外相同。TOML 只改 source_sha256／glb_sha256。`check_shoe_shape_export.py` 未改任何 runtime/test tolerance。
- Khronos validator：0 errors／warnings／infos／hints。既有 source→GLB 及 Blender round-trip PASS，最大 mesh 誤差分別 1.283924e-6／1.744900e-6 m；round-trip grip 最大 1.660393e-6 m，原 0.1 mm 容差不變。GLB release reference 誤差 5.454740e-7 m，與 v1A 相同。
- `inspect_motion.py` 在 motion_revision=S0.2C 下完整 PASS：fixed lengths、grip binding、contacts、head proxy、opening 順序、release 前後向本壘延續均未跳過。舊 `check_ready_lift.py` 的歷史 pre-edit hash guards 不適用 v1A→v1B，未放寬；沿用原 ABC 絕對 checks 並配合全段 motion／非腳部 exact 比較：Ready front X −0.999999940、clasp q max 0.416655605（<0.8）、f79 glove home cosine 0.999961792（>0.98）、right arm ring ratio min 0.347049298（>0.25）、chest release −25°→min −108.964935°、rear landing slide 0。證據 `build/style-feet-v1b/abc-regression.json`。
- Debug／Release configure、build、CTest 各 4/4 PASS。兩種 build 另外直接對 candidate 跑原 `pitcher_motion_test` 與 `static_pitcher_test` PASS（後者含 10 個 failure cases）。原 S2 all-ticks／chunking／pause／replay／contacts／骨長 checks 保留；grip source 最大誤差 1.35952e-6 m、runtime release error 4.9151248e-7 m，非腳部 fixture 與 v1A 完全相同。只新增候選 fixture 的五格 bounds、16 個 foot points；正式 fixture 不變。
- 實際 app：既有 v1A harness 操作本輪 process 的 SDL 鍵盤輸入，PrintWindow 擷取實際 1920×1080 raster。Candidate Debug／Release 及正式 v1A Release 均 play／pause／817 single steps／replay／Complete／minimize→restore PASS、exit 0。正常速度 wall time 分別 3.4330／3.4364／3.4145 s；Complete tick 816，pause pixels 與 final hold 相同，背景時間不補入。
- Candidate／正式七個 review tick 的完整 title（含 grip／state）exact；六個 comparison poses 的 pixel differences 只在鞋部。Executable／DLL／staging 為相同 build 的 byte copies，正式 launch GLB／TOML 仍為 v1A。Debug GPU-based validation 已啟用、0 errors，shutdown live report 只有供回報用的 device，無 child object；Release 不啟用 debug layer。
- 影片由實際 runtime tick captures 重組：60 fps／205 frames／1×／3.416667 s，首格 tick 0 Ready，非 wall-clock 錄影。已檢視 side／runtime paired sheets、鞋方向診斷、lifted／grounded close-up 與全段抽樣；**未完成正常速度連續影片自看**。Computer-use 初始化及重試均因 sandbox helper startup 失敗，依本輪指定的 v1A isolated launch workflow 沿用既有 app harness，沒有冒稱 UI 工具成功。

證據全部留 ignored `build/style-feet-v1b/`。最初 relative Blender preview path 曾解析到 `C:/build/style-feet-v1b/side/`；已用 workspace 絕對路徑重製正式 evidence，沒有採用錯路徑內容。動作不變不代表新 shape 的完整 collision／dynamic readability 已接受；深色鞋在 Ready／landing 仍易重疊，batting view 的 upper／sole 分離度有限。造型是否通過由 Michael＋Julia 決定；material／highlight、articulation／sole／cleats、rubber arm 仍延後，S3 未開始。

## Style Feet v1B promotion（2026-09-16）

從 clean `482a116` 開始，HEAD／main／origin/main／live remote 一致，AGENTS.md 未改。Michael＋Julia 已接受 v1B geometry；candidate 三檔與該 commit 交付 hashes／bytes 一致，直接 copy 至正式，驗證後再次逐 byte 相同，沒有重新保存 source 或匯出。正式 fixture 採已驗證 candidate 數值，僅改 acceptance 註解。

正式 SHA256：`.blend` `f54437abc9ddcf75474962c1b34d6c30e689144dd30adb806b7f03df1faa943b`；GLB `d6361c53ad3caf005e19e50f61991508cff856f068d1639b3f29bcc50e06c8fa`；TOML `25d17be36fcb6426030358a7450c9024f5bf4ecf9353812fe54d0ab535095f1b`。

最小 regression：Khronos 0 errors／warnings／infos／hints；目前正式 source 唯讀 evaluated samples 與 accepted candidate 相同，既有 source／GLB／round-trip PASS；Debug／Release 原 S2 motion test PASS（all ticks、skin、contacts、fixed bones、chunking／pause／replay）。Runtime release alignment error 4.9151248e-7 m，原容差不變。沒有重製影片／screenshot，也沒有重跑歷史診斷或宣稱新 GPU 檢查；evidence 在 ignored `build/style-feet-v1b-promotion/`。

v1B review artifact 保留。Front-foot plant orientation 尚待下一階段量測；material／highlight、cleats、foot articulation、rubber arm 延後，S3 未開始。

## Foot Shape v1C：Slender Directional Shoe Silhouette（2026-09-16）

增量 preflight：HEAD／main／origin/main／live remote 均為 `f9b6f2af7e0a6f1a188ad7e7d7b62402e00a5785`，workspace clean，AGENTS.md 未變。正式三檔先前已 promotion 至 v1B，本輪沒有再 promotion；直接從 hash 相同的 `review/style-feet-v1b/` 建立 v1C review candidate。上層 `build/style-feet-v1c/` 的舊 orientation diagnosis 保留，新證據全部在 ignored `build/style-feet-v1c/slender/`。

| Candidate | SHA256 |
|---|---|
| pitcher.blend | `e2eb83e5c80e7f3c93ac7ae4f1ec1c3167618197c20c3d2d958d1d69c84f43ad` |
| pitcher.glb | `ab4f444c1b688ef534589690f59f8dce60f43579a4ff58aaf4559093ae29b925` |
| pitcher.toml | `d515e00178a3557537de247eeaf984c983fa3350c223236a41eef7a235aebf21` |

- Source edit：`revise_shoe_silhouette.py` 只改 270 個 foot positions，仍為 2362 vertices／3936 triangles、220 vertices／foot；未建新 topology。保存前與重開後核對全部 205 格 bones／grip matrices、keys／handles、rest／hierarchy、weights、colors、indices、camera、contacts 與 1922 個非腳部 mesh exact。每腳 59 個平底 contact vertices 全段 exact；planted bottom 差 0（原 1e-7 m guard），landing support positions 未變，airborne 最低 0.000852719 m，原 noncontact >1e-6 guard 保留。
- 尺寸：width／maximum height 不變；toe 增長 0.025 m，length 1.163750→1.188750 m（+2.148%），width:length 0.85→0.832124。Upper 上方六圈 width 0.959512→0.690250 m、width:length 0.85→0.599362。F79／97／108 projected upper width 72.9008→52.4431 px，整鞋約 75.17 px 不變；這量化形狀變化，並非 human readability 通過證據。
- Export：全部非 POSITION accessor bytes 相同，包括 39 animation channels／times／IBMs／JOINTS／WEIGHTS／COLOR／indices；非 foot POSITION exact。TOML 只改 source／GLB hashes。既有 export checker 增加資產／evidence 路徑參數，v1A→v1B 舊 caller 與 v1B→v1C 新 caller 均通過，未放寬任何 tolerance。候選 fixture 只改五格 foot bounds、16 個 foot points／provenance，正式 fixture 未改。
- Khronos：0 errors／warnings／infos／hints。Source／GLB／round-trip PASS，mesh 最大誤差 1.283923e-6／1.744900e-6 m、round-trip grip 1.660393e-6 m，原 0.1 mm 容差保留。`inspect_motion.py` 在原 S0.2C revision 下執行全部 grip／fixed lengths／contacts／head proxy／opening／release checks，未因 style 版本跳過。Runtime release alignment error 4.9151248e-7 m 未變。
- Debug／Release configure、build、正式 v1B CTest 各 4/4 PASS；另直接以 v1C asset 跑原 `pitcher_motion_test` 與 `static_pitcher_test`，兩種 build 均 PASS（含 all-tick、chunking／pause／replay、contacts、fixed bones 與 10 個 static failure cases）。未改 C++／HLSL／CMake／staging／renderer 或依賴。
- Candidate Debug／Release 實際 app play／pause／817 single steps／replay／Complete／minimize→restore PASS；正常速度 wall time 3.4333／3.4371 s，Complete tick 816、exit 0，pause／final hold pixels 不變，背景時間不補入。Debug GPU-based validation 啟用、0 errors，shutdown live report 僅回報用 device，無 child objects。Release 不啟用 debug layer。
- Baseline runtime captures 重用已驗證 v1B：GLB／TOML、executable／DLL／staging bytes 與目前相同；candidate 使用新隔離 launch copy，正式 build 仍為 v1B。七個 review tick 的 title／grip／state 與 baseline exact；六組 paired image 差分只落在鞋部。影片是 candidate 每 4 ticks 擷取一格的 60 fps／205 frames／1× 重組，首格 Ready，3.416667 s，非 wall-clock 錄影；decoder 核對通過。
- 已檢視 actual runtime paired sheets、release／lifted close-up、saved mesh top／oblique 與完整 clip 抽樣；未連續觀看正常速度影片。Computer-use 初始化仍因 sandbox helper startup 失敗，沿用既有 process-targeted app harness，沒有宣稱該 UI 工具成功。鞋底外緣較突出、深色雙鞋重疊及 perspective foreshortening 仍是 human review 項目，未宣告完整 collision 或 style acceptance。

停止於 Michael＋Julia review：v1C 未 promotion，沒有新的 Motion Truth／Character Motion Rule；material／highlight、cleats／sole detail、foot flex／articulation、rubber arm 延後，S3 未開始。


## Rubber Arm v1A candidate（2026-09-16）

Incremental preflight：HEAD／main／origin/main／live remote 為 `e7ee800c2f0a9f624caf78aa4366c72ae5ebc82e`，workspace clean，正式 v1B 未變。Michael＋Julia reject v1C shoe silhouette；orientation Motion Truth 正確，保留 rejected artifact、不 revert，暫停純 geometry shoe polish。局部診斷、修改與 review 路徑由 pitcher README 維護。

Candidate `review/style-rubber-arm-v1a/` SHA256：

- `pitcher.blend`：`8b0226ce7a313f3f9197a47b024cac6c9715be0f6648b0e00a979263f6ce6194`
- `pitcher.glb`：`94052574318a75eda9975f9c03fb054ef7dc9b9be794c293a79036a15a1a7616`
- `pitcher.toml`：`0326894ff911e61d138174cd8c9b1c385d5457a6152c91372e66eb24d21a2df9`

- Source 前後／重開檔：205 frames bones／grip／keys／rest／hierarchy／contacts／camera exact；2050 個非 arm vertices／weights exact（含 v1B feet）。152 arm positions／228 weights 改變，無新 topology。Frame 1 Ready、60 fps／1–205、release f97 不變。
- GLB：除 arm POSITION／JOINTS／WEIGHTS 及 POSITION bounds 外，accessor bytes／structure exact；39 animation channels、IBMs、colors／indices 不變。TOML 只改 source／GLB hashes。Fixture 只更新 24 個 arm points、一個 release bounds 與 provenance，非 arm／grip 原文保留。
- Khronos 0 errors／warnings／infos／hints。第一次誤用 `-o` 作檔名參數只回 usage；修正 stdout 參數後完成，沒有把失敗呼叫當成功。
- Source／GLB／round-trip PASS：mesh 最大誤差 1.283923e-6／1.744900e-6 m，GLB release alignment 5.454740e-7 m。`inspect_motion.py` 原 S0.2C revision 的固定骨長、grip、contacts、head proxy、opening／release checks 全部 PASS，未放寬容差或跳過 assert。
- Debug／Release configure／build／正式 v1B CTest 各 4/4 PASS；另直接以 candidate 執行兩種 build 的既有 S2 motion／static tests 均 PASS，source grip max 1.35952e-6 m。沒有 production code／CMake／staging 修改。
- 實際 candidate app：Debug／Release play／pause／817 steps／replay／Complete／minimize→restore PASS、exit 0；正常 wall time 3.4281／3.4227 s，Complete tick 816，pause／final hold pixels 不變，背景時間不補入。Debug GPU-based validation enabled、0 errors，shutdown live report 只有供 report 使用的 device，無 child objects；Release 無 debug layer。
- Baseline runtime captures 重用 v1B，已核對 GLB／TOML／exe／DLL／staging bytes。Candidate 新擷取 205 格，以 60 fps 編碼 1×，decoder 核對 205 frames／3.416667 s；非 wall-clock 錄影。斜側面 render 相對路徑曾被 Blender 解到 `C:/build/style-rubber-arm-v1a/`，原 PNG bytes 已複製到 ignored evidence，沒有重製／改 source。
- 已檢視 runtime／side 比較、雙角度 clean tube／wire 與全段抽樣；未連續觀看正常速度影片。手套側 Ready／coil 較直厚、部分角度 inner bend／重疊仍在，release 遮擋與 hand 接合觀感待 review，不宣告 style acceptance。

Evidence：ignored `build/style-rubber-arm-v1a/`。正式三檔／fixture 不變，candidate 待 Michael＋Julia review、未 promotion；Character Motion Rules 六條內容不變，S3 未開始。


## Throwing Arm Whip Timing v1A candidate（2026-09-16）

Incremental preflight：HEAD／main／origin/main `4437a57def45102a047c0edb23d9c77afd5196a4`，workspace clean，正式 v1B source hash `f54437abc9ddcf75474962c1b34d6c30e689144dd30adb806b7f03df1faa943b`。Rubber Arm candidate 暫緩 promotion，本輪未使用它。Saved official f97／107／112／126 evaluated local TRS 確認 post-release 姿勢仍在長時段變化，非只依歷史 generator 推論。Timing knots／數字／evidence 由 pitcher README 維護。

Candidate `review/style-arm-whip-v1a/` SHA256：

- `pitcher.blend`：`f6e437cf18aece5bdde97069e4f9ef3ec5ee3d0151c99e90cd31bfaca2a923f2`
- `pitcher.glb`：`98a126e42de3e5b4a1856cf56454bee196144dbc6620e0bc28dcc771b4450a9e`
- `pitcher.toml`：`bf59cb5da49d487b25304ea93bbe59570738366eb24144ac5173c916d836aae5`

- Source／重開檔：只有三個右臂 bones 的 f98–204 local TRS keys retime；205 格 protected body／feet／left arm exact，全部 geometry／weights／rest／hierarchy／camera／contacts exact。F1–97、f205 的完整 evaluated bones／mesh exact。第一次保存前檢查攔到 Blender 重算 LINEAR keys 的 unused AUTO handles；確認只有 unused handles 後改檢 key 值／interpolation 並維持 evaluated exact 檢查，未改 release 前 pose。不是 tolerance 放寬或移除 motion assert。
- GLB：9 個右臂 TRS output accessors 改變；其他 accessors（含 geometry／weights／IBMs／body／left arm）exact，但 grip child 有 exporter bake 的小數值差：translation component max 5.029142e-7 m、rotation 4.100730e-8、scale 2.384186e-7，逐格 vector difference <原 1e-6 binding guard。Source grip keys／binding 不變；所有 f1–97／f205 output bytes exact。TOML 只更新 source／GLB hashes。未人工修補 GLB。
- Fixture 只 patch f108／171 的 hand-grip／受右臂影響的 bounds，以及 source vertices 1800／2100 在這兩格的四個 points，另更新 provenance；其他原文保留，正式 fixture 不變。
- Khronos：0 errors／warnings／infos／hints。Source／GLB／Blender round-trip PASS，mesh 最大誤差 2.098526e-6／1.706817e-6 m；release alignment 5.454740e-7 m 與正式相同。`inspect_motion.py` 原 S0.2C checks 全部 PASS：固定骨長、grip、contacts、head proxy、opening／release，原容差不變。
- Debug／Release configure／build／正式 v1B CTest 各 4/4 PASS；candidate 的原 motion／static tests 兩 build 均 PASS（source grip max 1.02721e-6 m），包含 30／60／120 render chunking 的 deterministic tick、pause／replay、all ticks／skin／contacts／fixed bones。無 C++／HLSL／renderer／staging／CMake 變更。
- 隔離 candidate app play／pause／817 steps／replay／Complete／minimize→restore PASS，Debug／Release 正常 wall time 3.4225／3.4089 s，Complete tick 816、exit 0；pause／final hold pixels 不變、背景時間不補入。Debug GPU-based validation enabled、0 errors；live-object report 僅供報告的 device，無 child objects。Release 無 debug layer。
- Runtime baseline captures 重用已核對 GLB／TOML／exe／DLL／staging bytes 的正式 v1B。Candidate 205 frames 新擷取；f1–97 與 f205 完整 frame pixels exact。短片 60 frames／60 fps／1×／1 s，完整片 205 frames／60 fps／3.416667 s；decoder 核對通過，均為 tick captures 重組，非 wall-clock 錄影。
- 已檢視 authoring 局部 sequence、actual runtime f97–111 每格與全段抽樣；未連續觀看正常速度影片。快速收回後較長時間靠近胸口、hard-elbow 在更快 timing 下是否仍搶眼，留給 human review，不宣告通過。

Evidence：ignored `build/style-arm-whip-v1a/`。正式三檔／fixture 不變，candidate 待 Michael＋Julia review、未 promotion；Rule 3／6 適用但六條原文不改，S3 未開始。


## Throwing Arm Whip Timing v1A promotion（2026-09-16）

Michael 正常速度 human review 接受 v1A timing direction：改善來自 post-release timing compression／速度對比。Preflight HEAD／main／origin/main／live remote 均為 accepted candidate commit `33714438bc94a4263e31919ff4fba27d5512e939`，workspace clean。正式三檔由 `review/style-arm-whip-v1a/` 原樣複製，逐 byte／SHA256 相同；fixture 也原樣同步已驗證的 candidate，沒有重新產生 samples。沒有重新保存 source、export、retime 或修改 metadata。

正式 SHA256：

- `pitcher.blend`：`f6e437cf18aece5bdde97069e4f9ef3ec5ee3d0151c99e90cd31bfaca2a923f2`
- `pitcher.glb`：`98a126e42de3e5b4a1856cf56454bee196144dbc6620e0bc28dcc771b4450a9e`
- `pitcher.toml`：`bf59cb5da49d487b25304ea93bbe59570738366eb24144ac5173c916d836aae5`

- 唯讀正式 source：fresh evaluated samples 與 accepted candidate exact。對照舊 v1B，205 格 pelvis／chest／head／feet／left arm transforms exact；geometry／weights／rest skeleton／camera／contacts exact，鞋仍是 v1B。F1–97、f205 完整 bones／mesh exact；release f97／tick 384、clip 60 fps／1–205／3.4 s 不變。
- Khronos 0 errors／warnings／infos／hints。Source／GLB／round-trip PASS，mesh max 2.098526e-6／1.706817e-6 m；release alignment 5.454740e-7 m。既有 motion checks PASS，grip binding、固定骨長、contacts、head proxy、opening／release guards 全部保留，未放寬容差。
- Debug／Release configure／build／正式 CTest 各 4/4 PASS，含 S2 all-tick／skin／30、60、120 chunking determinism／pause／replay、fixed bones／contacts。實際 build 載入的 GLB／TOML bytes 已確認等於 promoted official。
- 正式 Debug／Release app play／pause／817 steps／replay／Complete／minimize→restore PASS、exit 0；正常 wall time 3.4084／3.4280 s、Complete tick 816，背景時間不補入。Pause／final hold 使用記憶體內像素比較，不保存 screenshots。Debug GPU-based validation enabled、0 errors；shutdown live report 只有供 report 的 device，無 child objects。

Evidence：ignored `build/style-arm-whip-v1a-promotion/`；沒有新的影片／visual review，也沒有 production code 變更。Regression 後再核對正式三檔與 accepted candidate 仍 byte-identical。

Whip review artifact 保留。Rubber Arm 舊 timing candidate 不 promotion／merge，待新正式 timing 下重新評估；v1C shoe 維持 rejected artifact，material／highlight／cleats／articulation 延後。六條 Character Motion Rules 原文不變，S3 未開始，M1 未完成。
