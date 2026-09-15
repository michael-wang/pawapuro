# 右投手 Authoring Sample（正式 Style Feet v1B；motion S0.2C）

2026-09-16：Michael＋Julia 已接受 S0.2C（A／B／C）authoring motion、S1 static import，以及 Character Motion Rules v0.1 design。S2 runtime animation／CPU skinning 已通過 Michael＋Julia 技術與 human review。v1A 的 1.25× footprint 與 v1B shoe-like geometry 均已通過 human review，正式三檔已 promotion 至 v1B；plant orientation／material／articulation／rubber-arm debts 仍保留。球仍是獨立 reference fixture，S3 未開始，M1 未完成。

## Style Feet v1B promotion（2026-09-16）

Michael＋Julia 已接受 v1B 的 toe／heel／upper／sole geometry，及保留的 v1A 1.25× footprint 尺寸。`review/style-feet-v1b/` 三檔已 byte-preserving promotion 為正式 asset，歷史 review artifact 保留；未重新保存 `.blend`、重跑 revision 或改 motion／metadata。正式 fixture 採用候選已驗證數值，只更新 acceptance 註解。

Front-foot plant orientation 尚待量測／修正，geometry acceptance 不代表這項 motion style 已通過。Material／highlight、cleats、foot articulation、rubber arm 仍延後，S3 未開始。Promotion validator、source／GLB／round-trip 與 Debug／Release S2 motion regression 通過，詳見 environment 的 v1B promotion 紀錄。

以下 v1B candidate 段落保存當時交付與觀察；其中「待 review／未 promotion」為當時狀態，現在以本節 acceptance 為準。歷史 `revise_shoe_shape.py`／`check_shoe_shape_export.py` 以 promotion 前 v1A 為比較基準，不應對目前正式 v1B 重跑 revision。

## Style Polish v1B：Shoe-like Foot Shape candidate

2026-09-16：`review/style-feet-v1b/` 是從正式 v1A（`b102e02e`）製作的 **待 Michael＋Julia human review candidate**，沒有 promotion。正式三檔及正式 fixture 不變；本輪只改 shoe geometry，未新增 Motion Truth，S3 未開始。

`revise_shoe_shape.py` 只讀目前已存檔 v1A，核對 source hash、拒絕覆寫 candidate。沿用每腳 220 vertices／原 triangles 與 rigid weights；實際改 438 個 POSITION，1922 個非腳部 vertices exact。既有 latitude rings 改成平底、較直且收束的 heel、rounded toe 與偏中後段較高的 upper；前端有小幅 rigid toe spring，以保留原 toe-down lift 的離地間隙。不是 articulation。沒有改 color、material、mesh topology、骨架、rest、IBMs、keys、grip、camera 或 staging。

Foot rest basis 為 identity；Blender +Y 是鞋頭，經既有轉換成 GLB／game −Z（本壘）。兩脚 before／after 的 rest width × length × height 均約 **0.989187 × 1.163750 × 0.198450 m**；最大 bounds 差 2.98e-8 m（float rounding），完全在 accepted envelope 內。Planted world bottom Y 仍為 0.355 m、差值 0；全 205 格離地最小間隙為右腳 f86 的 0.000852719 m（v1A 0.001549684 m），未穿地。形狀改變使 airborne bottom 最大差 0.0361957 m；沒有改路徑補償。Landing 後 slide 仍為 0。

Michael 優先看 ignored `build/style-feet-v1b/`：

- `v1a-vs-v1b-runtime.jpg`：六個同 camera／tick 的正式 v1A（左）與 candidate（右）實際 app raster。
- `candidate-runtime-1x.mp4`：完整 runtime tick 0–816，每 4 ticks 擷取一格，再以 60 fps 重組 1×；205 顯示 frames／3.416667 s，clip 仍為 3.4 s。不是 wall-clock 螢幕錄影。
- `lifted-foot-closeup.jpg`：f49 既有 side camera 與實際 batting view 的乾淨 before／after；`grounded-feet-closeup.jpg` 看 Ready／front contact／rear landing。
- `shoe-local-direction.jpg`：實際 saved mesh 的 top／side／斜側面診斷；wire 僅在診斷圖，不是新增材質。`candidate-motion-overview.jpg` 是整段抽樣。
- `candidate-release/pawapuro.exe`：相同 S2 executable／DLL／staging 的隔離 copy，只替換該 copy 的 GLB／TOML。Space 播放、P 暫停、`.` 單步，正常 build 仍載入正式 v1A。

已檢視乾淨 side／runtime 對照、lifted pose、接地與跨全段抽樣。平底和較直後跟可見，但低厚度、單一深色與 batting camera 距離仍讓 upper／sole 不易分開；Ready／landing 的雙鞋仍容易合成一片，f65 腳與衣襬仍有投影重疊。未發現抽樣畫面中的嚴重穿地，並非全身 collision 保證；是否一眼像球鞋與是否更笨重仍待 human review。**未完成 Codex 正常速度連續觀看**；已實測 app 正常速度執行完成，兩者分開記錄。

日常 export 只匯出已存檔 candidate，不呼叫 revision script：

```powershell
$Blender = 'C:/astra-dev/tools/blender-4.5.13-windows-x64/blender.exe'
$Pitcher = 'C:/astra-dev/pawapuro/pawapuro/batting/pitcher'
$Asset = "$Pitcher/review/style-feet-v1b"
$Evidence = 'C:/astra-dev/pawapuro/build/style-feet-v1b'
& $Blender --background "$Asset/pitcher.blend" --python-exit-code 1 --python "$Pitcher/export_sample.py" -- --evidence $Evidence
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/verify_sample.py" -- --asset-dir $Asset --evidence $Evidence
& $Blender --background "$Asset/pitcher.blend" --python-exit-code 1 --python "$Pitcher/inspect_motion.py" -- --output "$Evidence/motion-audit.json"
& ./build/debug/pitcher_motion_test.exe "$Asset/pitcher.glb" "$Asset/pitcher.toml" ./pawapuro/batting/staging.toml "$Asset/motion_expected.txt"
```

`check_shoe_shape_export.py` 比較所有 GLB accessor bytes／metadata，並核對小型 candidate fixture；需要 evidence 的 source comparison、正式與候選 evaluated samples，預設唯讀 fixture，只有 `--write-fixture` 才更新候選 fixture。本次只改五格受 foot 影響的 bounds、16 個 foot sample points 及 hashes；grip／非腳部點原文不變。既有 validator／source／round-trip／motion assertions 與 tolerances 均保留，細節由 [environment](../../../docs/development/environment.md#style-polish-v1b-shoe-like-foot-shape2026-09-16) 維護。

Material／highlight、articulation／sole／cleat detail 與 rubber arm 仍是獨立 debt。v1B 尚未接受，不自行 promotion。

## Style Polish v1A：1.25× planar feet candidate

只改 `review/style-feet-v1a/pitcher.blend` 的 440 個 rigid foot vertices：Blender local X／Y（game X／Z）繞各 foot rest anchor 放大 1.25×，local Z／厚度不變。實際 398 個位置改變，中心軸 42 個不動；1922 個非腳部 vertices、全 weights／topology／colors／骨架／全部 animation channels／grip／marker 完全保留。現已將此接受版本 byte-preserving promotion 到正式本目錄三檔，rubber arm 未改；`review/style-feet-v1a/` 保留歷史 review artifact。

| 尺寸，m | S2 baseline | v1A candidate |
|---|---:|---:|
| Rest 平面寬 | 0.791350 | 0.989187 |
| Rest 平面長 | 0.931000 | 1.163750 |
| 厚度 | 0.198450 | 0.198450 |
| Planted world bottom Y | 0.355000 | 0.355000 |

Ready 的右腳 world X／Z extent 即上表寬／長；左腳朝向不同，world extent 為長／寬。Frame 171 後腳 world X／Z extent 從 (0.802763,0.918538) 變為 (1.003454,1.148172)。完整六個 review poses 的 bounds 在 `build/style-feet-v1a/foot-world-bounds.json`。**Planted 接地高度不變；airborne 右腳有傾斜，放大平面會改變 world-space 最低點，最大 0.0907805 m（frame 127），不代表骨骼／路徑改動。** 全 205 格未穿過支撐平面；沒有修改腳姿勢補償形狀。

本機 review 優先入口（全部位於忽略的 `build/style-feet-v1a/`）：

- `baseline-vs-125-runtime.jpg`：六個 poses、同 camera／1920×1080 raster、每組左右相同 crop／倍率。上方為固定角色 crop 1×，下方腳部 crop 1.4×。
- `candidate-runtime-1x.mp4`：實際 Release app 的 tick 0–816，每 4 ticks 擷取一格，以 60 fps 重組 1×。解碼 205 frames／3.416667 s，clip 本身仍 3.4 s，最後一格顯示 1/60 s；不是 wall-clock 螢幕錄影。Codex 已看截圖／跨全段的 sequence，未連續觀看正常速度影片；另外實測 app 正常速度播放完成。
- `ready-contact-landing-feet.jpg`：Ready／front-contact／rear-landing 的固定 3× 腳部對照。
- `candidate-release/pawapuro.exe`：獨立 app copy，Space 看完整正常速度；P 暫停、`.` 單步。Executable／DLL／staging 與正式 Release build 相同，只在此 ignored copy 放 candidate GLB／TOML。這是 v1A review 當時的隔離副本；promotion 後一般 `build/debug`／`build/release` 的正式 launch assets 已由 configure 更新為同一 v1A。

檢視可見更寬的鞋部輪廓；Ready／front-contact／final 的深色雙鞋更容易連成一片，跨步展開 frame 65 有鞋面與衣襬投影重疊。Michael＋Julia 已接受這版 footprint 尺寸；這些歷史觀察不代表 foot visual style 已完成，本次不改 pose／camera。已查看 full-scene startup，六個 paired review ticks 的畫面差分只落在鞋部區域，未新增畫面邊緣或打者遮擋；不是完整 collision 證明。Hard-elbow debt 保持原樣。

`revise_footprint.py` 是固定升版前 S2 baseline 的小型 source 操作／保護 script，拒絕覆寫現有 candidate；日常 export 不呼叫它。Candidate 使用既有 `export_sample.py`／`verify_sample.py`／`inspect_motion.py`。`review/style-feet-v1a/motion_expected.txt` 只相對正式 fixture 更新 7 格受影響 bounds、16 個腳部點及 provenance；grip／非腳部點保持原文。可直接用未修改的測試：

```powershell
& ./build/debug/pitcher_motion_test.exe ./pawapuro/batting/pitcher/review/style-feet-v1a/pitcher.glb ./pawapuro/batting/pitcher/review/style-feet-v1a/pitcher.toml ./pawapuro/batting/staging.toml ./pawapuro/batting/pitcher/review/style-feet-v1a/motion_expected.txt
```

Release 同命令換 build 目錄。Promotion 同步採用此已驗證 fixture 的數值，只更新正式檔的 acceptance 註解；CTest target／assert／tolerance 不變。驗證結果見 [environment](../../../docs/development/environment.md#style-polish-v1a-larger-grounded-feet2026-09-16)。正式三檔與 `701a780` 接受 candidate 的 bytes／SHA256 完全相同；沒有重新保存或匯出 `.blend`。Foot Shape（toe／upper／sole）、material／highlight、articulation／sole／cleats、rubber-arm debts 見 [Character Motion Rules](../../../docs/design/character-motion.md#runtime-character-style-polish-v1alarger-grounded-feet)，本次不開始任何下一 task 或 S3。

## S2 runtime animation preview

啟動 `build/release/pawapuro.exe`（或 Debug），**Space** 播放一次 3.4 秒的完整 `pitch_R`；Complete 後再按 Space 重播。**P** 暫停／繼續，暫停時 **`.`** 前進一個 1/240 秒 tick，**Esc** 離開。Minimize 會暫停正在播放的 clip；restore 後按 P 繼續，不補背景時間。Startup 真正是 frame 1／tick 0 Ready，沒有自動 loop。

App 從 executable 同目錄的 `batting/pitcher/pitcher.glb`／`pitcher.toml` 載入；CMake configure 複製正式三檔中的 GLB／TOML。`PitcherMotion` 檢查 GLB 實際 SHA256 與 metadata、clip／duration／release tick，啟動計算 release alignment 後回到 tick 0。Runtime 不讀 `.blend`、不執行 authoring scripts。TOML 的 source hash 是 provenance，runtime 不假裝現場驗證未部署的 `.blend`。

實際 subset：一個 skin `PitcherRig`／13 joints、15 TRS nodes、一個 `pitch_R`／39 TRS channels，全部 LINEAR、共用 205 個 FLOAT SCALAR times；POSITION FLOAT VEC3、opaque COLOR_0 normalized UNSIGNED_SHORT VEC4、JOINTS_0 UNSIGNED_BYTE VEC4、WEIGHTS_0 FLOAT VEC4、inverse binds FLOAT MAT4、indices UNSIGNED_SHORT。2362 unique vertices，每 vertex 最多兩個 nonzero influences，weight sum 容差 2e-5；不正規化壞資料以掩飾錯誤。Unsupported subset 直接拒絕，沒有第二套 parser 或 fallback。

`mesh_glb` → `glb_pose` → `pitcher_motion` → renderer dynamic stream。位置／full grip basis、ownership 與 GPU fence lifetime 由 [設計文件](../../../docs/design/batting-feel.md#s2runtime-pitcher-animation-playbackcpu-skinning2026-09-16) 說明。唯一 staging placement／scale=1 不變，reference ball 始終獨立，release tick 384 只是 diagnostic。

| Review pose | Authoring frame | Runtime tick | Clip time，s |
|---|---:|---:|---:|
| Ready | 1 | 0 | 0 |
| Coil | 49 | 192 | 0.8 |
| Stride | 72 | 284 | 1.183333 |
| Front contact | 79 | 312 | 1.3 |
| Release pose | 97 | 384 | 1.6 |
| Early follow-through | 108 | 428 | 1.783333 |
| Rear-foot landing | 171 | 680 | 2.833333 |
| Balanced | 205 | 816 | 3.4 |

本機證據在忽略的 `build/pitcher-s2/`：先用 app 看完整 1×，再看 `release-motion-sheet.jpg`／`debug-motion-sheet.jpg` 及 `*-tick-*.png` 原始 1920×1080 圖。Sheet 是固定 crop 放大，camera 沒改。Codex 已檢視截圖並實測正常速度執行完成，**未以影片連續觀看，不以 sheet 宣稱正常速度自看通過**。Footprint、hard-elbow 與動態遮擋留待 human review，詳細成本／限制見 [environment](../../../docs/development/environment.md#s2-runtime-pitcher-animation-playbackcpu-skinning2026-09-16)。

`pitcher_motion_test` 使用小型 `motion_expected.txt`：從既有 accepted promotion `source-samples.json` 選八格 grip、全 mesh bounds，以及每格 source vertex IDs 0／200／500／800／1100／1300／1500／1800／1900／2100／2300／2361，Blender 座標轉 `(-x,z,-y)` game local。GLB vertex ordering 可不同，因此點樣本比最近頂點距離，與 grip／bounds 同用原 0.1 mm 容差。Fixture 記錄 source／GLB hash，本輪另直接讀 saved `.blend` 的 evaluated animation 重核；CTest 不需要 Blender。它是 regression evidence，不是第二份 keyframes／runtime Data。

S1 static regression 保留；以下 S1 段落是歷史交付入口，現行 app 使用上面的 S2 path。

## S1 static bind-pose runtime

從 build output 的 `pawapuro.exe` 啟動，讀同目錄下 `batting/pitcher/pitcher.glb`。CMake configure 從正式本目錄 GLB 複製；app 不從 source tree 搜尋，未載入 `pitcher.toml`；animation／skin 僅解析，不 evaluate 或 deformation。啟動時雙臂展開是原始 bind/rest mesh，並非 `.blend` 的 frame 1 Ready；球維持獨立 reference pitch。

- 唯一 primitive：POSITION FLOAT VEC3、COLOR_0 normalized UNSIGNED_SHORT VEC4（opaque）、UNSIGNED_SHORT triangle indices；2362 個 source vertices → 3936 triangles → 11808 個 position/color vertices。單一 embedded BIN、15 static nodes；JOINTS_0／WEIGHTS_0、skin／clip 安全解析但不 evaluate。
- 不支援 materials／textures、morph、extensions／compression、sparse、其他 attribute set 或多 mesh。失敗回報 source／owner／具體原因，不猜格式或退回舊投手。Static transforms 採 glTF column-major，拒絕非有限、非 affine、singular／reflected node transforms；目前 sample 的 mesh node 為 identity。
- `static_pitcher.cpp` 核對 `PitcherMesh` 與 `hand_R` 下的 `grip`；反射 X 一次、三角形交換後兩頂點一次，以公尺 scale=1 加上 staging 的 `pitcher_blockout.position_m`。共享 style／height Data 仍供舊 static batter／歷史設定使用，不再變形 GLB。

先看本機忽略目錄 `build/pitcher-s1/` 的 `debug-startup.png`／`release-startup.png`（1920×1080）及 `debug-pitcher-crop.png`／`release-pitcher-crop.png`（原圖固定區域 3× nearest 放大，非新增 camera）。`*-midflight.png`／`*-complete.png` 與 `*.log` 保留球路檢查；`baseline-*-startup.png` 是本輪開始前的 procedural app，不能當成動畫 before。完整 metrics、cgltf prepare 與測試命令見 [environment](../../../docs/development/environment.md#s1-static-pitcher-glb-runtime-import2026-09-16)。

S1 human review 已接受基本位置／尺度／左右／顏色／grounding 與 scene integration，renderer 無須擴張；不代表 animation／skinning、rubber-arm 動態造型、release／ball attachment、dynamic occlusion／early-flight readability 或 M1 已驗證。Footprint 偏小與 elbow 輪廓的 bind-pose feedback 記於 [Character Motion Rules](../../../docs/design/character-motion.md#s1-acceptance-與延後的-polish)，延後至 S2 能在真正 batting camera 播放後再做 Character Polish review；不退回 importer、不推論所有 animated poses。S2 沿用此資產、不做 polish；後續 full motion authoring 仍先依該 owning doc 建立 Motion Brief。

## S0.2C 已接受並升為正式：Follow-through Rotation／Rear-Foot Recovery

日常 authoring truth 為此目錄的 `pitcher.blend`，GLB／TOML 同步升版；三檔與 `d3de8a99e1072f4ff2b940a6dd6dab07a8c3e7a3` 的接受候選逐 byte 相同。`review/s02c/` 及既有歷史 review artifacts 保留。本次未重新保存來源、修改 motion 或重製影片。Michael 實際正常速度觀看 feedback：「右腳前踩、軀幹延伸都不錯，有投球的味道。」這是 human review 結果，不回填為 Codex 已完成播放自看。

先看忽略目錄 `build/pitcher-s02c/`：

- `before-follow-side.mp4`／`after-follow-side.mp4`：正式 S0.2B／C 同一斜側面，97–205，1×／60 fps，各 109 frames／1.816667 s。
- `after-full-side.mp4`／`after-full-batting.mp4`：完整 1–205，首格 Ready，1×／60 fps，各 205 顯示 frames／3.416667 s；GLB clip time 仍為 0–3.4 s。
- `follow-before-after.jpg`／`follow-batting.jpg`：97／108／137／151／171／205，frame／clip time 標示；固定 crop 放大原 camera raster，沒有改 camera。
- `top-diagnostic.jpg`：evaluated pelvis／chest 朝向、肩線、足心 world/game positions 與 −Z 本壘方向；鞋輪廓是 footprint guide，不是碰撞測試，也沒有新增保存的 camera。
- `release-continuous.jpg`、`landing-continuous.jpg`、`full-overview.jpg`：release 接點、落地前後連續格與全段抽樣。

### C 修改與結果

`revise_follow_through.py` 是當時從 S0.2B 產生 C 的一次性腳本，核對 S0.2B hash 並拒絕覆寫既有 candidate；目前正式 S0.2C 不應重跑它。僅寫 frames 98–205 的局部 TRS；沒有呼叫任何舊 generator／revision script。Pelvis 的額外轉動較早、chest 繼續帶過；前移與右腳空中向前路徑重疊，落地後身體逐漸回穩。頭部轉動較小；雙臂沿原局部收勢隨胸口整體帶動，保留 B 的 roll／weights 品質，沒有重新設計副手。

- Chest yaw：release −25.000° → f144 −108.965° → f205 −65.000°；前折從 release 28° 到 f127 約 56.322°，最後回到 8°。Pelvis／head 使用不同曲線及時序，不是全部同步轉動。
- 最終右足心 world/game Z **16.476601 m**、左足心 **16.850599 m**；右腳比左腳更靠本壘 **0.373999 m**。右腳 X −0.410 m，左腳 X 0.3675 m；沒有再拉成大跨步。
- 沿用已接受的右腳離地高度／鞋底旋轉，增加與軀幹重疊的前移。實測 landing 仍為 f171，因此 source contacts 維持右腳 1–85／171–205、左腳 1–17／79–205；沒有假造新接觸時間。離地底部最小 0.001550 m，171–205 底部誤差與 world translation slide 均 0。
- 不延長 clip：171 落地後仍有 34 格軀幹／頭／手回穩；最後姿勢沒有用複製 hold 補長。Release 97、grip binding、release alignment、camera／scale／staging 全部保留。

### C 匯出與驗證

以下為正式 S0.2C 的既有匯出／驗證入口。只有明確保存 authoring 編輯後才 export；本次 promotion 僅作 byte copy，日常 export 不執行 revision script：

```powershell
$Blender = 'C:/astra-dev/tools/blender-4.5.13-windows-x64/blender.exe'
$Pitcher = 'C:/astra-dev/pawapuro/pawapuro/batting/pitcher'
$Asset = $Pitcher
$Evidence = 'C:/astra-dev/pawapuro/build/pitcher-s02c'
& $Blender --background --factory-startup "$Asset/pitcher.blend" --python-exit-code 1 --python "$Pitcher/export_sample.py" -- --evidence $Evidence
& 'C:/astra-dev/tools/gltf-validator-2.0.0-dev.3.10/gltf_validator.exe' -o -a "$Asset/pitcher.glb" > "$Evidence/validator.json"
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/verify_sample.py" -- --asset-dir $Asset --evidence $Evidence
& $Blender --background --factory-startup "$Asset/pitcher.blend" --python-exit-code 1 --python "$Pitcher/inspect_motion.py" -- --output "$Evidence/motion-audit.json"
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/check_ready_lift.py" -- --scope follow-through --before "$Evidence/before/pitcher.blend" --after "$Asset/pitcher.blend" --output "$Evidence/local-comparison.json"
```

C scope 比較 1–97 全 bones／mesh／keys 與左腳全段，原 1e-6 容差不變；實測皆 0。Weights／rest／mesh／camera／marker 常數完全相同。保留 A 藏球／Closed Ready、B 手套方向及 ring checks，並檢查後足 Z、落地 slide、chest 續轉；沒有把合法的 release 後 motion 誤判成全段 mesh 差值失敗。`inspect_motion.py` 對 C 仍執行全部原 assertions。舊 B scope 也重跑通過。

已檢視最終兩視角 contact sheets、release／landing 連續格、全段抽樣及俯視診斷。編碼後用既有 decoder 核對四支影片 60 fps、frame count、首尾 frame 與 1×，**未完成正常速度播放自看**；不能用逐格或 decoder 檢查取代 human motion review。斜側面約 115–160 部分右臂被 torso 遮擋，兩隻深色鞋接近時分離度有限；原始 batting-view 人物較小。Michael＋Julia 現已接受這版 authoring motion 的整體觀感、重量釋放與 recovery 節奏；上述畫面限制仍保留紀錄，app 動態遮擋與 early-flight readability 尚未驗證。沒有 runtime／GPU／C++ 測試，沒有修改 production／renderer／staging。

具體匯出誤差、hash 與驗證入口見 [environment](../../../docs/development/environment.md#s02c-follow-throughrotationrear-foot-recovery2026-09-15)。

## S0.2B 歷史交付紀錄

以下保存 B 當時的狀態與操作，不是目前正式版本；正式 S0.2C 請使用上方入口。

證據在忽略的 `build/pitcher-s02b/`：

- `arm-before-after.jpg`：f77／79／81 右臂放大，原斜側面與原診斷第二角度，乾淨 before／after。
- `ready-coil-regression.jpg`：Ready／coil 合手、藏球與抬腳對照；`glove-poses.jpg`：兩個既有視角的展開／指向／回收，附 frame／clip time。
- `after-full-side.mp4`：完整 1–205、60 fps／1×、3.416667 s；對應 before 直接看 `build/pitcher-s02a/after-full-side.mp4`。
- `before-batting-45-105.mp4`／`after-batting-45-105.mp4`：45–105、61 frames、60 fps／1×、1.016667 s。Before 使用已核對來源 hash 的 S0.2A renders，沒有重渲整段。
- `transition-continuous.jpg`、`deformation-overview.jpg`、`final/arm-diagnosis/`：分手／接回、整段抽樣與局部診斷。影格檢視不等於正常速度播放自看。

**正式 authoring truth 是 `pitcher.blend`**，同目錄 GLB／TOML 同步升版，預設 frame 1。三檔與 commit `4dc9569de51a7b7b099f6259dbafd05310c62352` 的接受候選逐 byte 相同；`review/s02b/` 保留歷史驗收 artifact。Promotion 未重新保存正式 source，隔離副本重匯出亦完全一致。驗證見 [environment](../../../docs/development/environment.md#s02b-promotion-to-official-pitcher-asset2026-09-15)。B before 位於 `build/pitcher-s02b/before/`，是 S0.2A；`build/pitcher-s02a/before/` 是 S0.1。

### 修法與驗證入口

只比較兩個右臂策略：roll-only 仍留下前臂收窄；選用相同 pose roll 加右臂 tube 局部 weights。`revise_arm_glove.py` 讀已存檔 S0.2A，補償 child TRS 以維持右手／grip，roll 修正限 50–109、兩端平順接回；混合集中在肘附近四圈，管端由前臂帶動並收在原球形手內。沒有改 rest roll、mesh、骨長或使用另一種 skinning，並非整條手臂改成單一 rigid weight。

右臂確認後才用 `point_glove.py` 修改左臂 50–96：展開後肩→手套中心朝 game −Z，f79 水平偏角約 0.50°，保留彎曲與自然高度；97 起完全接回舊左臂。這兩支是明確的一次性局部編輯，不供日常 export 呼叫；本輪未重跑 create／reblock／ready-lift 腳本。

全 205 frames：body／head／feet matrices 差 0，左臂授權區間外差 0；右臂關節位置最大差約 3.19 μm、右手／grip matrix 最大絕對差 7.63e-6，屬 parent compensation 浮點誤差。只改 144 個右臂 tube vertices 的 weights；deformation 差異與 trajectory 分開量測。原 source／GLB／round-trip／release 容差不變，詳見 [environment](../../../docs/development/environment.md#s02b-arm-deformationglove-direction2026-09-15)。

日常匯出／驗證使用正式資產；export 僅供有意匯出已存檔來源時執行，promotion 本身不重寫正式匯出檔：

```powershell
$Blender = 'C:/astra-dev/tools/blender-4.5.13-windows-x64/blender.exe'
$Pitcher = 'C:/astra-dev/pawapuro/pawapuro/batting/pitcher'
$Asset = $Pitcher
$Evidence = 'C:/astra-dev/pawapuro/build/pitcher-s02b'
& $Blender --background --factory-startup "$Asset/pitcher.blend" --python-exit-code 1 --python "$Pitcher/export_sample.py" -- --evidence $Evidence
& 'C:/astra-dev/tools/gltf-validator-2.0.0-dev.3.10/gltf_validator.exe' -o -a "$Asset/pitcher.glb" > "$Evidence/validator.json"
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/verify_sample.py" -- --asset-dir $Asset --evidence $Evidence
& $Blender --background --factory-startup "$Asset/pitcher.blend" --python-exit-code 1 --python "$Pitcher/inspect_motion.py" -- --output "$Evidence/motion-audit.json"
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/check_ready_lift.py" -- --scope arm-glove --before "$Evidence/before/pitcher.blend" --after "$Asset/pitcher.blend" --output "$Evidence/local-comparison.json"
```

`verify_sample.py --asset-dir` 只選擇待檢資產，不改 staging；`check_ready_lift.py --scope arm-glove` 保留 B 的逐格保護，原 A 規則仍可單獨執行。`inspect_motion.py` 對 S0.2B 仍執行原 checks。Renderer 可用既有 `--frames` 渲染 45–105；encoder 核對 manifest 的連續範圍與實際檔案，再解碼驗證 FPS／frames。Sheets 使用 `assemble_ready_lift.py --scope arm-glove --evidence ... --before-renders .../build/pitcher-s02a`，不建立新 preview framework。

目前已檢視局部、連續影格與整段抽樣，未完成正常速度播放自看，未重試已知失敗的播放器；沒有新參考影片觀看。沒有 runtime／GPU／C++ build 測試。右臂 deformation 已獲 human acceptance；收勢遮擋仍須隨 C review，數值不代表完整 self-collision 保證；C 的追加左旋／右腳跨前維持待處理。

## S0.2A 歷史基準與操作紀錄

以下保留 A 交付當時的 scope、限制與命令；目前 A／B 已接受，正式 S0.2B 請使用上方入口。舊 A 的 72–205 全 mesh 不變檢查不適用於 B 的局部 deformation。

2026-09-15：本版只修正 Closed Ready、合手藏球與連動抬左腳，**等待 Michael＋Julia review／可能再修正，並非整支投球已通過**。S0.1 同樣是未驗收 candidate；本輪沒有進入 B、C 或 S1。

## Michael 先看什麼

Evidence 在忽略的 `build/pitcher-s02a/`，以下檔名相對該目錄：

| 檔案 | 用途 |
|---|---|
| `before-ready-side.mp4`／`after-ready-side.mp4` | ed3d8be S0.1／S0.2A，既有斜側面、1×、60 fps、frames 1–84，各 1.4 s。 |
| `before-ready-batting.mp4`／`after-ready-batting.mp4` | 同區間、同倍率與 staging batting camera。 |
| `after-full-side.mp4`／`after-full-batting.mp4` | 修正版完整 frames 1–205、1×、60 fps、205 顯示 frames／3.416667 s。 |
| `contact-side.jpg`／`contact-batting.jpg` | Before／after Ready、抬腳中段、coil、展開、接回 frame 72；附 frame／clip time。 |
| `ready-coil-side.jpg`／`ready-coil-batting.jpg` | Ready／coil 手套、持球與左腳放大對照；裁切原 camera render，沒有改 camera。 |
| `separation-continuous.jpg`／`transition-continuous.jpg` | Frames 49–63 分手露球與 64–76 銜接連續影格。 |
| `after-overview.jpg` | 整段抽樣 overview，不是正常速度自看證據。 |
| `arm-diagnosis/diagnostic-sheet.jpg` | S0.1 frames 77／79／81，斜側面＋另一 authoring 角度，clean／wire／骨架投影。 |

Before 來源保存在 `build/pitcher-s02a/before/`，對應 commit `ed3d8be8840a45491b821964677d2295952905cd`。**不要用 `build/pitcher-s01/before-normal.mp4`，那是更早的 S0**。原 S0／S0.1 證據與本輪中間 candidate 都留在忽略目錄。

本輪實際檢視 saved-source renders、兩個視角、分手及接點連續影格，並用 Blender 隨附 decoder 核對六支 MP4 的 FPS／frame count。**未完成 Codex 正常速度播放自看**；影格檢視不等於影片觀看。本輪可讀附件只有文字，沒有 Michael 的參考影片／圖片，未宣稱觀看或 reference-verified timing。沒有 app／runtime／GPU 測試。

## 本輪修改與邊界

- 胸口 Ready 朝 game −X（三壘側）：game +90° Y yaw 使原 front −Z 轉成 −X；透過 basis conversion 寫入 Blender，而非猜畫面方向。Pelvis 起始約 80°，chest 90°；coil 分別約 98°／110°。頭的 world orientation 保留 S0.1，注意力仍朝本壘。
- 雙手在 chest-relative clasp pose 合攏；手套包覆由 `hand_R`／固定 `grip` 帶動的球。沒有改 grip binding、球半徑、parent 或 visibility 規則。Frames 1–49 球面取樣位於手套橢球內；分手後沿連續手部路徑露出。唯一隱藏規則仍是 release marker 後。
- 左腳初始支撐位置／yaw 改為封閉準備站姿，1–17 固定；18 起沿與軀幹蓄力協調的弧線向自身右側收進。Coil 鞋心在手套正下方，沒有永久改腳的 parent。右腳沿用原完整 transforms。
- 50–71 將上述局部修訂平順收回既有 stride，修訂權重在 72 歸零且端點導數為零。使用固定長度 FK；新合手姿勢的 pose roll 隨轉身後的 rest orientation 對齊，再接回既有 roll，不改 rest transforms／weights。
- 72–205 的 keys、evaluated bones 與 mesh 完全保留；release frame 97、60 fps、frames 1–205、四段接觸區間均不變。接點沒有新增 hold。展開仍偏快、深色雙鞋部分重疊，以及簡化手臂彎曲輪廓，仍是 human review 項目。

### 右臂診斷與 B／C 待辦

Frame 79 的凹折在兩個角度、相鄰 frames 與 wireframe 都可見，不是單純遮擋。骨架連接連續，沒有該格突然 roll 跳變；主要是既有寬範圍混合 weights 搭配接近相反的 skinning rotations，讓 tube 局部壓扁。前臂／手部混合區比上臂更嚴重。數據及限制見 [environment](../../../docs/development/environment.md#s02a-closed-readycoordinated-leg-lift2026-09-15)。

此為姿勢相關的 B 區段問題，未證實阻止這次 A 的三個關係；本輪沒有修改 frame 79 或重建 rig。B 應另檢查局部 roll／weights 的最小修正；具體修法尚未驗證。跨步時手套指向本壘、出手前右臂修形、出手後增加左旋與右腳跨到左腳前方，全部維持待處理，不因本輪技術檢查通過而自動授權。

## 可編輯來源與最小契約

- `pitcher.blend` 是日常 authoring truth，預設 **frame 1 Ready**。一個 mesh／skin／`pitch_R` clip、13 骨骼、15 GLB nodes、39 TRS channels；2362 vertices／3936 triangles。
- Mesh、topology、五色、weights、rest／hierarchy、camera、placement／scale 不變。大頭、扁鞋、detached feet、球形手與連續簡化手臂保留。
- 本輪實際修改腳本是 `revise_ready_lift.py`，讀已存檔 S0.1 並拒絕覆寫目的 candidate；只寫 frames 1–71。`create_sample.py` 是初始生成，`reblock_motion.py` 是一次性 S0→S0.1，**本輪未重跑兩者**，也不是日常 export 依賴。
- `export_sample.py` 只讀目前存檔，不重建、不保存 `.blend`；匯出 LINEAR GLB／TOML。唯一 release marker 97 → clip 1.6 s → 未來 240 Hz tick 384；不是 Native release integration。
- Source 60 fps／fps_base=1、1–205 → GLB 0–3.4 s。Preview 包含首尾端點，205 顯示 frames 為 3.416667 s。沒有改 clip timing。
- 右腳接觸 1–85／171–205，左腳 1–17／79–205；marker／review poses／contacts 仍從 `.blend` 產生 metadata。`grip` 是 `hand_R` child，表示球心；marker 後隱藏 authoring 球，不模擬飛行。

### 空間與格式

長度已是公尺，2.45 是初建比例 scale，runtime 不再乘；placement `(0, 0.355, 18.5166)` m、scale=1。Blender local = `(-game_local.x, -game_local.z, game_local.y)`；標準 glTF +Y-up export 得到 GLB local = `(-game_local.x, game_local.y, game_local.z)`。S1 轉回 game 反射 X 一次、反轉 winding，再加 placement。

GLB 只有 POSITION／COLOR_0／JOINTS_0／WEIGHTS_0、triangles、LINEAR animation；四 influences 槽位，最多兩個非零 weights。沒有 materials／textures／normals／morph／extensions。預覽沿用 vertex-color emission／Raw，未調 lighting；round-trip byte color 的量化限制保留。

## 匯出與檢查

先保存 Blender 編輯，再從 repo 根目錄執行；不跑 generator：

```powershell
$Blender = 'C:/astra-dev/tools/blender-4.5.13-windows-x64/blender.exe'
$Pitcher = 'C:/astra-dev/pawapuro/pawapuro/batting/pitcher'
$Evidence = 'C:/astra-dev/pawapuro/build/pitcher-s02a'
& $Blender --background --factory-startup "$Pitcher/pitcher.blend" --python-exit-code 1 --python "$Pitcher/export_sample.py" -- --evidence $Evidence
& 'C:/astra-dev/tools/gltf-validator-2.0.0-dev.3.10/gltf_validator.exe' -o -a "$Pitcher/pitcher.glb" > "$Evidence/validator.json"
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/verify_sample.py" -- --evidence $Evidence
& $Blender --background --factory-startup "$Pitcher/pitcher.blend" --python-exit-code 1 --python "$Pitcher/inspect_motion.py" -- --output "$Evidence/motion-audit.json"
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/check_ready_lift.py" -- --before "$Evidence/before/pitcher.blend" --after "$Pitcher/pitcher.blend" --output "$Evidence/local-comparison.json"
```

每步確認 exit code。原 0.1 mm source／GLB／round-trip／release 容差不變。`inspect_motion.py` 對 S0.1 與 S0.2A 執行相同固定骨長、grip、接觸、頭部 proxy、opening 順序與 release 方向檢查，沒有因 revision 名稱跳過。`check_ready_lift.py` 額外逐格對照 72–205 所有 13 bones／2362 vertices，容差各 1e-6，實測皆 0；並比較不變資料與 protected keys。

## 重製證據

`render_preview.py` 用原 camera 離線渲染整段到新的空白目錄，manifest 含 source SHA256；`encode_preview.py` 先核對完整 renders，再可用 `--last-frame 84` 產生比較區間，不拉伸時間。以下為 after 斜側面；before 改用 `$Evidence/before/pitcher.blend`，batting 使用 `--camera batting --width 1920`：

```powershell
& $Blender --background --factory-startup "$Pitcher/pitcher.blend" --python-exit-code 1 --python "$Pitcher/render_preview.py" -- --output "$Evidence/after-side" --animation --width 1280
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/encode_preview.py" -- --frames "$Evidence/after-side" --output "$Evidence/after-ready-side.mp4" --last-frame 84
& $Blender --background --factory-startup --python-exit-code 1 --python "$Pitcher/encode_preview.py" -- --frames "$Evidence/after-side" --output "$Evidence/after-full-side.mp4"
& $Blender --background --factory-startup "$Evidence/before/pitcher.blend" --python-exit-code 1 --python "$Pitcher/diagnose_arm.py" -- --evidence $Evidence
& 'C:/Users/USER/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' "$Pitcher/assemble_ready_lift.py" --evidence $Evidence
```

Sheets 要有 before／after 的 side／batting renders 與 arm diagnosis。影片、renders、JSON、logs 都留忽略的 build，不提交。乾淨預覽沒有箭頭／骨架；diagnostic 的額外 camera 只存在該次記憶體，不保存到 source。

沒有 production C++／HLSL／renderer／staging Data 變更，也沒有 C++ build／CTest／app／GPU 測試。Strike-zone truth、horizontal off-axis camera、pitch sightline、deterministic Native simulation 保留；動態遮擋與 early-flight 對比仍未驗收。
