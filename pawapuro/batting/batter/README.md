# Left-handed Batter — Authoring S0 candidate

2026-09-16。Motion Brief 已獲 Michael＋Julia 接受；本目錄首次交付一名左打者與一支 `swing_L`，**authoring motion 待 human review**。不是 runtime asset promotion，也未接 app／碰撞。設計與來源觀察由 [batter-motion.md](../../../docs/design/batter-motion.md) 擁有。

## 先看這兩支

Ignored `build/batter-authoring-s0/`：

- `swing-batting-1x.mp4`：既有 staging 對應 batting camera，完整乾淨 swing。
- `swing-side-1x.mp4`：完整 three-quarter，輔助看支撐／lag／收勢；依全部 225 格的角色與球棒範圍取景。
- 兩支均為 **60 fps、1×、225 格**；首格是 f1 Ready。Clip 時間 0–3.733333 s，影片 3.750 s，差別僅最後一格顯示 1/60 s。無球、聲音、VFX、診斷箭頭。
- `batting-contact-sheet.jpg`、`side-contact-sheet.jpg`：11 個 phase poses；`plant-acceleration-continuous.jpg`：f103–126 每格；`follow-settle-sequence.jpg`：後段繞身至 finish。
- `hands-bat-lag-closeup.jpg`、`batting-hands-bat-lag-closeup.jpg`、`foot-support-closeup.jpg`：手棒與接地放大。
- `motion-diagnostic.jpg`：evaluated pelvis／chest yaw、腳底、hand／grip／barrel／tip arcs、world／screen spacing；只作診斷。

已實際檢視上述關鍵／連續影格與 source 全 225 格數值；**未完成播放器中的正常速度連續自看**。解碼確認影片 FPS／格數不等於視覺驗收。力量感、lag 可讀性及 finish 交由 Michael＋Julia review。

## 可編輯 source 與最小契約

- `batter.blend` 是 authoring truth，預設停 f1；不要用 generator 取代日常編輯。`create_sample.py` 只作首次建檔且拒絕覆寫既有 source；`refine_sample.py` 是本次一次性輪廓／vertex-color 修訂，有重跑 guard。最後 source 另修正 AuthoringOnly zone guide 與 three-quarter 取景，已同步記入建檔腳本；不是 export 的隱藏步驟。
- `export_sample.py` 讀取已存檔 source、取樣／檢查並產生 GLB／TOML／evidence，**不呼叫 create／refine，也不保存 .blend**。手改後先在 Blender 明確存檔，再 export。
- 一個 armature、17 joints、兩個 mesh：`BatterMesh` 與獨立 equipment `Bat`。Root／pelvis／chest／head、左右各 3 個 hidden arm controls 與 hand、detached feet，以及 3 個 bat semantic joints。沒有 fingers、IK、兩手 solver 或通用角色系統。
- Rubber arms 使用具體曲線的 hidden bend controls，烘焙 location／quaternion／scale。固定 bone length 指 **control 本身長度與 unit scale**；相鄰 controls 的距離可變，並非宣稱真人上下臂節長固定。視覺 elastic contour 仍需 review。
- `hand_R → bat_grip → bat_barrel / bat_tip` 是唯一 attachment hierarchy；Bat 全部 rigid weights 給 bat_grip，整段不放棒。另一手由同一握把軸的 authored offset 協調，沒有雙 parent。Source 持有 mesh／rig／action；匯出為自包含 GLB。本輪沒有 runtime lifetime／loader caller。
- 公尺／比例已烘進 source，`runtime_scale=1`。GLB → game 為 `(-x,y,z)` 加目前 staging batter placement `(1.25,0.008,0)`；TOML placement 只記 provenance，不能作另一份 gameplay Data。Blender basis 為 game-local `(-x,-z,y)`。
- `AuthoringOnly` 有 home plate、目前 strike-zone plane、placement、review cameras；timeline 有 pitcher guides。匯出只選 `BatterAsset`，不含 floor／guides／camera／light。保留 named bat nodes 是為本支 swing 的 path 診斷。
- GLB：單一 non-looping `swing_L`、60 fps、1–225、LINEAR baked samples、skin／vertex colors、沒有 materials／textures。兩個 mesh 的未來 runtime caller 尚未實作，不改目前 pitcher loader。

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

已知 motion review 問題：f118 screen spacing 突增是本版短促加速候選，是否太急仍待正常速度判斷；簡化 torso 的轉向輪廓仍淡；手臂近下巴時會疊成粗輪廓，continuous rubber silhouette 尚待 review；早期 follow-through 手棒在大頭後方遮擋，不能只靠曲線驗證就說可讀。全片 bat surface vertices 對 head／helmet ellipsoid 檢查為 0 inside samples，但**不是完整 triangle collision**，不涵蓋 brim、手臂／torso 自穿插或所有遮擋。Two-hand grip、lift 幅度與較長 settle 是否有力量，仍以正常速度 human review 判定。

沒有執行 C++ build／CTest／GPU validation；production code、staging、pitcher 完全未改。沒有 runtime BatterMotion、bat-ball collision 或 contact presentation。完成此 candidate 後停止。
