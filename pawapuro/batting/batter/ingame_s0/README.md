# In-game S0 — Take／commit authoring candidate

2026-09-17；**待 Michael＋Julia human review**。這裡只擁有 editable candidate 與開啟方式；Motion Brief、reference、timing 與檢查結果見 [Batter Motion Gate B](../../../../docs/design/batter-motion.md#player-swing-s0-gate-b)。未 promotion，runtime 仍讀上層 accepted 三檔。

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
