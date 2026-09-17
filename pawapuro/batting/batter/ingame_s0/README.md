# In-game S0 — Take／commit authoring candidate

2026-09-17；**Gate B motion 已獲 Michael＋Julia human acceptance；Gate C runtime candidate 待 human review**。這裡擁有 editable source、runtime export 與操作方式；Motion Brief、reference、timing 與檢查結果見 [Batter Motion Gate B](../../../../docs/design/batter-motion.md#player-swing-s0-gate-b)。上層原三檔仍供舊 choreography／contact regression；app 的 Manual Swing S0 使用本目錄的新衍生資產。

## 開啟與選擇案例

用 Blender 4.5.13 LTS 開啟 [batter_ingame_s0.blend](batter_ingame_s0.blend)，不需要執行 script。頂端 **Scene selector** 選：

- `01_Take`：f1–201，不 commit；f117 plant，f169 完成收回。
- `02_Early`：f1–223；f109 commit，前腳仍下降，f115 plant。
- `03_Nominal`：f1–229；f115 commit，f118 plant。
- `04_Late`：f1–239；f125 commit，此前 f117 已 plant，不重新 lift。

每次從 f1 播放，60 fps／fps_base=1；Numpad 0 回到已選的 accepted batting camera。Timeline markers 顯示 release、arrival guide、commit、plant、contact-area 與 finish；arrival 的整數 marker 只是附近提示，精確比較時間約 1.992833450 s。`contact-area` 不是 collision／Hit／Perfect。可切 `Review_ThreeQuarter` 對應相機作輔助，但主要影片固定 batting camera。

四個 scene 的 mesh／rig data 共用，骨骼動作各有具名 action；不是四個獨立改過 preparation 的角色。`Preview_Take` 保存共同 preparation／take；三個 `Preview_*` 是其共用前段＋具體 entry＋共同 swing 的 baked cases。`Shared_Committed_Swing_source_derived` 保留共用 committed motion；`AcceptedReference_swing_L_READ_ONLY_COPY` 是檔案內供比較的舊 action 副本。編輯請在 candidate action 上進行，不用 generator 重建角色。Keys 以 quarter-frame 取樣保存；沒有新增 bones、handlers、IK、animation graph 或 runtime 分支系統。

## 先看成品

1. [四例 1× 影片](../../../../build/batter-ingame-s0/batter-ingame-s0-1x.mp4)：14.866667 s；Take 0–3.350 s、Early 3.350–7.066667 s、Nominal 7.066667–10.883333 s、Late 10.883333–14.866667 s。
2. [Phase／腳部比較](../../../../build/batter-ingame-s0/phase-feet-comparison.jpg)。
3. 主視角遮擋處才參考 [entry／support 輔助圖](../../../../build/batter-ingame-s0/entry-support-aux.jpg)。

上述 ignored evidence 只存在本機，不隨 Git clone 取得；候選 `.blend` 已納入版本管理，可直接重開／播放。影片是 Blender 離線 authoring preview，不是 app capture、input latency 或 gameplay feel 驗收。原始 reference MP4 未複製進 repo。

## 保護與 provenance

上層 accepted 三檔開始／結束 SHA256 不變：

- `batter.blend`：`74e31d2558970ec1bdf012b7a0071ba71ead516673e65782e31320b81daa2245`
- `batter.glb`：`6977f690c0aad047518882db310246893f6bab4537f71261b0c423d6a343bcf2`
- `batter.toml`：`0c49c158c062f0e16595cf58d4589b4dbf978490d31c878f261891a5420277a1`

`build/batter-ingame-s0/authoring-provenance.json` 保存候選 hash、共同 source phase map 與壓縮存檔前後 action 摘要；`candidate-check.json`／`surface-check.json`／`video-validation.json` 保存實測。未輸出 candidate GLB／TOML，不擴充 exporter。本輪不代表任意 commit 時刻或 production contact 已可用。

## Gate C runtime candidate

啟動 repo 的 `build/release/pawapuro.exe`（或 Debug）。啟動時自動以主螢幕可用工作區寬度的 75% 選擇 16:9 client size，必要時連同邊框／標題列及留白等比例縮小，整個視窗置中；不需額外參數，也不再要求至少 1920×1080。可在 [staging.toml](../../staging.toml) 的 `[window] width_fraction = 0.75` 調整比例（finite，0 < value ≤ 1，缺省 0.75）；`--staging` 仍可指定本次資料來源，預設讀 exe 旁的 build copy。取整為 16×9 單位；只於啟動校正一次，不追蹤顯示器變更，不提供 resize／maximize／fullscreen。工作區查詢失敗會 log 並退回 display bounds 加保守留白；連 display bounds 都不可用時明示退回 800×450／SDL 預設位置，無法保證符合未知工作區。邊框查詢失敗使用保守估計，不更動 Windows DPI；投影使用實際 client pixels，BallAid 固定 pixel 值依高度／1080 縮放。其他螢幕與 DPI 組合尚未實測，尺寸、閱讀與順暢度待 Michael 試玩。

Space 開始／完成後下一球；Arrows 移動 live aim、R recenter；J 新按下在 preview tick **432–496（含端點）** 可 commit；P pause、`.` 單步、Esc 離開。Title 的 `OPEN/CLOSED` 與拒絕／queued／commit 狀態可供操作確認。正式 input／snapshot／contact 邊界由 [batting-contact Gate C](../../../../docs/design/batting-contact.md#player-swing-s0-gate-c) 擁有。

### Export 與資產責任

在 repo root，用既有 Blender 4.5.13 執行：

```text
blender --background --factory-startup --python-exit-code 1 --python pawapuro/batting/batter/ingame_s0/export_runtime.py
```

[export_runtime.py](export_runtime.py) 只開啟 saved `batter_ingame_s0.blend`，不 save、不呼叫 generator。Source SHA256 必須為 `9e37b73e8419e721b885326e06ba3b185fbd11168da77184a13e2b07d6961ea6`；原三檔與 Gate B source bytes 均保留。

- `motion.glb`：單一 `ingame_source` clip，取自 saved source 內 `AcceptedReference_swing_L_READ_ONLY_COPY`；相同 mesh／rig／weights。不是三個案例選片，也不是每個 tick 一支完整動畫。
- `motion.toml`：saved source／GLB／exporter SHA256、Blender 版本與 `gate-b-world-residual-v1` 配方契約。CMake 複製 GLB／TOML 到 exe 旁的 `batting/batter/ingame_s0/`，新 clone 不依賴 ignored JSON 或 authoring script。
- `motion_expected.txt`：直接取自 saved Take／三個 baked preview actions 的 5,304 組 bone world matrices，含 quarter-frame keys 與 half-tick sub-frame；不由 Native 產生 expected。

[IngameMotion](ingame_motion.cpp) 在載入時快取 donor clip 的 225 個整數 source-frame world poses，再依 Gate B 原配方做 world TRS interpolation、preparation phase、六格 pose／velocity residual、front-foot descent、rear toe anchor 與 hand／bat attachment。共同 source phase knots 為 `(0,99,0)、(4,113,1.6)、(6,116,1.5)、(10,121,1)、(114,225,1)`，第三欄是 Hermite source-frame slope；速度差分 epsilon 為 0.01 authoring frame。Native 保留 Blender 4.5 小角度 exponential-map 約定，沒有將數值上近零的原殘差改為另一套速度（[Blender mathutils axis-angle 約定](https://github.com/blender/blender/blob/v4.5.3/source/blender/blenlib/intern/math_rotation_c.cc#L1015)；另以本機 4.5.13 實測確認）。

這個 donor clip 的 60 Hz transport 不等於把 Gate B quarter-frame motion 降採樣：Native 在每個 240 Hz key 上重建原配方，fractional preview tick 則以相鄰 quarter-frame 的 **local translation／normalized quaternion component interpolation** 重現 baked key 行為。World time、commit-relative authoring frames 與 donor source frame 明確分開；render 與 bat semantic matrices 出自同一 pose。只增加普通 Engine `rebuild_glb_pose`，動作與支撐語意留在 Pawapuro。

連續入口的新增 development support policy：commit authoring frame c∈[109,115] 的剩餘下降長度從 6 線性降至 3 frames；c∈(115,117) 從 3 降至 0；c≥117 保持已 plant 的腳。下降沿 Gate B 的 Hermite foot-phase 接到 f117，沒有重抬脚、追球或改 swing phase。代表性三例與 saved-source fixtures 對照，其餘合法 ticks 另作支撐檢查；這項連續政策仍待 runtime human review。

Asset、225-frame cache、pose／skinning scratch 與 triangles 由 `IngameMotion` 活到 preview 結束；每次按鍵不重新載入／配置。App 的 combined vertices 預先 reserve，renderer borrowing／fence 契約沿用原實作。Read-only fractional sampling 使用 caller-owned pose scratch，不改 authoritative tick／bat semantics。

### Gate C 實測與 evidence

- Debug／Release build 與各 14 項 CTest 通過；原 12 項 pitcher／pitch／aim／contact baseline regression 保留。新增 Native input／motion 檢查涵蓋 65 個合法 commit ticks、Take、entry／plant／sweep／finish、sub-frame 與 completion。
- Saved-source matrix 最大差約 `5.09e-5`，原定門檻 `1e-4`；共同 preparation 差 0；全合法 ticks 的 post-plant drift 0、足底最低約 `−2.09e-7 m`（guard `−1e-6 m`）、hand offset 約 `8.02e-7 m`、grip 約 `2.85e-7 m`。不是完整 mesh collision 或 silhouette acceptance。
- Native tests 驗證 press／held／repeat 相當的 edge gating、同球再次按壓、domain 外拒絕、pause／step／resume、focus/minimize eligibility、reset、immutable snapshot、30／60／120 Hz recorded-command replay 與受控 backlog。Raw OS event 的不同 cadence 不保證同一 target tick。
- 真正 app 以 computer-use SendInput → Windows／SDL events 操作。最終 motion／input 路徑的 Debug capture consumed ticks 為 439／464／484，sweep cues 為 463／488／508、contact-area cues 為 479／504／524，finish 為 895／920／940；另一次 Release smoke 已接受 endpoint496 並完成至952；cue 是動作 phase，沒有 collision／Hit 判定。Debug GPU validation 0 errors，Esc 正常 shutdown／exit 0。
- [實際 app 1× 影片](../../../../build/player-swing-s0/manual-swing-runtime-1x.mp4)：Take → 三個不同時刻 J → 下一球，17.9 s、960×600、30 fps encoding。原始 PrintWindow 擷取約 **11.60 fps**，依 wall-time timestamps 補重複影格，最大 nearest-sample 誤差約 47.5 ms；不是逐 tick 重建，亦非原生 30 fps capture。較快的 BitBlt 試取樣空白，已停止該方法。
- [少量 runtime poses](../../../../build/player-swing-s0/runtime-poses.jpg)、`debug-entry.png`、input／GPU logs、capture timestamps 與 encode validation 位於 ignored `build/player-swing-s0/`。錄影只能協助看完整流程；快速 sweep 的細節請以 app／saved-source fixtures 檢查。未量測端到端 latency，未宣告玩法手感接受；新 motion physical contact 明確 NotEvaluated。

Gate B 舊 evidence 與 acceptance 歷史保留；Gate C 不重新 author、不修下巴疊影／大頭遮擋，不開始 contact integration。
