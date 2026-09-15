# 右投手 Authoring Sample（S0）

狀態：2026-09-15 已製作第一版，等待 Michael＋Julia review。這是可編輯 authoring source 與 runtime candidate；**尚未接入 Pawapuro app，沒有 Native release／球路驗證**。

## 先看什麼

本機 evidence 在 `build/pitcher-s0/`，不提交影片或逐 frame renders：

- `pitcher-normal.mp4`：斜側面、1×、60 fps。
- `pitcher-slow.mp4`：同一份 source frames、0.25×，方便看跨步／手臂路徑。
- `pitcher-batting.mp4`：staging camera 與 horizontal lens shift，僅投手／支撐平面，沒有打者／球場。不是 app capture。
- `seven-key-poses.png`：七個關鍵姿勢。
- `release-grip-reference.png`：frames 90／91／92 的同位置放大，固定青環對照持球。
- `roundtrip-comparison.png`：原始 `.blend` 與 GLB 重新匯入後的對照。
- `sample-validation.json`、`validator.json`：數值／格式證據。

## 開啟與播放

本機 PowerShell：

```powershell
& 'C:/astra-dev/tools/blender-4.5.13-windows-x64/blender.exe' --factory-startup 'C:/astra-dev/pawapuro/pawapuro/batting/pitcher/pitcher.blend'
```

檔案預設停在 frame 43（抬腳）、斜側面 camera view。滑鼠放在 3D Viewport／Timeline，Space 播放／暫停；Shift＋Left 回到 frame 1。Timeline 的唯一 `release` marker 在 frame 91。Blender timeline 可以重複播放；**clip 契約仍是一次播放、不循環**，不是 runtime loop 設定。要看正確速度，先用離線輸出的影片；互動 viewport 不保證每次都能即時跑滿 60 fps。

## Source、尺度與座標

- `pitcher.blend` 是可編輯來源。原創低細節 mesh／FK armature／動作由 Codex 依現有 `reference_scene.cpp` 與 `staging.toml` 製作，沒有外部角色或 mocap 資產。
- `create_sample.py` 僅供**初次生成或另建候選**；目的 `.blend` 已存在就拒絕。不要用它覆蓋手動修過的來源。
- `export_sample.py` 讀已存檔的 `.blend`，產生 `pitcher.glb`／`pitcher.toml` 與 source samples；不呼叫 generator、不保存 `.blend`。匯出 mesh 在記憶體中暫時解除 parent，避免 skin mesh 非 root 的警告，之後復原。
- 本輪 asset 的長度已是公尺。`2.45` 只在初建時把既有比例換成公尺，**不是總高，也不在 placement 再乘一次**；rest mesh 高約 2.56515 m。
- local origin 是既有後腳地面參考。runtime candidate placement 為 staging 的 `(0, 0.355, 18.5166)` m；scale=1。
- Blender local = `(-game_local.x, -game_local.z, game_local.y)`。投手朝 Blender +Y，右投手臂在 Blender +X。
- 標準 Blender +Y-up 匯出得到 GLB local = `(-game_local.x, game_local.y, game_local.z)`。投手朝 GLB −Z，右手在 GLB +X。
- 未來 GLB → Pawapuro：反射 X **一次**，再加 placement；transform／inverse bind 要使用相同 basis 轉換，triangle winding 隨反射翻轉。D3D12 本身不替 importer 選座標慣例。本輪只驗證 authoring／GLB，不宣稱 Native 已正確實作轉換。

## 本 sample 的內容契約

- 一個 `PitcherMesh`，2362 vertices／3936 triangles；一副 13-node armature，含 root 與 grip。
- Root 下有 pelvis、兩隻獨立 detached feet；chest／head／左右 arm／forearm／hand 提供必要控制。手臂是跨隱藏 bend 的連續 tube mesh；不靠 nonuniform bone scale 製造伸長，以免 parent-scale shear 在 TRS 匯出時失真。
- 頭／帽／臉、球形手、手套與鞋採剛性 weights；衣襬與手臂採少量混合 weights。無 fingers、facial rig、constraints／IK runtime 或布料模擬。
- 只有 `POSITION`、`COLOR_0`、`JOINTS_0`、`WEIGHTS_0`；每 vertex 四個槽位，實際最多兩個非零 influences。五種平塗顏色存在 `COLOR_0`，不是只有 viewport object color。GLB 不帶 materials／textures／normals／morph／compression／extensions。
- 所有幾何已三角化。一個 `pitch_R` clip，39 個 TRS channels，實際輸出全部為 LINEAR；60 fps、fps_base=1、frames 1–151。GLB time 0 = source frame 1，clip duration=2.5 s；playback_speed=1、loop=false。
- 七個 review poses：1 ready、43 leg lift、67 stride、78 torso rotation、85 arm acceleration、91 release、121 follow-through；151 是收勢後的結尾。這不是七個等長停頓。
- **唯一 authoring release marker** 在 `.blend` frame 91。TOML 的 time=1.5 s、未來 240 Hz tick=360，均由 `(marker.frame − start_frame)` 匯出計算，不手填另一份 timing。
- `grip` 是 `hand_R` 的 child，原點代表球心。Grip 與球形手中心相距約 0.1346 m；hand radius 約 0.1433 m。Grip 沒有獨立動畫偏移來湊 reference。
- 輔助持球跟隨 grip，在 frame 92 起隱藏。固定青環、球、支撐平面、兩個 camera 都在 `AuthoringOnly` collection，未匯入角色 GLB；不模擬 release 後球路。

Blender preview 使用 vertex-color emission 與 Raw view transform，方便對照既有 app 的平塗數值。這是 authoring 顯示設定，沒有新增 runtime material／lighting 系統。GLB 重新匯入 Blender 會將 color attribute 轉成 byte color，存在小量顏色量化，實測見 environment。

## 日常匯出與驗證

先在 Blender 保存想要匯出的 source；再從 repo 根目錄的 PowerShell 執行。所有工具使用明確位置，不改全域 PATH：

```powershell
$Blender = 'C:/astra-dev/tools/blender-4.5.13-windows-x64/blender.exe'
$Pitcher = 'C:/astra-dev/pawapuro/pawapuro/batting/pitcher'
$Evidence = 'C:/astra-dev/pawapuro/build/pitcher-s0'
& $Blender --background --factory-startup "$Pitcher/pitcher.blend" --python-exit-code 1 --python "$Pitcher/export_sample.py"
& 'C:/astra-dev/tools/gltf-validator-2.0.0-dev.3.10/gltf_validator.exe' -o -a "$Pitcher/pitcher.glb" > "$Evidence/validator.json"
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/verify_sample.py"
```

逐一檢查 exit code。`verify_sample.py` 是本 sample 的離線檢查，不是 production C++ importer：核對檔案 hashes、節點／clip／attributes、weights、inverse binds、marker、right-hand sign、planted-foot transforms、GLB pose 與 source vertices，然後在**空白 Blender scene** 重新匯入 GLB 比較十個姿勢。Importer 自己產生的 bone-display Icosphere 不屬於 GLB mesh，檢查時依 custom_shape 引用排除。

## 重製預覽

延續上方 PowerShell 變數：

```powershell
& $Blender --background --factory-startup "$Pitcher/pitcher.blend" --python-exit-code 1 --python "$Pitcher/render_preview.py" -- --output "$Evidence/source-side" --animation --width 1280
& $Blender --background --factory-startup "$Pitcher/pitcher.blend" --python-exit-code 1 --python "$Pitcher/render_preview.py" -- --output "$Evidence/source-batting" --animation --camera batting --width 1920
& $Blender --background --factory-startup "$Pitcher/pitcher.blend" --python-exit-code 1 --python "$Pitcher/render_preview.py" -- --output "$Evidence/roundtrip-side" --roundtrip --width 1280
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/encode_preview.py" -- --frames "$Evidence/source-side" --output "$Evidence/pitcher-normal.mp4"
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/encode_preview.py" -- --frames "$Evidence/source-side" --output "$Evidence/pitcher-slow.mp4" --slow
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/encode_preview.py" -- --frames "$Evidence/source-batting" --output "$Evidence/pitcher-batting.mp4" --batting
& 'C:/Users/USER/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' "$Pitcher/assemble_sheets.py" --evidence $Evidence
```

最後一個命令使用本機已存在的 Python／Pillow；別台機器可使用具有 Pillow 的 Python。影片使用 Blender 隨附 encoder，不另裝 FFmpeg。每個 source frame 都離線渲染，影片解碼後核對 60 fps／frame count；正常影片保留起、迄兩個端點，共 151 frames（2.516667 s），不是 clip timeline 變成不同速度。慢速每個 frame 重複四次，共 604 frames（10.066667 s）。

## Review 限制

支撐腳／落地腳的固定區間已用 transforms 檢查；粗動作、明顯穿插與 silhouette 也有逐姿勢圖檢視，但沒有 cloth／self-collision solver。加速時手臂和大頭／帽簷的投影重疊、跨步與收勢的重量感仍需 Michael＋Julia review。這一版先提供有界的粗動作，不宣稱已達正式動畫品質。

Batting camera 使用現有 position／target／36° vertical FOV 與推導的 horizontal shift；本輪 scene 只有投手與簡單支撐平面，不能驗收真正打者／球場背景中的動態遮擋或 early-flight contrast。

跨概念責任見 [設計](../../../docs/design/batting-feel.md)，本輪版本、量測、限制見 [environment](../../../docs/development/environment.md)。S0 review 後仍須另行授權 S1。
