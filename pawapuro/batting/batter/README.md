# Left-handed Batter — S0 / Runtime S1 / S2 accepted

2026-09-16。Michael＋Julia 已接受 Left-handed Full Swing S0 authoring baseline：compact Ready、late gather、抬腳／短步、plant 先於主加速、body turn→手棒 lag、短促 acceleration、contact-area pass 與完整 finish。本目錄原有三檔就是 canonical accepted source，沒有再 promotion／save／export。**Batter Runtime S1 static import 已獲 Michael＋Julia human acceptance；S2 synchronized swing playback 已獲 Michael＋Julia human acceptance**；不代表 runtime animation、碰撞、hit quality、contact presentation 或 M1 通過。設計與來源觀察由 [batter-motion.md](../../../docs/design/batter-motion.md) 擁有。

## S0 authoring review evidence（歷史交付）

Ignored `build/batter-authoring-s0/`：

- `swing-batting-1x.mp4`：既有 staging 對應 batting camera，完整乾淨 swing。
- `swing-side-1x.mp4`：完整 three-quarter，輔助看支撐／lag／收勢；依全部 225 格的角色與球棒範圍取景。
- 兩支均為 **60 fps、1×、225 格**；首格是 f1 Ready。Clip 時間 0–3.733333 s，影片 3.750 s，差別僅最後一格顯示 1/60 s。無球、聲音、VFX、診斷箭頭。
- `batting-contact-sheet.jpg`、`side-contact-sheet.jpg`：11 個 phase poses；`plant-acceleration-continuous.jpg`：f103–126 每格；`follow-settle-sequence.jpg`：後段繞身至 finish。
- `hands-bat-lag-closeup.jpg`、`batting-hands-bat-lag-closeup.jpg`、`foot-support-closeup.jpg`：手棒與接地放大。
- `motion-diagnostic.jpg`：evaluated pelvis／chest yaw、腳底、hand／grip／barrel／tip arcs、world／screen spacing；只作診斷。

已實際檢視上述關鍵／連續影格與 source 全 225 格數值；**未完成播放器中的正常速度連續自看**。解碼確認影片 FPS／格數不等於視覺驗收。其後 Michael＋Julia 已完成正常速度 human review 並接受上述 S0 動作。

## 可編輯 source 與最小契約

- `batter.blend` 是 authoring truth，預設停 f1；不要用 generator 取代日常編輯。`create_sample.py` 只作首次建檔且拒絕覆寫既有 source；`refine_sample.py` 是本次一次性輪廓／vertex-color 修訂，有重跑 guard。最後 source 另修正 AuthoringOnly zone guide 與 three-quarter 取景，已同步記入建檔腳本；不是 export 的隱藏步驟。
- `export_sample.py` 讀取已存檔 source、取樣／檢查並產生 GLB／TOML／evidence，**不呼叫 create／refine，也不保存 .blend**。手改後先在 Blender 明確存檔，再 export。
- 一個 armature、17 joints、兩個 mesh：`BatterMesh` 與獨立 equipment `Bat`。Root／pelvis／chest／head、左右各 3 個 hidden arm controls 與 hand、detached feet，以及 3 個 bat semantic joints。沒有 fingers、IK、兩手 solver 或通用角色系統。
- Rubber arms 使用具體曲線的 hidden bend controls，烘焙 location／quaternion／scale。固定 bone length 指 **control 本身長度與 unit scale**；相鄰 controls 的距離可變，並非宣稱真人上下臂節長固定。S0 視覺 baseline 已接受，後續 polish 不在 S1 scope。
- `hand_R → bat_grip → bat_barrel / bat_tip` 是唯一 attachment hierarchy；Bat 全部 rigid weights 給 bat_grip，整段不放棒。另一手由同一握把軸的 authored offset 協調，沒有雙 parent。Source 持有 mesh／rig／action；匯出為自包含 GLB。S0 沒有 runtime caller；S1 lifetime 見下節。
- 公尺／比例已烘進 source，`runtime_scale=1`。GLB → game 為 `(-x,y,z)` 加目前 staging batter placement `(1.25,0.008,0)`；TOML placement 只記 provenance，不能作另一份 gameplay Data。Blender basis 為 game-local `(-x,-z,y)`。
- `AuthoringOnly` 有 home plate、目前 strike-zone plane、placement、review cameras；timeline 有 pitcher guides。匯出只選 `BatterAsset`，不含 floor／guides／camera／light。保留 named bat nodes 是為本支 swing 的 path 診斷。
- GLB：單一 non-looping `swing_L`、60 fps、1–225、LINEAR baked samples、skin／vertex colors、沒有 materials／textures。兩個 mesh 的 S1 static runtime caller 已實作（見下節），animation caller 留待 S2。

## 本候選 timing／support

全部 **1-based authoring frames**；`time=(frame-1)/60`，delivery start=0。不是 reference capture frame。

| Pose／phase | Frame／秒 | 實際關係 |
|---|---|---|
| Ready | 1–65／0–1.067 | Compact 手棒、雙脚穩定；pitcher early leg lift 時安靜 |
| Gather | 72／1.183 | 明顯啟動對應 pitcher commit-forward guide；body 小幅準備先於腳離地 |
| Lead lift | 88／1.450 | 右前腳鞋底最高約 0.32 m，左後腳支撐；是中等 lift 第一候選 |
| Stride | 99／1.633 | 出手 guide f97 前後下降，短步朝 pitcher；前腳中心總前移 0.15 m |
| Plant／turn | 107／1.767 | 右前脚建立完整 sole support；骨盆已打開，胸與手棒仍保留 lag |
| Bat lag | 113／1.867 | 手在身旁，棒軸開始下降、barrel 留後；pelvis／chest 不是同曲線縮放 |
| Acceleration | 約 116–124／1.917–2.050 | 短段 forward sweep；f118 已主加速，barrel spacing f121 最高 |
| contact_area | 121／2.000 | 代表 barrel 通過 authoring zone；不是 collision 或 gameplay contact |
| Follow-through | 134／2.217、165／2.733 | 胸口繼續轉、手棒繞身，後腳 toe pivot／heel up，頭只小幅跟進 |
| Finish | 225／3.733 | 身體先收、手棒持續縮小 spacing 至最後姿勢，沒有複製長尾 hold |

前腳 `foot_R` sole contacts：f1–71、f107–225；後腳 `foot_L` full sole：f1–114，f115–225 改 toe support、heel lift，最後不強迫壓平。Head 不與 chest 剛性同步。這是 authoring support 意圖，不是 runtime 接觸物理。

`contact_area` 唯一 timeline marker f121 導出 TOML；pitcher guides 僅註記 coil 0.8／stride 1.183／front-contact 1.3／release 1.6／arrival 約 2.0 s。Barrel 此格 game `(-0.008927,0.793643,0.434232)` m，落在 staging zone XY 內；與 plane Z=0.4318 m 相差 0.002432 m，沒有 snap 或更改 strike zone。附近 world spacing 最高在 f121：0.490453 m/frame，同格 screen spacing 49.928671 px/frame；screen peak 則在 f118：172.864071 px/frame（1920×1080 accepted camera 投影）；這是離散 path spacing，非球棒碰撞速度。

## 重跑 export／驗證／預覽

工具沿用本機 Blender 4.5.13、Khronos validator 2.0.0-dev.3.10，沒有安裝 dependencies。從 repo root 的 PowerShell：

```powershell
$b = 'C:/astra-dev/tools/blender-4.5.13-windows-x64/blender.exe'
$e = 'C:/astra-dev/pawapuro/build/batter-authoring-s0'
& $b -b pawapuro/batting/batter/batter.blend -t 4 --python pawapuro/batting/batter/export_sample.py -- --evidence $e
& $b -b -t 4 --python pawapuro/batting/batter/verify_sample.py -- --evidence $e
& $b -b pawapuro/batting/batter/batter.blend -t 4 --python pawapuro/batting/batter/inspect_motion.py -- --evidence $e
& 'C:/astra-dev/tools/gltf-validator-2.0.0-dev.3.10/gltf_validator.exe' -o pawapuro/batting/batter/batter.glb
& $b -b pawapuro/batting/batter/batter.blend -t 4 --python pawapuro/batting/batter/render_preview.py -- --output "$e/batting" --camera batting --width 1280 --animation
& $b -b pawapuro/batting/batter/batter.blend -t 4 --python pawapuro/batting/batter/render_preview.py -- --output "$e/side" --camera side --width 1280 --animation
& $b -b -t 4 --python pawapuro/batting/batter/encode_preview.py -- --frames "$e/batting"
& $b -b -t 4 --python pawapuro/batting/batter/encode_preview.py -- --frames "$e/side"
```

先 export 產生同版 source samples，再 verify；不要用舊 evidence 核對新 source。檢查腳本失敗應修原因，不放寬容差。

## 實測驗證與限制

| 項目 | 結果／原容差 |
|---|---|
| 重新開啟 saved source／export hash | 通過；export 不保存 source |
| Khronos | 0 errors／warnings／infos／hints；2 meshes、1 skin、17 joints |
| 225 格 direct GLB skinning vs source | mesh 最大 1.5371e-6 m、bounds 1.1921e-6 m；容差 1e-4 m |
| 225 格 Blender import round-trip | mesh 最大 2.3429e-6 m、semantic 2.0659e-6 m；容差 1e-4 m |
| Color round-trip | Euclidean RGB 最大 0；原容差 0.005。Source 改用 BYTE_COLOR 儲存以與匯入的 sRGB8 精度對齊，沒有放寬驗證 |
| Fixed bone length／weight sums | 最大 1.6891e-7 m／1.4902e-8；容差 2e-5 m／1e-6 |
| 足底／contacts | 最低 −7.4506e-8 m；容差 −1e-5 m。上述 full-sole intervals transform drift 0；容差 2e-6 |
| Rear toe／bat attachment | toe vertex drift 2.7525e-7 m；bat_grip relative-to-hand drift 5.0106e-7；各容差 2e-6 |
| Clip／marker | 60 fps、LINEAR、單 clip、1–225、contact_area f121；TOML 由 source 導出並檢查 hashes |

S0 交付時列出的觀察／限制（之後 baseline 已 human accepted）：f118 screen spacing 突增是本版短促加速候選，是否太急仍待正常速度判斷；簡化 torso 的轉向輪廓仍淡；手臂近下巴時會疊成粗輪廓，continuous rubber silhouette 尚待 review；早期 follow-through 手棒在大頭後方遮擋，不能只靠曲線驗證就說可讀。全片 bat surface vertices 對 head／helmet ellipsoid 檢查為 0 inside samples，但**不是完整 triangle collision**，不涵蓋 brim、手臂／torso 自穿插或所有遮擋。S0 的持棒、lift 與 settle 方向現已獲 human acceptance；不代表後續 runtime animation 已驗證。

S0 交付沒有執行 C++ build／CTest／GPU validation；該輪 production code、staging、pitcher 完全未改。沒有 runtime BatterMotion、bat-ball collision 或 contact presentation。完成此 candidate 後停止。


## Batter Runtime S1 — static bind import candidate

### Review evidence

Ignored `build/batter-runtime-s1/`：先看 `debug-ready.png`（完整 1920×1080 startup）、`debug-batter-crop.png`、`debug-composition.png` 與 `debug-tick-392-early8.png`（release 後 8 ticks）。Release 對應 `release-s1-ready.png`／`release-s1-batter-crop.png`／`release-s1-tick-392-early8.png`。這些是實際 app screenshots，非 Blender renders。

目前 app 打者是 **GLB bind/rest geometry，不是 frame-1 Ready，也沒有播放 swing_L**。測試腳本完成真實 app 播放後逐 tick 擷取，畫面已實際檢視：一名左打者在既有 batting box，棒在 rear／plate 側、detached feet 可見，原投手／好球帶與 early-flight sightline 保留。Bind pose 的手臂較靠臉與 bat 輪廓是 source rest 外觀，沒有為取景修 source。Runtime S1 的 static GLB transport、BatterMesh＋Bat shared hierarchy、placement／scale／handedness、bat attachment、grounding、composition／sightline 現已獲 Michael＋Julia 接受；不能推定 animated occlusion 安全。

### 最小 runtime contract／ownership

- Engine `MeshGlb` 改為 `primitives` 列表加唯一共享 nodes／skin／clip；每個 `GlbPrimitive` 擁有 mesh／node name、owning node index、raw bind vertices、indices、influences、mesh-node-transformed triangles。只支援目前兩 caller 所需的 named mesh nodes、每 mesh 一個 primitive、同一 skin／LINEAR clip；不支援 mesh instancing、多 skin、materials 或 arbitrary glTF scene。
- Parser／檔案緩衝在 `read_mesh_glb` 返回前釋放，結果全 owned。既有 pose evaluator 仍只評估一份 hierarchy；skinning helper 接收指定 primitive 和共享 pose。Pitcher caller 僅 mechanical adaptation，S3 sequencing／physics 未改；本輪不呼叫 batter pose evaluation／skinning。
- Pawapuro `static_batter.cpp` 同步讀一次 asset，驗證唯一 `BatterMesh`／`Bat`、共享 `BatterRig`、唯一 `hand_R → bat_grip → bat_barrel/bat_tip`，Bat 的非零 weights 必須全部指向 bat_grip。CPU 展開兩 mesh，X reflection／winding reversal 各一次，加唯一 `staging.batter_blockout_position_m`，scale=1，不使用 height 重縮放。
- `StaticBatter` 擁有 startup world triangles；`reference_scene` 複製進既有 scene vertices，再由 renderer 同步複製至 immutable GPU buffer。沒有把 parser 指標／臨時借用留到 draw，GPU 資源仍依既有 fence／shutdown lifetime。移除 procedural batter，無隱藏 fallback；錯誤由 startup 回報 owner／path／reason。
- 依 `configure_file(COPYONLY)` 將 canonical GLB 放到 Debug／Release 的 `batting/batter/batter.glb`，runtime 從 executable base path 讀取。S1 不解析 TOML，不將 metadata placement／contact_area 當 runtime truth；原 TOML 的 candidate 字串保留為 S0 匯出快照，本文件記錄最新 human acceptance。
- **renderer／HLSL 完全未改**，沒有第二個 dynamic buffer。Source `.blend`／GLB／TOML bytes 未改；正式 GLB SHA256 `6977f690c0aad047518882db310246893f6bab4537f71261b0c423d6a343bcf2`。

### 實測 transport

| Primitive | Source vertices | Triangles | Expanded vertices |
|---|---:|---:|---:|
| BatterMesh | 2608 | 4488 | 13464 |
| Bat | 142 | 280 | 840 |
| 合計 | 2750 | 4768 | 14304 |

20 nodes／17 shared joints。以下為 metres、game basis、bind/rest：

| 項目 | 值 |
|---|---|
| Combined local bounds | `(-0.519000, 0, -0.795800)` → `(0.468000, 2.182592, 0.698000)` |
| Staging placement／scale | `(1.250000, 0.008000, 0)`／1 |
| World bounds | `(0.731000, 0.008000, -0.795800)` → `(1.718000, 2.190592, 0.698000)` |
| bat_grip world | `(0.830000, 1.048000, -0.360000)` |
| bat_barrel world | `(0.958455, 1.852986, -0.634038)` |
| bat_tip world | `(1.009240, 2.171236, -0.742378)` |

鞋底 local Y=0；world Y=0.008 來自既有 staging 的 8 mm offset，未自行吸地。Bounds 包含棒，最大高度不是角色頭高。Meshes 同 basis，握把／barrel／tip 依同 hierarchy world transform 轉換。

### 驗證

- Debug／Release build、各 **6/6 CTest** 通過。`static_batter_test` 檢查正式 hash／兩 primitive counts、node ownership、finite accessors／weights、bounds、basis／winding／colors、semantic rest positions、staging-only placement（改 height 不二次縮放）、ball／overlay range isolation。
- 第二 mesh translation fixture 只移動 Bat，不影響 body；9 個 missing／malformed cases 包含第二 mesh POSITION／indices／JOINTS／weights、missing shared skin、錯誤 attachment 與 missing semantic node。沿用 1e-5 position／color、2e-5 weights 等容差，沒有刪除或放寬既有 assert。
- 原 `pitcher_motion`／`pitch_delivery`／`static_pitcher`／`reference_pitch`／`batting_staging` 全保留通過；S3 **20 replays、30/60/120 chunking**、pause／step／Complete、prediction／arrival 不變。Release tick384、gameplay complete479、presentation complete816；grip handoff error `4.9151248e-7 m`，prediction delta=0。
- 實際 Debug／Release app 均逐 tick 走過 0–816、release 邊界、pause pixel stability、Complete hold、正常時間 full pitch、replay、minimize／restore 不加背景時間與正常 shutdown（exit0）。完整 logs／smoke JSON 留在 evidence；正常 delivery 約 3.434／3.420 s，非 timing 規格變更。
- Debug D3D12 debug layer＋GPU-based validation：**0 errors／corruption**；owned resources 釋放後只剩 report 用 device，沒有 live child resources。Release 無 debug layer，不冒稱執行 GPU validation。
- Computer Use helper 初始化兩次因 sandbox setup 錯誤失敗；改用既有 S3 app smoke 腳本與 Win32 截圖進行實際 app 驗證。不是只憑 unit tests 推定畫面。Debug／Release startup client pixels 比對完全一致。

以上為 S1 實作與驗證紀錄；S1 已 human accepted。S2 現況見下節，collision／contact presentation 與 M1 未完成。


## Batter Runtime S2 — synchronized swing playback candidate

**S2 已獲 Michael＋Julia human review accepted。** 以下保留 S2 交付量測與當時 evidence 限制。 這是 accepted authoring choreography 的同步 development preview；不是「玩家按鍵就從 delivery tick 0 起播 swing_L」的正式 input／commit 規則。`contact_area` 仍不是 collision truth。不修改 source、球路、camera、staging 或六條 Character Motion Rules。

### 操作與 review evidence

啟動 `build/release/pawapuro.exe`（或 Debug）；Space 開始／雙方 Complete 後 replay，P 同步 pause／resume，`.` 在 paused 時只前進一個 240 Hz tick，Esc 離開。Minimize 自動 pause，restore 不補背景時間。Startup 為兩角色各自 clip tick 0；S1 static bind batter 已自 immutable scene 移除，沒有雙重打者或 fallback。

Ignored `build/batter-runtime-s2/`：

- **`synchronized-runtime-1x.mp4`**：完整 batting view、雙方 Ready 到 finish。
- `batter-runtime-crop-1x.mp4`：同一 runtime 畫面固定像素裁切，沒有改 camera；可對照既有 `build/batter-authoring-s0/swing-batting-1x.mp4`。
- `synchronized-contact-sheet.png`：Ready、Gather、lead lift、pitch release、Plant、arrival、contact_area、follow-through、pitcher finish、batter finish，標 tick／time／authoring frame。
- `contact-every-tick.png`：ticks 476–484；`contact-every-four-ticks.png`：ticks 464–496。原始 PNG 位於 `release-contact/`。
- `debug-smoke.json`／`release-smoke.json`、app logs、build／CTest logs、`capture-provenance.json`／`video-validation.json`。

影片來自實際 Release app 的 authoritative tick 截圖（0–896，每 4 ticks 一格），編碼／decode 核對 60 fps、225 frames、1×，不是 wall-clock 螢幕錄影。Clip time 為 3.733333 s，最後一格顯示 1/60 s，影片長 3.75 s。另獨立執行正常 wall-time playback 驗證。已實際檢視全段主要 runtime poses 與 contact 連續影格；未宣稱用播放器完成正常速度視覺自看。正常速度 readability／接觸時序是否可玩仍由 Michael＋Julia review。

### Concrete owners／資料流與 lifetime

- `BattingPreview` 在 Pawapuro 層擁有唯一 preview wall-time accumulator、`PitchDelivery`、`BatterMotion`、Ready／Playing／Complete、pause／step／replay；每個 240 Hz tick 直接驅動兩者。子物件不 consume wall time，也沒有獨立 preview controls。超額 debt 依既有每 frame 最多 16 ticks 規則保留，Complete／replay 清除。
- `PitchDelivery::step_tick()` 公開為 guarded deterministic operation；原 `advance(elapsed_ns)` 保留給 standalone caller／tests。既有 release-once、prediction／arrival、raw Complete、tick 816 completion 不改；外層 preview 不延長它的 clip 或狀態。
- `BatterMotion` owns accepted GLB、共享 pose、兩個預配置 skin arrays、14304 個展開 triangle vertices 與 bat semantic world transforms。每 tick pose evaluate 一次，`BatterMesh`／`Bat` 都用既有 Engine CPU skinning；只做一次 X reflection／winding reversal，加 staging-only placement，scale=1。Semantic matrices 同 basis。沒有 equipment renderer。
- TOML 只讀 hash／clip／time base／non-loop／contact_area／既有 bat semantic 名稱。Gather／Plant 從現有 `review.key_poses` 的兩個命名 pose 讀作 diagnostics，不建 event DSL，也不控制播放。CMake 同步複製既有 TOML 至 executable 旁；未重匯出 metadata。
- App 的 `dynamic_characters` 一次 reserve 26112 vertices（pitcher 11808＋batter 14304），每 render clear／append 兩者，再一次送入原 dynamic stream。**Engine loader／pose／skinning、renderer／HLSL 均未改**；無第二 buffer、interpolation 或 runtime speed multiplier。
- CPU assets、pose、skin／flatten buffers 由 preview 持有到 app 結束；合併 vector 由 app 持有。Renderer 只在 draw 的同步 upload 期間借用，GPU retirement 沿用既有 fence。檔案／TOML parse 暫存不流入 draw。

### 實測共同 timeline

| Event | Preview tick | 秒 |
|---|---:|---:|
| Batter Gather（TOML f72） | 284 | 1.183333 |
| Lead lift（source f88） | 348 | 1.450000 |
| Pitch release | 384 | 1.600000 |
| Batter Plant（TOML f107） | 424 | 1.766667 |
| Pitch gameplay Complete／arrival 可取得 | 479 | 1.995833 |
| Batter contact_area（TOML f121） | 480 | 2.000000 |
| PitchDelivery／pitcher Complete | 816 | 3.400000 |
| Batter／BattingPreview Complete（clip f225） | 896 | 3.733333 |

`contact_area − arrival = +1 tick = +0.004166667 s`。這只是兩份既有資料在同 timeline 的關係；不改 authoring timing，不生成 hit／miss。479 後球不再 integration，816 後 pitcher 保持 final，batter 繼續 finish；兩者完成前 Space 不 replay。

以下是 actual authoritative game-world positions，單位 m。球的 479／480 值是 raw Complete，不是吸附到 plane 的 interpolated arrival sample：

| Tick | Ball | bat_grip | bat_barrel | bat_tip |
|---:|---|---|---|---|
| 478 | (-0.001931, 0.781045, 0.480416) | (0.825284, 0.831275, 0.429135) | (0.000912, 0.805920, 0.185479) | (-0.325002, 0.795895, 0.089150) |
| 479 | (0.004963, 0.759459, 0.306804) | (0.837307, 0.829623, 0.444869) | (-0.011376, 0.799802, 0.309040) | (-0.346901, 0.788012, 0.255340) |
| 480 | (0.004963, 0.759459, 0.306804) | (0.850000, 0.828001, 0.460000) | (-0.008927, 0.793644, 0.434232) | (-0.348503, 0.780060, 0.424045) |

原 plane arrival 約 `(-0.00000045, 0.77500063, 0.43180001)`；pitch-relative interpolated time 0.392833450 s。與第 95 個 physics tick 的 raw state 不混用。Logs 記錄 marker ±4 ticks 完整精度。

### 驗證與成本

- Debug／Release build 成功、各 **8/8 CTest**。原 static multi-mesh／pitcher motion／S3 delivery／reference physics／prediction／staging tests 全保留；不放寬原容差。
- 新 `batter_motion_test`：兩 meshes／17 joints、225 samples、metadata hash／fps／marker／loop failure cases；全部 897 ticks finite skinning、fixed vertex counts／colors、rigid bat attachment、steady storage、20 次 identical initial／final。
- `write_motion_fixture.py` 只讀正式 saved `.blend` evaluated animation，不 save／export。`motion_expected.txt` 記錄 source／GLB hashes，17 poses、816 surface points、每 mesh bounds、grip／barrel／tip；runtime 最大 error **1.2157e-6 m**，容差 **1e-4 m**。這是有限幾何抽樣，加上全 ticks 的 attachment／finite checks，不是完整 surface collision 檢查。
- 新 `batting_preview_test`：30／60／120 Hz elapsed chunking 得到相同 tick、兩份 pose 與球 state；20 replays，fractional／catch-up debt、pause／step、release 一次、479 result 即時可得、816／896 完成邊界、原 raw Complete 與 prediction bit-exact。Release grip alignment 仍為 **4.9151248e-7 m**。
- 實際 Debug／Release app：逐 tick 0–896，同步 start／pause／step、479／816 replay guard、Complete hold、正常 wall-time playback（約 3.778／3.764 s）、replay、minimize／restore 不加背景時間、exit0。Debug GPU-based validation **0 errors／corruption**，shutdown 後無 live child resources；Release 無 debug layer，不冒稱 GPU validation。
- 使用既有 Win32 app smoke harness／PrintWindow；前輪 Computer Use helper 的 sandbox 初始化失敗不當成成功。影片與逐格不是 Blender render。

同一次 app smoke 的 CPU mean（µs）；skin／pose 為 evaluation sample，assembly／upload 為 rendered frame，包含 pause 期間 frames，並非效能目標或 GPU time：

| Stage | Debug | Release |
|---|---:|---:|
| Batter pose | 25.894 | 3.521 |
| BatterMesh skin | 344.644 | 17.802 |
| Bat skin | 16.509 | 1.030 |
| Combined dynamic assembly | 21.147 | 23.858 |
| Existing dynamic upload memcpy | 50.413 | 36.026 |

兩 build 均 2691 batter evaluations；assembly／upload samples 為 1587／2789，合併 capacity／size 始終 26112。Release assembly mean 不比 Debug 小，這是該次 app capture／render sample 結果，不據此做 optimization 或效能外推。

### 限制／review gate

Runtime contact 連續影格可見短促 bat sweep；follow-through 中手棒會被大頭遮住，torso 的簡化輪廓仍淡。球於 arrival 停住、棒繼續經過它是保留既有 raw Complete 的 preview 語意，不是碰撞回應。數值對齊不能宣告 late gather、hands／bat lag、雙角色 readability 或 contact timing 已通過 human review。

Accepted batter `.blend`／GLB／TOML、pitcher assets、staging／camera 完全未改；S1 acceptance 保留。S2 現已 human accepted；其 acceptance 不定義 collision、正式 swing input／commit、hit quality 或 contact presentation。Bat Contact S0 現況見下節；六條 Motion Rules 不變，M1 未完成。


## Bat Contact S0 — continuous closest-approach probe

S0 probe 已獲 Michael＋Julia acceptance。以下是該輪 **geometry study** 紀錄；只量 ball center 到 `[bat_barrel, bat_tip]` semantic centerline segment 的距離。`contact_area` 仍是 authoring marker，不是 collision truth。沒有 ball／bat radius、sweet spot、hit／miss、penetration 或 response，沒有使用 enlarged visual ball marker 或 rendered mesh thickness 作 collision envelope；player swing input 尚未設計。

### 查詢與重現

- Pawapuro `sample_reference_pitch(initial,t)` 使用既有 gravity 與 authoritative initial state 的 constant-acceleration equation，時間是 release-relative；只讀取初始值，不依 frozen Complete state，不前進 tick／phase。可在 plane 後解析取樣，但不延伸 mutable simulation。與 float integration 有微小捨入差，不宣稱 bit-identical。
- `BatterMotion::sample_barrel(clip_time,scratch)` 只在 caller-owned `GlbPose` 評估既有 LINEAR clip，傳回同 basis／staging placement 的 barrel／tip world points；不 skin、不改 authoritative tick、pose、triangles 或 perf samples。Scratch 只活在 study 呼叫中，沒有 global cache。
- `contact_probe.*` 是 concrete Pawapuro geometry study；沒有 Engine／generic collision framework。由獨立 `contact_probe_test` 執行並可寫 JSON；app main／renderer 完全未改，也不自動加入 per-tick probe 或 hit log。
- 搜尋 marker ±8 ticks，即 **[472,488]**／[1.966667,2.033333] s，包含原 plane arrival。先每 tick 16 substeps，取 coarse 最佳點相鄰的兩格，再固定 32／64／128 subdivisions。各層都保留已找到的最小值；本版 minimum 在 window 內部，無須擴窗。這是有收斂證據的 bounded numerical minimum，不是已證明適用任意 clip 的 CCD solver。

Repo root 重跑（既有 Debug／Release build）：

```powershell
& build/release/contact_probe_test.exe pawapuro/batting pawapuro/batting/staging.toml build/bat-contact-s0/diagnostic.json
```

Ignored `build/bat-contact-s0/`：先看 **`closest-approach.png`**。它使用 production `project_batting_point` 的 accepted 1920×1080 batting camera；cross／segment／Q／最短線為 fractional-time sample 的離線投影，背景明確標為 actual runtime tick479，不是 sub-tick runtime render。下方為本輪 Debug app 實際 ticks477–481，原 PNG 在 `debug-contact/`。圖中的既有大球／prediction 圈只是 visual markers，不是半徑判定。另有 Debug／Release diagnostic JSON、build／CTest／app logs 與 smoke JSON。No new runtime overlay。

### Bounded minimum 與時間基準

| 項目 | 實測 |
|---|---:|
| Centerline separation | **0.043298393 m（約 43.30 mm）** |
| Absolute preview time | **1.995764160 s** |
| Fractional preview tick | **478.983398438** |
| Segment u | **0：barrel endpoint；Q=A** |
| 相對 plane arrival（1.992833450 s／tick478.280028） | **+2.930710 ms** |
| 相對 gameplay Complete tick479（1.995833333 s） | **−0.069173 ms** |
| 相對 contact_area tick480（2.0 s） | **−4.235840 ms** |

Plane arrival 使用原 `PitchArrival.time_s`（最後一個 physics interval 的內插 sample），gameplay result 仍在 tick479 才可取得。本次不改 prediction／arrival semantics。Probe window 的後段繼續用解析 ball path，沒有拿 tick480 frozen ball 與動棒宣告距離或 miss；本次 minimum 本身略早於 raw Complete tick。

Minimum 的 game-world vectors，position 為 m，velocity 為 m/s：

| 項目 | (X,Y,Z) |
|---|---|
| Ball C | (0.004848963, 0.759819031, 0.309694052) |
| Ball velocity | (1.654644370, −5.200534344, −41.666999817) |
| Barrel A／closest Q | (−0.011291265, 0.799904287, 0.306970805) |
| Tip B | (−0.346702099, 0.788144112, 0.252554268) |
| Bat point velocity | (−1.250505447, −1.477599144, 29.923170090) |
| Relative velocity：ball − bat | (2.905149937, −3.722935200, −71.590171814) |

Velocity 用 minimum 的 **固定 u material point**，以 ±0.1 ms central difference 估算；不是任由 u 滑動的最近點 locus derivative。Ball speed **42.022878 m/s**、bat point speed **29.985716 m/s**、relative speed **71.745751 m/s**，僅作 diagnostic，不推算 exit velocity／response。

| 相對 minimum | −2 ms | −1 ms | minimum | +1 ms | +2 ms |
|---|---:|---:|---:|---:|---:|
| Separation（mm） | 149.530342 | 83.666817 | 43.298393 | 83.751207 | 149.798993 |

### 收斂與回歸

| Refinement subdivisions（coarse 16/tick 相同） | 最小 tick | Separation（mm） |
|---|---:|---:|
| 32 | 478.984375000 | 43.299399 |
| 64 | 478.982421875 | 43.298988 |
| 128 | 478.983398438 | 43.298393 |

三層距離 spread 約 **0.001006 mm**、時間 spread **8.138 µs**；nested distances 逐次下降，minimum time 在小區間內擺動。128 的 refinement spacing 約 4.069 µs；GLB evaluator 仍使用 float time／transforms，不把表格的小數位當成物理精度。收斂 assert：32→128 距離差 <1e-5 m、時間差 <2e-5 s；沒有改既有容差。

- Debug／Release build，**各 9/9 CTest 通過**；Debug／Release 完整 probe JSON 相同。
- Ball query 與 t0 至 arrival 的全部 integer ticks：最大 position error **8.1093e-6 m**，velocity error **6.1989e-6 m/s**；沿用 reference integration 的 0.0002 tolerance，另核對 plane sample 與 Complete 不變。
- Bat query 在 tick0／Gather／Plant／479／contact_area／final 與 runtime semantics 最大差 **0 m**，沿用 1e-4 m tolerance。Probe 前後 authoritative tick、pose、triangles、sample count 不變。
- Closest point 覆蓋 interior、barrel endpoint、tip endpoint、degenerate segment；repeat exact，30／60／120 render chunking 完成後的 probe 結果 exact 相同。既有 S2／S3 20 replays、pause／step／debt、release ownership、arrival／prediction、asset／animation tests 全保留。
- 本輪重跑一次 actual Debug app smoke：0–896 逐 tick、start／pause／single-step／replay／Complete／minimize restore、正常 wall-time playback（約 3.767 s）；Debug GPU validation **0 errors／corruption、無 live child resources、exit0**。新 actual ticks477–481 已檢視。App pixels 與 S2 同 ticks 比對一致（見本輪 evidence 檢查）。

正式 assets、motion、staging、main／renderer／HLSL 未改。這個結果只描述既有 motion／pitch 的時空距離；S0 當時未決定 collision envelope、hit rule、player input 或 response。S1 的第一 envelope／contact event candidate 見下節；S0 數值及原有 motion 未改。


## Bat Contact S1 — first continuous contact event candidate

**Bat Contact S1 已獲 Michael＋Julia acceptance。** 從 accepted S0 query 接入一次性的 authoritative sphere–capsule contact detection。只記錄第一次進入 envelope，不改 ball trajectory／ownership、animation、camera、arrival 或完成時間。Sweet spot、response、exit velocity／spin／hit quality 與 player input 均未開始；不增加 Motion Rule。

### Gameplay Data／owner

`staging.toml` 新增 `[bat_contact] ball_radius_m=0.037`、`bat_radius_m=0.033`，總和 0.070 m。**Reality is a consultant**：37／33 mm 是可調的第一 gameplay candidate，不是永久常數。不讀 `release.ball_marker_radius_m=0.085` 或 rendered bat thickness；原 staging geometry 所有欄位不變。沿用 startup Data parse／missing defaults／unknown-key rejection；個別 radius 必須有限且在 [0.001,0.2] m，這只是輸入 guard，不是可玩區間承諾。

`BattingPreview` owns immutable envelope snapshot 與 `optional<BatContact>`。每次 start／replay 清成 NoContact；每個 authoritative tick 完成原 delivery／batter update 後，搜尋該 tick interval 的 earliest entry，找到後存成 Contact 並 log 一次。後續 tick 不再搜尋／trigger，保留結果到 replay。沒有 event bus／callback framework，也不讓事件控制任何 simulation owner。

沿用當前 accepted choreography 的 marker ±8 ticks（472–488）作 bounded broad phase；marker 本身不 trigger，事件由 geometry 求得。每個 interval 固定 **64 substeps** 找第一個 outside→inside bracket，再 **24 次 bisection** 保留 inside 端。球由 immutable initial state 解析取樣，棒由 scratch pose 取 barrel→tip，endpoint hemispheres 合法；沒有 grip→tip 或 triangle collision。Scratch 為 query-local CPU storage，event 是 owned value，不保留 pose 指標或 GPU resource。

此法經本版 32／64／128 對照驗證，**不是對任意新 motion／極窄 grazing contact 的通用 CCD 保證**；若未來 envelope／clip／球路改動，需重新檢查 bounded window 與 subdivision。初始中心線完全重合時 normal 未定義會明確報錯，未預建 overlap-response policy。Window 的解析查詢不延伸 mutable Complete ball。

### First contact 實測

| 項目 | 值 |
|---|---:|
| Absolute preview time | **1.994995922549 s** |
| Fractional preview tick | **478.799021412** |
| 記錄／dispatch tick | **479**，完成 interval [478,479] 後 |
| Centerline separation | **0.069999976425 m**（約 70 mm） |
| u at first contact | **0，barrel endpoint hemisphere** |
| u at S0 minimum | **0**，不因此移動 semantic nodes 或定義 sweet spot |
| contact_area time／delta | 2.000000000 s／**−5.004077451 ms** |
| Envelope exit（test-only diagnostic） | 1.996531412704 s |
| 連續 overlap 時窗 | **1.535490157 ms**，小於 240 Hz tick 的 4.166667 ms |

本版整數 tick479 恰好落在 envelope 內，但不能以它代替 first entry；只檢查 tick samples 會丟失 sub-tick entry，亦不能保證其他相位的短窗不被跳過。`contact_area` tick480 不參與 contact 判定。Ball 仍在479 gameplay Complete、pitcher816、batter／preview896完成。

Normal 是 **closest capsule axis point Q → ball center C**。以下為 game-world，positions m、velocities m/s：

| 項目 | (X,Y,Z) |
|---|---|
| Ball C | (0.003577803, 0.763811409, 0.341704190) |
| Barrel／closest Q | (−0.010080814, 0.801038325, 0.284018874) |
| Tip | (−0.344117284, 0.789606929, 0.221658334) |
| Capsule surface point Q + normal × Rbat | (−0.003641749, 0.783488512, 0.311213374) |
| Normal | (0.195123181, −0.531813264, 0.824076235) |
| Ball velocity | (1.654644370, −5.193000793, −41.666999817) |
| Bat surface-point velocity | (−1.039765835, −1.456141472, 29.649436951) |
| Relative velocity：ball − bat | (2.694410324, −3.736859322, −71.316436768) |

速率為 ball **42.021946 m/s**、bat **29.703376 m/s**、relative **71.465083 m/s**。與 S0 不同，這裡記錄的是 **capsule surface material point**：在 entry 時將 Q+nRbat 轉入 bat semantic 的 rigid local frame，再以 ±0.1 ms 取同一 material point 的 central difference；不是滑動 Q 或 world normal 的 derivative。這些值只記錄，不推算 response。

### 收斂／驗證與 evidence

| Substeps per tick + 24 bisections | First time（s） | Separation（m） |
|---|---:|---:|
| 32 | 1.99499592254870 | 0.0699999764247533 |
| 64（runtime） | 1.99499592254870 | 0.0699999764247533 |
| 128 | 1.99499592254676 | 0.0699999764247533 |

時間 spread 約 1.94e-12 s，sampled geometry 相同；這只描述 deterministic bracket/refinement 收斂。GLB evaluator 仍為 float time／transforms（此時間區域 float spacing 約 0.119 µs），不可把列印小數當成物理精度。

- Debug／Release build、各 **10/10 CTest**；兩版 contact diagnostics 相同。原 S0、S2／S3、asset／animation／arrival／prediction tests 全保留，未放寬 tolerance。
- 新測試：現行 first contact、entry 前後 ±1 µs outside／inside、normal unit、32／64／128 收斂、single-step crossing／pause、one-shot、20 replays、30／60／120 chunking exact invariant；20+20 mm artificial envelope 為 NoContact，40+40 mm earlier contact，visual marker 改0.15 m 不影響結果。
- 每 tick 對照不含 contact owner 的 PitchDelivery／BatterMotion：球位置／速度與 ownership、兩份 pose、479／816／896完成時間不變。新 staging tests 覆蓋 radius overrides、visual separation、zero／negative／NaN／wrong type／unknown key。
- Actual Debug app smoke 通過 start／pause／step／replay／Complete／minimize restore；三次 preview 各只有一個相同 BatContact log。GPU validation **0 errors／corruption、無 live child resources、exit0**。Ticks477–481 與 S0 畫面逐 pixel 相同；正式投打三檔 unchanged，main／renderer／HLSL 未改。
- Ignored `build/bat-contact-s1/` 保存 `contact-release.log`、Debug／Release CTest／build logs、`debug.log`、`debug-smoke.json`、actual `debug-contact/0477..0481.png`、`regression-evidence.json`。不重製影片或新增 runtime overlay。

Repo root 重現數值：`build/release/bat_contact_test.exe pawapuro/batting pawapuro/batting/staging.toml`。App 原 Space／P／`.` controls 不變；首次 contact 僅輸出 `BatContact:` 一行（time／fractional tick／u／normal／velocities），event 可直接由 Native owner 讀取。完成後停止，等待 review；不做 contact response。


## Bat Contact S2 — timing-to-contact geometry map（2026-09-17）

**Read-only study 已完成，待 Michael＋Julia review。** S1 已接受的 truth 是現行 synchronization 產生一次 continuous contact：first time 約1.994995922549 s／tick478.799、70 mm envelope、約1.535 ms overlap，事件不改球。Sweet spot、hit quality、response、player timing window 仍未定；本節不授權它們。

### 範圍／取樣與重現

同一份 pitch initial state、正式 swing_L、37／33 mm envelope；只在 non-mutating query 使用 `batter_sample_time = preview_time + diagnostic_phase_offset_s`。正值表示同一 pitch time 下打者 phase 超前，負值落後，**不是 player input delay**。正式 BattingPreview 不傳此 study 參數，維持 offset=0；沒有快捷鍵、Data override 或 runtime retiming。

Reuse S1 的 first-entry detection、surface material-point velocity、64 substeps＋24 bisections；S0 minimum query 同樣只偏移 bat sample。Ball absolute time／release-relative time不變，first-contact time 不直接加減 offset，而由相遇幾何求得。Surface velocity 的 ±0.1 ms samples 也套相同 phase，避免取錯 motion phase。`normal closing speed = −dot(ball_velocity − bat_surface_velocity, normal)`，正值為靠近；不是 relative speed 的長度，也不是 exit velocity。

離線 `contact_phase_test` 先檢查 offset0 的 S1 time／u／normal，並與正式 runtime event 全欄位 exact 比對，成功才 sweep。第一輪 **−40..+40 ms、2 ms、41 samples**；+40仍 Contact，因此依授權擴為 **−60..+60 ms、2 ms、61 samples**。+60仍 Contact，**較超前側的外緣尚未找到；不擴到更大範圍**。每個 offset 的時間搜尋仍為 preview ticks472–488，各 minimum 均非時間窗邊界。

只對粗掃描發現 normal.y 轉正的 **+16..+26 ms** 做一次 **0.5 ms／21 samples** refinement；沒有 optimizer、第二個局部搜尋或挑選正式 timing。

Repo root 重現（使用既有 build；不需要 Blender 或新 dependency）：

```powershell
& build/release/contact_phase_test.exe pawapuro/batting pawapuro/batting/staging.toml build/bat-contact-s2 16 26
```

### Map 結果與邊界

- **已取樣 Contact：−8..+60 ms**；已取樣 NoContact：−60..−10 ms。落後側的 transition 只定位在 **(−10,−8] ms**：−10的 minimum為70.335663 mm，−8為64.655560 mm。沒有另做邊界 optimizer，不能把−8當精確 threshold。超前側的 NoContact boundary 未知；+60的 minimum仍為55.953163 mm。
- First-contact u 在−8..+10的2 ms samples都是0；**+12..+52為interior**，由約0.0094移向0.9282；+54..+60為tip endpoint u=1。這是接觸位置，**不是 sweet-spot score**。
- normal.y 隨超前 phase由負轉正；coarse +18為−0.002623、+20為+0.029230。唯一 local refinement 將零點夾在 **+18.0（−0.002623）與+18.5（+0.005302）ms**，這時u已約0.087–0.094。
- 因而 coarse **+20..+52 ms**，加上 local **+18.5..+26 ms** 的實測點，同時有 **0<u<1、normal.y>0**。可供 review 的小區段如+20..+26：normal closing約64.47–64.78 m/s。只可稱有用的 upward geometry candidate，不推算 launch、飛行距離或 Perfect Hit。
- 全部 Contact samples：relative speed **64.168561–71.764794 m/s**，normal closing **27.285285–69.543583 m/s**。兩者不同；offset0 closing為56.257127 m/s。+8→+10附近的速度變化保留為既有 LINEAR motion取樣結果，沒有平滑或改asset。

| 代表 phase offset | 狀態／first time（s） | u | normal.y | relative／closing（m/s） | Minimum（mm） |
|---:|---|---:|---:|---:|---:|
| +54 ms | Contact／1.973412534 | 1.000000 | +0.331264 | 68.591／64.342 | 13.844 |
| +24 ms | Contact／1.985739261 | 0.183617 | +0.094278 | 64.989／64.779 | 0.686 |
| 0 ms | Contact／1.994995923 | 0 | −0.531813 | 71.465／56.257 | 43.298 |
| −8 ms | Contact／1.998691568 | 0 | −0.896869 | 71.149／27.285 | 64.656 |
| −10 ms | NoContact | 不適用 | 不適用 | 不適用 | 70.336 |

Offset0仍 exact 重現 S1：u=0、normal `(0.195123181,−0.531813264,0.824076235)`。+24的0.686 mm是 **後續 minimum centerline separation**，不是first-contact separation或穿透深度；所有contact仍以70 mm envelope首次entry求得。

和S0／S1一樣，本圖使用解析球路取樣，不用 frozen Complete位置。部分落後phase的幾何contact時間在原gameplay Complete之後（例如−8 ms）；這不會讓正式app的球恢復積分，也不等於已設計新的delivery completion／input rule。

### Evidence／驗證

Ignored `build/bat-contact-s2/`：

- **`phase-map.png`**：offset對u、normal.y、first-contact time、minimum separation、relative／closing speed，以及唯一local refinement。圖只在Contact點畫event量；NoContact仍有minimum。
- **`representative-cases.png`**：上述五例，accepted batting-camera projection＋共同zoom，標C、barrel→tip、Q、normal與u。它們是離線geometry diagrams，沒有假造retimed runtime screenshots；NoContact只畫minimum，不填入不存在的contact normal。
- `coarse-41.csv`、`phase-map.csv`（61）、`local-refinement.csv`（21），完整ball／bat／relative velocity vectors、normal、minimum與screen coordinates。NoContact的event欄位留空。
- `summary.json`、build／CTest／phase logs。Plot使用既有Windows Charting library，沒有安裝matplotlib或其他dependency。

Debug／Release build、各 **11/11 CTest** 通過。新study test涵蓋：offset0 exact S1／runtime、正負sign、repeat map deterministic、全部61點的32／64／128 Contact／NoContact分類與entry收斂、30／60／120 chunking、20次replay之後完整map相同；study前後preview／delivery ticks、pause／debt、raw ball state、兩份pose、batter triangles／sample count與stored event均未改。既有S0／S1、S2／S3、assets／arrival／prediction tests保留。NoContact邊界穩定指已取樣分類穩定，不宣稱找到sub-ms threshold。

本輪沒有重跑app GPU smoke：main／renderer／HLSL／BattingPreview controls／正式asset／staging未改；上輪S1 GPU驗證紀錄保留，不能冒稱是S2新驗證。新的pure query參數只由offline caller提供，正式runtime offset0由direct tests與20 replay確認。完成後停止，待Michael＋Julia review；不做response、sweet spot、hit quality或正式player input。


2026-09-17 review 更新：Michael＋Julia 已接受 Bat Contact S2 timing-to-contact geometry study；上述交付時的待 review 狀態已解除。新的 [Hit Authorization S0](../../../docs/design/batting-contact.md) 僅為 design candidate，不改本節 physical truth 或 runtime。
