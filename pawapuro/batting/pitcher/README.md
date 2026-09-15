# 右投手 Authoring Sample（S0.1）

2026-09-15：Michael 判定原 S0 motion 未通過；本版是 **S0.1 粗動作候選，等待 Michael＋Julia review，不進入 S1**。已更新可編輯 `.blend`、GLB 與 metadata；尚未接入 app。

## Michael 先看什麼

Evidence 在忽略的 `build/pitcher-s01/`：

| 檔案 | 用途 |
|---|---|
| `before-normal.mp4`／`after-normal.mp4` | 同一斜側面全身 camera、1×、60 fps。Before 2.516667 s；after 3.416667 s，沒有拉成相同長度。 |
| `before-batting.mp4`／`after-batting.mp4` | 相同 staging camera／horizontal shift。只有投手與支撐平面，沒有打者／球場，不是 app capture。 |
| `after-slow.mp4` | 同一批 after frames，每格重複四次，0.25×、60 fps、13.666667 s。 |
| `contact-sheet.png` | Ready、coil、stride、前腳接觸、軀幹打開、前甩、release、早期收勢、右腳跟進／落地、最後回穩，標明 frame／clip time。 |
| `release-continuous.png`／`recovery-continuous.png` | Release 周圍 23 個連續影格；右腳落地前後 37 個連續影格。 |
| `whole-motion.png`／`batting-contact-sheet.png` | 整段動作 overview／打席視角檢查。Overview 不是正常速度播放證據。 |
| `motion-diagnostic.png` | Evaluated pelvis／chest／head yaw、grip speed、俯視肩線／grip arc／腳底接觸。乾淨影片不帶這些標記。 |
| `roundtrip-contact-sheet.png` | 匯出的 GLB 重新匯入 Blender 的對照。 |

原 `build/pitcher-s0/` 的影片與 PNG 保留；S0 source／GLB／TOML 另保存在 `build/pitcher-s01/before/`。乾淨 before 是以保留的 S0 `.blend` 重製，只隱藏固定青色 release 診斷環；camera、比例、原動作與播放長度不變。

**證據限制**：本輪實際讀取／取樣 `.blend`、檢視渲染影格，且用 Blender 隨附 decoder 核對 MP4 的 FPS／frame count。瀏覽器播放器工具啟動失敗，沒有完成參考影片觀看，也沒有完成 Codex 正常速度播放自看；不以 contact sheet 冒稱看過影片。Michael 提到的遊戲參考圖在本次可讀 attachments 中不存在。這版 timing 依文字 feedback 與動畫原則提出，**不是 reference-verified timing**。研究與 motion brief 見 [design](../../../docs/design/batting-feel.md#s01-motion-reblocking)，量測與限制見 [environment](../../../docs/development/environment.md#s01-motion-reblocking2026-09-15)。

## 可編輯來源與最小契約

- `pitcher.blend` 是日常 authoring truth。預設 frame 49、斜側面 camera view；在 Timeline／3D Viewport 按 Space 播放。互動 viewport 不保證實時 60 fps。
- 原 mesh、拓樸、五色、weights、13-node skeleton／rest transforms、camera、placement／scale 完全保留；2362 vertices／3936 triangles。一個 mesh／skin／`pitch_R` clip、15 個 GLB nodes、39 TRS channels。
- 右臂使用原 rest 骨段長度的 FK 方向曲線，肩部隨 chest；不移動肘／手關節來伸縮肢體。隱藏 bend、連續 tube、球形手與 detached feet 保留。沒有新增 solver／骨架／framework。
- `create_sample.py` 只供最初 S0 生成；`reblock_motion.py` 只供從 S0 source 明確另存 S0.1 candidate，拒絕已存在的目的檔。兩者都不是日常 export 的依賴。
- 本版 authoring curve 的結果烘焙為可編輯的逐 frame FK TRS keys；saved source 已含全部動作。`export_sample.py` 讀目前存檔，不重建、不保存 `.blend`；輸出 GLB／TOML 並檢查 source hash 未變。
- 60 fps／fps_base=1，frames 1–205 → GLB 0–3.4 s；playback_speed=1、loop=false。影片含首尾端點，共 205 個顯示 frames，因此長 3.416667 s。
- `.blend` 唯一 timeline marker `release` 在 frame 97 → 1.6 s → 未來 240 Hz tick 384。Metadata／preview release label 從 marker 產生；不是 Native release integration。
- `.blend` scene `key_poses` 保存 review poses，release 另從唯一 marker 插入；`contact_intervals` 保存四段腳底支撐區間。Export 產生 `[review]`／`[[contacts]]`，驗證與圖表讀取這些來源，不使用原 S0 frame 73／91／151 的檢查時間表。
- 右腳地面支撐 1–85，86–170 離地，171–205 恢復支撐；左腳 1–17 初始支撐，18–78 抬腿／跨步，79–205 前腳支撐。接觸不是受力模擬；落地後 chest／手／頭仍回穩，沒有 idle hold 補長。
- `grip` 是 `hand_R` 的 child，代表持球球心，固定 local binding；持球在 marker 後一格隱藏，不模擬後續球路。Reference world position、球半徑及 Native 初始條件不變。

### 空間與格式

長度已是公尺，`2.45` 是初建比例 scale，不能在 runtime 再乘；rest mesh 高約 2.56515 m。Local origin 為後腳地面參考，placement 為 staging 的 `(0, 0.355, 18.5166)` m，scale=1。

Blender local = `(-game_local.x, -game_local.z, game_local.y)`；標準 glTF +Y-up export 得到 GLB local = `(-game_local.x, game_local.y, game_local.z)`。未來 GLB → game 反射 X 一次、反轉 winding，再加 placement；transform／inverse bind 使用相同轉換。本輪不驗證 C++ importer。

GLB 只含 `POSITION`、`COLOR_0`、`JOINTS_0`、`WEIGHTS_0`；四個 influences 槽位、最多兩個非零 weights。Triangles、LINEAR animations；無 materials／textures／normals／morph／extensions。Blender 預覽使用原 vertex-color emission／Raw view transform，未改 lighting。重新匯入會把 color 轉 byte color，保留既有量化限制。

## 日常 export／驗證

先保存 Blender 中的編輯，再從 repo 根目錄執行；不跑 generator：

```powershell
$Blender = 'C:/astra-dev/tools/blender-4.5.13-windows-x64/blender.exe'
$Pitcher = 'C:/astra-dev/pawapuro/pawapuro/batting/pitcher'
$Evidence = 'C:/astra-dev/pawapuro/build/pitcher-s01'
& $Blender --background --factory-startup "$Pitcher/pitcher.blend" --python-exit-code 1 --python "$Pitcher/export_sample.py"
& 'C:/astra-dev/tools/gltf-validator-2.0.0-dev.3.10/gltf_validator.exe' -o -a "$Pitcher/pitcher.glb" > "$Evidence/validator.json"
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/verify_sample.py"
& $Blender --background --factory-startup "$Pitcher/pitcher.blend" --python-exit-code 1 --python "$Pitcher/inspect_motion.py" -- --output "$Evidence/motion-audit.json"
```

逐一檢查 exit code。Validator 檢查格式；`verify_sample.py` 對整段 205 frames 比較 source、獨立 GLB TRS／skinning 計算及空白 Blender scene round-trip，並核對 hashes、weights、inverse bind、release、腳底固定 transforms。原 0.1 mm alignment／deformation 容差未放寬。`inspect_motion.py` 核對固定骨段長度、grip local binding、落地／離地區間、地面穿入、前後出手方向與 pelvis → chest 時序。右臂／頭部橢球 proxy 不涵蓋帽子、肩部 attachment 或其他身體部位，不是完整 self-collision 保證。

## 重製預覽

```powershell
& $Blender --background --factory-startup "$Pitcher/pitcher.blend" --python-exit-code 1 --python "$Pitcher/render_preview.py" -- --output "$Evidence/source-side" --animation --width 1280
& $Blender --background --factory-startup "$Pitcher/pitcher.blend" --python-exit-code 1 --python "$Pitcher/render_preview.py" -- --output "$Evidence/source-batting" --animation --camera batting --width 1920
& $Blender --background --factory-startup "$Pitcher/pitcher.blend" --python-exit-code 1 --python "$Pitcher/render_preview.py" -- --output "$Evidence/roundtrip-side" --roundtrip --width 1280
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/encode_preview.py" -- --frames "$Evidence/source-side" --output "$Evidence/after-normal.mp4"
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/encode_preview.py" -- --frames "$Evidence/source-side" --output "$Evidence/after-slow.mp4" --slow
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/encode_preview.py" -- --frames "$Evidence/source-batting" --output "$Evidence/after-batting.mp4"
& 'C:/Users/USER/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' "$Pitcher/assemble_sheets.py" --evidence $Evidence
```

Encoder 由 render manifest 判斷 camera、frame range／FPS，拒絕缺格、多格與錯誤順序；輸出旁的 JSON 記錄實際 decoded frame count／FPS／倍率。Retime 後請用新的空白 evidence 子目錄渲染，避免舊尾幀留下；不要覆蓋 S0 before。

重製 before 時開 `build/pitcher-s01/before/pitcher.blend`，render 到 `before-side`／`before-batting`，再以上述 encoder 輸出 `before-normal.mp4`／`before-batting.mp4`。這是有意保留的 S0 snapshot；新 clone 可從 Git commit `fb97f46` 取得到獨立忽略目錄，不 reset 當前 source。Sheets 需要這份 before frames。

本輪沒有 C++／HLSL／renderer／staging Data 變更；沒有重跑 runtime／GPU 測試。球路、動態遮擋、early-flight 對比與正常速度動作可讀性，仍待後續授權／人類檢查。
