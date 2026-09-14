# M1 Step 0 — 開發環境

核對日期：2026-09-14。本文件記錄本機觀察結果與準備缺項；scope 與驗收標準由
[milestone](../milestones/01-batting-feel.md) 定義。已完成 **Step 1 交付 1 的靜態場景驗證**；Step 1 整體尚未完成，後續已加入靜態 staging calibration 與啟動 Data；結果與建置方式見各節。

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

### 實際比較與驗證

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
