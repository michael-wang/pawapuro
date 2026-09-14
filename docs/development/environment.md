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
