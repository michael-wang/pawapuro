# Hit Authorization S0 — Gameplay Contact Model

最新 runtime 狀態見文末 **Hit Authorization S1**；下方 S0／Gate C 等歷史記錄保留。

2026-09-17；**Hit Authorization S0 五層責任已獲 Michael＋Julia human review accepted；S0.1 Reticle Semantics 亦已 human review accepted；Player Aim S0 interactive candidate 待 review**。僅定責任與 normalized examples；沒有 production authorization、正式 swing input、correction solver 或 ball response。Bat Contact S0／S1／S2 已接受的 physical truth 保留，M1 未完成。

## Reference：觀察與解讀分開

來源是 Michael 提供的兩張截圖與 `ref-batting-contact.mp4`，不是原遊戲規格或公式。檔案識別見文末。

- **直接觀察／截圖**：Normal 圖有寬黃色橢圓／十字，Power 圖有小黃色圓形準星與「強振」。兩張分別是陳傑憲（ミート C）與林安可（ミート F），角色與情境不同，不能用這兩張圖定量隔離 mode 或能力的影響。
- **直接觀察／影片**：實際解碼 f35–65 揮擊區段的連續影格；已檢視全片每 15 frames 抽樣、全畫面 f49／52／54／55 與局部 f46–58（不是逐一看過所有解碼影格）。f49–52 的來球／球形標記在黃色游標上方、偏離中心；揮擊後 f76、f91 可見「ファウル」。影片中打者也是陳傑憲、ミート C。這支持「偏心瞄準後仍有 foul」的可見順序，但沒有對照試驗證明能力造成結果。
- **限制**：放大的球形 UI 標記不能直接當 physical ball center；切鏡、遮擋與 screen-space 投影使精確接觸格、q、ellipse 邊界內外及 correction 用量無法從影片確定。不能宣稱已驗證 q≈1 或原遊戲的 high-Contact 公式。
- **Michael 的語意／本案 interpretation**：將此讀成 high-Contact defensive rescue 的設計線索；較大 spatial aim error 仍可避免 miss，但不自動獲得完美擊球。這是 Pawapuro 候選方向，不是對原作內部機制的查證。
- 影片實測 2866×1612、30 fps、157 frames、5.233 s；本文 f 為一基 frame，capture time=(f−1)/30。播放倍率與原始遊戲 FPS **未知**，不套用另一支舊 reference 的 YouTube 0.5× provenance。此次是解碼影格檢視，未執行正常速度影音播放，也不推導 authoritative timing。

## 五層責任

| 層 | 回答與輸出 | 不負責 |
|---|---|---|
| Player Aim / Intent | 保留 aim center、signed error (dx, dy)、normalized error (ex, ey)、q magnitude 與 swing mode；未來附 input tick 與版本 provenance | 不是 bat centerline、capsule 或 rendered mesh |
| Hit Authorization | 依 mode、Contact 與 spatial aim error，允許或拒絕一次有效接觸嘗試 | 不擴大物理半徑、不保證實際命中、不給 Perfect |
| Contact Resolution | 授權後，在有限合法候選內以 continuous geometry 找 time、barrel location u、normal、relative motion | 不決定能力、不憑空指定接觸點 |
| Contact Quality | 未來解讀 aim、修正成本與真實接觸訊號 | 本輪無 score、weights、sweet spot 或等級門檻 |
| Ball Response | 未來消費接觸資訊，產生 exit velocity、launch angle、spin、foul／fair | 本輪不產生球路回應 |

Authorization 是必要 permission，**不是充分接觸保證**。允許嘗試但 budget 內無解，必須回報無可信 solution，不能補造 hit。反過來，未授權的 physical overlap 仍是 geometry 事實，不自動成為 gameplay accepted hit；未來 caller 必須解決畫面相交卻不承認 hit 的演出一致性，不能隱匿事實。本輪既有 S1 event 完全不加 gate、不改行為。

## Normal／Power 與能力

黃色 ellipse 應表達 **gameplay authorization region**；中心是 player intent，dx／dy 是 relevant pitch point 相對中心的 aim error。比較點究竟採何 contact plane／crossing，以及 input 鎖定時刻，由實際 caller 決定，保留既有 strike-zone truth。

保留 `aim_error = (dx, dy)`，以及 `ex = dx/rx`、`ey = dy/ry`；`q = ex^2 + ey^2` 只摘要 normalized 偏離程度，不能取代 signed vector。q 是平方量，不是距離本身：q=0 中心，q=1 邊緣，q>1 外側。Authorization 的 rx／ry 尚無正式 balance 值，UI 不必永遠使用解析橢圓；下節 Player Aim S0 的公尺尺寸僅是 development reticle 首版 tuning。這是 mode-specific normalized design coordinate，不是從 reference 像素反算的公式。

- **Normal**：較大 authorization ellipse、較多 rescue 空間。Contact 第一版只影響其 spatial aim 範圍；不預設擴大 timing window、增加 Power 或改 response。
- **Power**：顯著較小 region、較少 correction budget；需要更精準的 intent，Power 的較高 upside 留給未來 response。Power 不放大 aim region。
- **硬邊界**：Contact／mode 都不改 physical ball sphere 或 bat capsule，不用 hidden giant bat／radius multiplier。Power 的取捨不應靠縮小 physical bat，否則把操作能力混成器材尺寸，破壞同一 physical truth。

## S0.1 — Reticle Semantics amendment（human accepted）

### Observed：靜態 UI，不是內部公式

已實際檢視本次對話附圖：Normal `codex-clipboard-f375b0df-6554-4987-853a-c9e482f9ae84.png` 可見黃色寬橢圓、中央黑色菱形小核心，水平／垂直黑線以核心為中心，視覺上分成四個區域；Power `codex-clipboard-95624e63-322b-43ad-9f75-2098c8f94aa4.png` 可見顯著較小的黃色 circle 與中央準星。這些是直接觀察，不能證明四區是原遊戲的正式判定規則，也不能推出完整公式。

### Intent center 與方向保留

中央核心記為 **reticle intent center / sweet-spot intent center**：Player Aim space 的理想中心候選。它不等於 3D physical bat segment sweet spot；兩者未來可以協調，但不得直接視為同一概念。

Normal ellipse 除了 authorization region，也可能提供 2D batting-intent coordinate。`(ex, ey)=(+0.5, 0)` 與 `(-0.5, 0)` 都有 q=0.25，方向卻相反；因此只保存 q 會不可逆地丟失方向。原始 signed error、normalized vector 與 aim center／mode 都須在概念上保留；這不是本輪定義正式 struct 或 production formula。

Authorization 消費 intent 決定是否允許 attempt，但整條資料流不能只剩 authorized／rejected 或 q。Resolution 不應只收到 q 而丟失方向；Quality／Response 未來可選擇消費 signed aim intent，目前不定公式或方向權重。既有 normalized table 僅比較偏離程度，不描述完整 intent 或 directional outcome。

### Hypothesis：Michael 提出的 directional intent

Michael 的強烈設計假說是：上方 aim error 可能增加 fly／lift tendency，下方可能增加 ground tendency，左右 signed error 可能影響 spray direction。**尚未查證為原遊戲 mechanic，也未成為 Pawapuro response 規則。** 靜態圖不能決定 exact field mapping、handedness mirror、launch-angle formula，或證明四象限各有獨立判定。

沿用本文件 error 為 relevant pitch point 相對 aim center 的描述；未來測試仍須明示這個差向量與「玩家把準星移向哪裡」的反向關係，不能混用兩種正負號。正式 gameplay direction 不採 screen left／screen right；batting／contact plane basis、batter handedness、pull／opposite-field 等 body-relative 或 field-relative 語意尚待定義。目前只保留 signed horizontal／vertical components，不決定 field mapping。

Normal／Power 第一版可共用同一類 Player Aim space 與上述資訊保留責任，各 mode 使用不同 region／correction budget。不能推定兩者 quadrant response 相同、Power 中心以外沒有 quality gradient，或小 circle 就是 physical bat size。

### Future testable questions

1. Horizontal signed aim 是否控制 spray tendency？
2. Vertical signed aim 是否控制 ground／fly tendency？
3. Mapping 是否隨 batter handedness mirror？
4. Normal／Power 是否共享同一 directional mapping？
5. Reticle intent center 與 physical sweet spot 應如何協調？
6. Aim intent 與 actual 3D contact normal 衝突時，誰有多少 authority？

以上等待 future caller 與可對照證據，本輪不回答、不新增 Motion Rule。此次 amendment 僅修改本文件並檢查 diff；未實作 input、UI、timing、collision、correction、quality 或 response，未執行 C++／GPU 測試。

## 小型 normalized table（示意，不是 balance）

以下 q 都相對**各自 mode 的 ellipse**；同 q 不代表同一 world aim error。對同一 dx／dy，較小 Power region 通常得到較大 q，可能已超出範圍。表內 pressure 僅表達候選設計期待，並非 solver 實測、budget 消耗量或正式分段門檻。

| q | Normal authorized? | Power authorized? | nominal assist pressure（兩者示意） | quality potential（兩者示意） |
|---|---|---|---|---|
| 0.0 | 是，仍需合法 solution | 是，仍需合法 solution | low | unconstrained by aim |
| 0.25 | 是 | 是 | low | reduced |
| 0.5 | 是 | 是 | medium | reduced |
| 0.75 | 是 | 是 | high | reduced |
| 0.95 | 是 | 是 | high | reduced；可表達 defensive attempt |
| 1.05 | 否 | 否 | 不適用，不啟動修正 | 不適用 |

Power 的 budget 較少，因此同為 high pressure 不代表同樣救得回來；也不暗設另一個 q cutoff。中心 unconstrained 只表示 aim 不先限制潛力，絕非保證 Perfect。實際 correction 仍取決於姿勢、路徑及時序，不能由 q 單獨算出。

高 Contact 可使同一 spatial mistake 落在較大的 Normal region 內；若在邊緣授權且找到可信 solution，仍可能只有 defensive／weak／foul 結果。**Foul 可以是成功避免 miss**，不能把所有 outcome 壓成 Miss < Weak < Normal < Perfect 的單一直線；保留 Defensive / Foul Contact 語意，不實作其判定，也不承諾 edge 一定 foul。

## 有限 correction 與物理層

未來可以研究小量 phase correction、同一路徑的合法 contact time 或 barrel segment solution，但必須有明確上限並保持 presentation 可信。選 time／u 是找真正相交的解，不是任選數值、snap bat、stretch arm、move ball 或 teleport。本輪不改 animation、不做 solver，也不授權 runtime time scaling。

目前只有單一 swing path；**phase 選擇無法保證覆蓋整個 2D ellipse**。這是後續 caller 的可達性問題，不能用擴半徑消除。需要哪種受限 presentation 調整及失敗處理，須另案 review，現在不預建 IK、clips 或框架。

Player timing judgment 是獨立 gameplay dimension。S2 的 physical Contact／NoContact phase 邊界不是 Perfect／Good／Late input window；correction 的 phase budget 也不等於玩家 timing 寬容度。

未來 quality 可讀：signed aim error (dx, dy)／(ex, ey)、normalized magnitude q、correction used、segment u、contact normal、normal closing speed、relative velocity、eventual player timing error。本輪不合成分數、不給權重；normal.y>0、interior u 或高速本身都不宣告高品質。

## 最小 Data／owner 提案與未決事項

只需 batting-specific 概念：`BatterAbility { contact, power }`、`SwingMode { Normal, Power }`、derived `AuthorizationRegion { rx, ry }` 與 `CorrectionBudget`。能力尺度、兩 mode 的 derived mapping、budget 維度與單位尚未決定；不新增 production TOML、stats system、modifier stack 或 reflection。

未來 Pawapuro Native 擁有可 replay 的 authorization／resolution 判定；Data 擁有經 caller 證明需要的 tuning，Engine 不擁有棒球能力語意。Intent 值及決策結果由該次打擊 owner 保存到其結果／replay 使用者完成；solver scratch 只活於單次查詢，不改現有 animation／simulation owner。這是 ownership 提案，尚未新增資料結構或 sequencing。

真正 player-input caller 出現後才能定：aim space 與投影／crossing sample、何時鎖定 intent、swing commit／timing error 定義、能力尺度及 rx／ry、Power region 的 Contact 關係、修正上限與可達性、無解或未授權 overlap 的一致演出、quality／defensive outcome 的消費方式。Ball Response 數值另案決定。此次 review 只確認責任邊界，不用先決定 balance。

## Player Aim S0 — Interactive Normal-Swing Reticle（待 human review）

S0／S0.1 的責任分層保持不變；這次只將 Player Aim 做成實際 app development input，沒有 production Hit Authorization、hit／miss、Power toggle 或 contact response。正式 player timing／swing commit／cursor lock 仍未設計，cursor input 尚未記入 authoritative replay。

- **Owner／座標**：app 擁有 concrete [PlayerAim](../../pawapuro/batting/player_aim.hpp)，生命週期到 app 結束；與 `BattingPreview` 並列，沒有餵入 contact detector。保存 evaluation plane 上 game/world X/Y 公尺座標，不保存 screen pixels；現有 `project_batting_point` 測試確認 +X 向畫面右、+Y 向上（pixel Y 減少）。Engine 不知道 aim 語意。
- **初始與 clamp**：startup=(0, 0.775) m，由 zone 中心決定，不追蹤 prediction。Center clamp 至 X=[−0.4318,+0.4318]、Y=[0.30,1.25] m；角落 ellipse 可超出 outline。這是首版候選，不是永久禁止 off-zone aim。Space replay 保留 aim，R 才 recenter。
- **Data 候選**：[staging.toml](../../pawapuro/batting/staging.toml) `[player_aim]` 的 `normal_radius_x_m=0.26`、`normal_radius_y_m=0.13`、`cursor_speed_mps=0.65`。這是 visual／development control tuning，**不是 authorization balance**。有限正值、半徑小於 zone 對應整體尺寸；loader 另有防誤植範圍並報來源／key。未加入 stats 或倍率。
- **操作**：held arrows 連續移動、diagonal normalized；R recenter。獨立使用 app frame elapsed，單次上限 50 ms、無 debt，不綁 240 Hz preview tick。Ready／Paused／Complete 皆可移動；失焦不移動，focus／restore 重設 input elapsed baseline，minimize 期間不移動。Space／P／`.`／Esc 原行為保留，Pause 只停 preview。
- **Diagnostics**：window title 顯示 aim、`dx=predicted.x−aim.x`、`dy=predicted.y−aim.y`、ex／ey／q。Prediction 只供比較，不吸附、不設 gate，不逐 frame 寫 stderr。左上例 aim=(−0.4318,1.25)，predicted≈(−0.00000045,0.77500063)，error≈(+0.43179956,−0.47499937)，normalized≈(+1.66076756,−3.65384150)，q≈16.1087055；**仍不判 miss**。
- **Rendering／lifetime**：32-segment yellow ring＋black cross＋small diamond 共 210 vertices，append 到原 combined dynamic vector；預先 reserve 總 capacity=26322，不逐 frame 配置。Gameplay truth 在 evaluation plane；ring 向固定 camera 側 −Z 偏 2 mm，cross／core 再各偏 1 mm，皆只改 rendering。App vector 活到退出，renderer 只在 draw 借用並 copy 到既有 fence 保護的 upload buffer；Engine／D3D12View／HLSL 未改。

### 實測與 review evidence

`build/player-aim-s0/` 是忽略目錄：

- `debug-startup.png`、`debug-top-left.png`、`debug-bottom-right.png`：實際 1920×1080 app screenshots；已檢視。`aim-diagnostic.png` 是左上 screenshot 裁切加上比較點／數值標註，不是新增 runtime UI。
- `player-aim-1x.mp4`：6.5 s、30 fps container／195 frames。實際 app wall-time capture 含 Ready 移動、開始 preview、投球途中移動及 Complete；78 個實際 samples 依 timestamp 最近鄰補格，**約 12 captures/s，不是原生 30 fps 錄影**，最大 sample/time 差約 73 ms，沒有拉伸動畫時間。`video-capture.json` 保存 timestamps／titles，`video-validation.json` 記錄實際 decode FPS。已檢視抽樣 sequence，不宣稱已正常速度自看影片；可閱讀 input／preview 關係，快速 swing 平滑度應以 app human review 為準。
- Computer-use Node 初始化兩次皆以 `trusted Node process exited unexpectedly` 失敗；沿用既有 Win32 app test harness。最初測試未取得 foreground，已補明確 focus assertion 再驗證，沒有因此修改遊戲 input。較快 BitBlt capture 回傳空白，未作證據，保留成功的 PrintWindow 影片。
- **Debug／Release build、各 12/12 CTest 通過**。新增 tests：startup、四向／projection、diagonal、四邊 clamp、recenter、dt cap、signed error／normalization／q、invalid tuning、固定 geometry count／capacity。Center／左上／右下各完整 896 ticks 的 path／velocity、投打 poses、contact existence／time／u／normal 與 baseline exact 相同。原 20 replay、30／60／120 chunking、arrival／prediction、PitchDelivery／Batter／Contact S0–S2 tests 保留。
- **實際 Debug app**：move／recenter／start／pause／paused move／step／Complete／replay 通過；replay 不重設 aim。Minimize 中 held arrow 不移動，restore 後 aim／paused tick 不變。`debug-smoke.json` 保存各狀態；`debug.log` 顯示 GPU validation=0 errors、exit=0、shutdown 僅剩供報告用 device，沒有新增 live child resource。Combined assembly mean≈36.124 µs（含原 characters，不是 reticle-only benchmark）。
- **已知 review 點**：world-plane ring 與 corner 可見，未發現需要改 overlay renderer 的 blocker；black core／cross 在深草地對比偏低，中心還有既有 orange prediction ring。是否需要更清楚的核心、尺寸／速度是否適合、球與 reticle 同時閱讀是否足夠，交由 Michael＋Julia 實際操作判斷，不宣告 human acceptance。

## 保留的 physical truth 與 S0 design-study 驗證

- S0 continuous closest approach 保留；moving ball／barrel 不能只測離散 frame。
- S1 sphere／capsule earliest-entry event 保留；ball37／bat33 mm、endpoint 合法性、sub-tick time、normal、relative motion 與原球路皆不改。
- S2 phase map 保留；phase 改變 u／normal／relative velocity，超前側到 +60 ms 仍 Contact、邊界未知。不重跑 study，不推導 input window。詳細數字仍由 [batter README](../../pawapuro/batting/batter/README.md#bat-contact-s2--timing-to-contact-geometry-map2026-09-17) 擁有。
- 本輪只改 Markdown；檢查 diff、連結、來源 hashes 與 evidence metadata。未跑 C++ build／CTest／GPU，也沒有 production 行為變更。沒有新增 Character Motion Rule。

### 可重查 evidence

本機忽略目錄 `build/hit-authorization-s0/`：`video-metadata.json`、`sources.json`、`decode.py`、原解析度抽樣 PNG、`overview.jpg`（全片概覽）、`contact-full.jpg`（f49／52／54／55）、`contact-crop.jpg`（f46–58，局部不含完整游標）。連續 f35–65 PNG 保留；`contact.jpg` 是拼圖產物，但工具未成功顯示該大圖，不將它列為已觀看證據。原始媒體不複製進 repo，以下識別供其他工作階段查核：

- `ref-batting-contact.mp4`：11381641 bytes，SHA256 `544808db37957d5525e44ae5c937b9bc4314ad451665bcd50ab288d93aa2ae7c`。
- `pawapuro-batting-ref-contact.png`：3885175 bytes，SHA256 `05f94daf09987a325b3b61dc3f091de99613716a8fd3b0dcdb9807c2f5a90368`。
- `pawapuro-batting-ref-power.png`：3963638 bytes，SHA256 `6f6d361eb317866d8002c535178bc95e17e7bd601949416dde1e9ee8b1d18a8f`。


<a id="player-swing-s0-gate-c"></a>

## Player Swing S0／Gate C — Manual Native runtime candidate

2026-09-17；Player Aim S0 與 Gate B motion 的 human acceptance 依 Michael 最新決策沿用；上方 S0 舊記錄保留。Gate C **待 Michael＋Julia human review**，不是 production Hit Authorization。責任仍為 Player Intent → Hit Authorization → Physical Resolution → Contact Quality → Ball Response，本輪只實作 development swing intent／播放。

- App 預設 [ManualSwingPreview](../../pawapuro/batting/manual_swing.hpp)，擁有 attempt、唯一 240 Hz clock、pending command 與 committed state。舊 `BattingPreview` 只供既有 choreography／contact regression，manual 路徑不呼叫舊 contact query、不保留舊 contact event。Contact 明示 **NotEvaluated**，不是 NoContact 或 Miss。
- J 使用新的 press edge，每球最多一次；queued command 不可被後一次覆寫。Development target domain 為 preview ticks **432–496（含端點）**，亦即 delivery-relative 1.800000–2.066667 s。全部 65 ticks 可提交；不 clamp／round 到三個 fixtures，不將區間外按下留到開窗。這不是 Perfect／Good／Late、production timing window 或 Contact stat。
- App frame 順序：先 account 舊 state 的 elapsed，最多消費 16 個 simulation ticks 並保留 backlog；再處理 SDL events／state transitions；更新 live aim／R；於該 frame input boundary 取 committed aim center snapshot；排程 `target = current tick + remaining backlog + 1`；後續 simulation 只在該 target 消費。新輸入不回填舊 elapsed debt。Diagnostics 記錄 boundary／target／consumed tick 與 backlog；例：tick420 加入100 ms，先走至436、剩8 ticks，J 排到445，非437。
- Command 保存固定 Normal mode、target／consumed tick、boundary／backlog 與 immutable aim center；motion SHA／recipe 由 attempt owner 的 immutable asset 提供，owner 另保留 const startup `BattingStaging` Data snapshot；git version 固定 Native 配方。Replay authoritative input 是 recorded tick／snapshot／Data／assets，不保證不同 render cadence 的原始 OS events 自然相同。
- Live cursor 仍可移動／R，跨下一球保留；不改 committed snapshot，不升為 production cursor lock。Title 的 signed dx／dy、ex／ey、q 是**目前 live center 對固定 predicted arrival point** 的 frame-boundary diagnostic，沒有將 q 當作完整 intent，也不拿它改 bat path、球路、physical contact 或 hit／miss。
- Ready／Paused／Complete、失焦或 minimize 不接受新 swing；pause／focus loss 清除 pending，但 committed 保留。恢復資格後 held J 必須先鬆開；Windows app 在 boundary 讀 physical J state，避免 SDL focus-loss 清除 keyboard state 造成重新武裝。Held／repeat 不產生新 edge；新球清除舊 pending／commit／狀態。P／`.` 保留同一 timeline。
- 不按可完成 Take＋pitcher finish（tick816）；commit 後的 finish 為 `commit+456 ticks`，最晚合法496完整播到952，Complete 後 Space 開下一球。主要 sweep phase 為 commit+24 ticks、contact-area pass cue 為+40 ticks；不是實際 collision 時刻或實測 input latency。
- Pitcher／release／固定 trajectory／prediction／arrival 語意不變。球在 arrival 後仍 frozen，title 明示限制；不以凍結球與晚揮製造接觸、不做 hit sound／flash／foul／Quality／Ball Response。

資產與 render／bat semantic pose 使用同一 `IngameMotion` evaluation；fractional time 不截為整數 authoring frame。重建 local-pose hierarchy 的普通函式留在 Engine，preparation、residual、foot support 留在 Pawapuro。具體匯出、65-tick／saved-source／input／GPU 檢查與 capture 限制見 [ingame_s0 README](../../pawapuro/batting/batter/ingame_s0/README.md#gate-c-runtime-candidate)。新動作的 continuous-contact coverage 刻意留待下一個另行授權的 slice；既有 S0–S2 regression 不刪除、不放寬容差。

## Arrival Cue S0：release reveal 試玩原型（2026-09-18）

操作模型為「看投手準備 → release 取得預期進壘位置 → 注意力移到打擊區 → 用實球接近判斷出棒」。Arrival Cue 表示固定 predicted evaluation-plane point，Actual ball／BallAid 表示當前球心，reticle 表示玩家意圖。提示僅由既有 `PitchPhase::InFlight` 揭露：Hand→Simulation 的 release 同次狀態轉換後，下次正常 render 即可見，不另加 timer／tick；Complete／reset 隱藏，pause 保留當下可見性，single-step 跟隨同一 simulation 狀態，與 J／commit 無關。移除常駐橘圈；**V** 比較預設 **Filled Baseball**（近白色不透明填色／細外輪廓／靜態紅縫線）與 **Ring**（原橘圈），共用 prediction、投影尺寸及揭露規則。縫線只有外觀，沒有球種／spin 語意；S0.1（2026-09-18）依 Michael 試玩回饋改為實心，中心與尺寸不變；Filled Baseball 模式，先畫提示，再畫既有 depth-tested 實球與準星，白底不覆蓋準星黑色 cross／core，也不靠 BallAid 才能看見靠近的實球。BallAid 維持原樣並最後繪製。Title 顯示外觀與 Hidden／Visible，隱藏時以 Hidden 取代 prediction-derived error／ex／ey／q，內部 signed diagnostic／snapshot 不變；**B** 仍切換預設 ON 的 BallAid。固定紅中只能驗證提示與會合感，尚未驗證未知落點難度；Michael 已確認 S0 進壘位置更清楚；S0.1 實心外觀與重疊辨識待 Michael＋Julia playtest review，Contact 仍 **NotEvaluated**，重疊不代表 Hit／Perfect。Flight Complete 隱藏是本 prototype 決定，不推論原作規則。


## Manual Swing Contact S0：固定來球 geometry feedback（2026-09-18）

Michael 已確認 presentation cleanup（移除 release 藍圈／直線與黃尺、紅色好球帶）使畫面更乾淨、球路較易辨識；本輪保留其視覺。以下取代 Gate C manual caller 的 Contact:NotEvaluated 邊界，**新 manual geometry candidate 待 Michael＋Julia review**，不代表 Hit Authorization／gameplay acceptance。

- `ManualSwingPreview` 以實際 consumed commit tick `c` 查詢 **[c, c+64] ticks**（commit 後 266.667 ms），逐個完成的 240 Hz interval 查 earliest entry；包含 commit 接縫、6 authoring frames entry／support、主要 sweep 與初段收勢。這是目前固定球路的有界 diagnostic：最早入口查到 world496 時解析球已越過 evaluation plane，最晚入口查到560；不照搬舊472–488，也不把 marker±8 當保證。後續收勢與 commit 前均不屬於已查詢範圍。
- `IngameMotion::sample_barrel` 使用 render 同一 `sample`，保留 fractional ticks／quarter-frame local interpolation、entry residual 與 support；game basis／placement 與 runtime semantics 一致。Caller 持有並重用獨立 pose scratch，不改播放 pose、clock 或 simulation，也不在 substep 載入資產。兩個具體 caller 共用既有 sphere/capsule earliest-entry、24 次 bisection 及 surface material-point velocity，正式64 substeps/tick；物理半徑仍 ball37／bat33 mm，與視覺無關。速度以事件前後0.1 ms唯讀取樣求得，不增加查詢事件範圍。
- 球永遠從 immutable initial state 與 release time 取解析 reference path；evaluation plane 之後是 **geometry diagnostic 的解析延伸**，不是畫面上的 frozen ball。Mutable 球路、prediction、arrival、動畫、aim／snapshot、J domain均不變，aim 不作 authorization gate；沒有 Ball Response。
- Title 前段顯示 `Geometry:Pending / No ball response`、`NoSwing`、`Contact` 或 `NoContactInWindow`。沒有 commit 且合法 domain 結束才確定 NoSwing；Contact 一球最多發布一次，於包含事件的已完成 simulation tick 才 dispatch；無接觸在 c+64 才確定，僅代表此區間未接觸。結果保留到 reset；pause 不另走時間、step 跟隨同一 clock。結果 log 包含 commit／事件時間／dispatch／segment u／normal／relative velocity 與查詢範圍。
- Release 數值檢查：65個合法 commit 的32／64／128 refinement 接觸有無一致，事件時間差 <0.1 µs。**432–442 Contact（11例）**，443–496 NoContactInWindow。例：432→world475.736 tick（1.982233556 s，dispatch476）；442→479.770（dispatch480，已超過 arrival，屬解析延伸）；443 最近的1/8-tick grid centerline distance約79.625 mm、456約496.664 mm，與70 mm合併 envelope比較。Grid minimum 是取樣值，不宣稱全域精確最小值。
- 604個既存 saved `.blend` bat semantic samples（含fractional entry）最大位置誤差4.77 µm，沿用0.1 mm fixture tolerance；亦檢查 sampler/render、唯讀性、Take、一次事件、reset、pause/step、Contact／NoContact 的30/60/120 Hz與backlog replay、interval split／初始overlap邊界。舊 S0–S2 使用原 BatterMotion／window，regression與容差保留。完整任意動作 CCD、未查詢時段、production authorization、quality／foul／fair／球速角度旋轉與 response 均未完成；手感、效能與畫面會合感未由數值測試驗收。

可重現摘要由 `manual_swing_test` 輸出每個 commit 的結果／contact tick／grid minimum；本機精簡結果位於 ignored `build/manual-contact-s0/`，不依賴其內容才能重跑。

### In-Game Contact Result Panel S0（2026-09-18）

左上「本球結果」固定列出「未出棒／碰到球／未測到碰球」，直接對應 owner 的 NoSwing／Contact／NoContactInWindow；僅確定結果使用亮底與三角指示，其他項目仍清楚顯示。Pending 不選結果：Ready 顯示「按 Space 開始」，開始但未 consumed commit 顯示「等待出棒」，已 commit 顯示「揮棒中」。結果保留至下一球 reset，pause／收勢／B／V 不另設計時。說明為「接觸測試：只檢查出棒後的一小段，球暫時不會飛出去。」若既有 event 的解析球心已越過 evaluation plane，補充「依球繼續前進的位置判斷；畫面中的球仍停住。」不重算碰撞、不新增判定等級或球路。固定中文以系統字型於啟動時光柵化並快取為 triangle geometry，使用既有 dynamic overlay／upload lifetime；未提交字型檔或加入 UI 系統。判定契約不變；Michael 已確認遊戲內結果面板明顯比 window title 清楚，作為目前 development feedback baseline 保留。


### Swing Tempo S0：Original／Compact Entry A/B（2026-09-18）

固定球路與畫面，只比較 committed swing 前段。原 A/B 實驗啟動預設 A；human review 後目前啟動預設 **B 快出棒／Compact**，A 保留為 development comparison／regression reference；**T** 僅於 Ready／Complete 選下一球，**Space** 開始時鎖定模式與 startup `[swing_tempo].compact_area_ticks`（預設30，finite 且 0 < T ≤ 40）。選擇跨球保留；Playing／Paused 不切換。面板保留本球標籤，Complete 另列下一球選擇，不替既有結果換模式。

以真正 consumed commit tick `c`、world offset `x = world_tick − c`，A 原樣取樣；B 在 x > 0 時令 `u = clamp(x/T, 0, 1)`、`sample_tick = c + x + (40 − T)(3u² − 2u³)`。未 commit／x ≤ 0 的 preparation 不變。映射只在 whole-pose／barrel 取樣入口套用一次，原 evaluator 仍以真正 c 計算 entry residual／support，保留 quarter-frame interpolation。球保持 world time，bat surface velocity 的 t±epsilon 各自映射；render、semantics、contact 與 completion 共用 `SwingTempoTiming`。Command snapshot 保留模式／T，log 記錄 `smoothstep-v1`、實際 commit 與推導 cue；replay 需相同 recorded tick、aim、attempt tempo、startup Data 與原資產。

| Commit 後動作 cue | A | B（T=30） |
|---|---:|---:|
| Sweep 起點 | 24 ticks／100 ms | 約17.6761 ticks／73.6504 ms |
| 同一 contact-area pose | 40 ticks／166.667 ms | 30 ticks／125 ms |
| 完整 finish | 456 ticks／1900 ms | 446 ticks／1858.333 ms |

這些是 motion cue，非 actual contact／Perfect 或端到端 input latency。B 在30 ticks 後保持提前10 ticks、以1×繼續；Take 不變。J domain 432–496、world query `[c,c+64]`、球／棒半徑、解析延伸與 Geometry only／No ball response 均不變。真實新 J edge 因 domain 被拒絕時，owner 保留「已按下，但不在本輪可出棒時段。」至下一次有效排程或 reset；這是獨立操作提示，不是第四種結果，也不預約出棒。

Release 集中檢查：A 原 manual/contact regression 通過；B 全65個 commit 查詢完成，**436–452 Contact（17例）**，432–435／453–496 為 NoContactInWindow。例：436→world473.213、452→479.853 ticks；這不是正式 timing window 或命中率。端點與接觸轉換兩側的32／64／128 refinement 一致；代表性入口 whole-pose／fractional semantics 與原 evaluator 的映射後姿勢一致（沿用0.1 mm門檻），包含支撐與 attachment，非完整 mesh collision 驗收。另檢查映射單調／兩端速率、完整 finish、30／60／120 Hz與backlog replay、模式鎖定／保留、拒絕提示／清除與面板顯示。短 app smoke 確認 A→B、Take、Complete 下一球切換不改舊標籤與正常退出；當次 agent 未實測 A/B 出棒手感或端到端 latency；後續 human acceptance 見下段。


**Compact promotion（2026-09-18）**：B 已獲 Michael＋Julia human acceptance，成為目前固定來球／Normal manual swing 的預設 gameplay baseline。Michael 回饋 B 在直覺正確的按鍵節奏下反應自然、能產生合理接觸；切回 A 後下棒前段明顯過慢。接受依據是 gameplay responsiveness／timing feel，不是 B 的接觸 commit 數量較多。本輪沒有調慢球速或改球路，回饋支持原揮擊前段過慢是先前操作困難的重要因素之一。125 ms contact-area cue 是目前 accepted tuning，未來仍可依新 gameplay evidence 調整，不是永久 production balance；完整 end-to-end input latency 仍未量測。Contact 仍為 bounded geometry feedback，不等於正式 Hit Authorization／Contact Quality／Ball Response。只改 manual entry 的預設選擇，底層 `SwingTempoTiming{}` 仍為 A；T、snapshot、mapping 與所有既有數值不變。

## Hit Authorization S1：Normal spatial gate（2026-09-18）

本節取代上方歷史 S0／Gate C「尚無 authorization」的 runtime 狀態；保留其研究與已接受行為。S1 是最小空間授權，待 Michael＋Julia review，不是最終 Hit／Miss。

- **Data ownership**：`HitAuthorizationTuning`／`[hit_authorization]` 擁有 `normal_radius_x_m=0.26`、`normal_radius_y_m=0.13`。這是 development tuning baseline，不是永久 balance。沿用原 finite／range／zone extent startup validation；`PlayerAimTuning` 只保留 cursor speed。準星 ellipse 與 live signed diagnostics 消費相同半徑，沒有第二份 visual ellipse。
- **決策時點／truth**：queued `SwingCommand` 真正 consumed 時，以不可變 `aim_center` 為 A；P 使用 `predict_arrival(delivery.pitch).state.position_m` 的 X/Y。它由 immutable initial pitch、既有 240 Hz integration／crossing interpolation 與 `strike_zone_plane_z()` 產生，與 Arrival Cue／prediction 同源；不使用 freeze 後球心、解析 contact sample、visual radius、bat capsule、u 或 live reticle。
- **公式**：`dx=P.x-A.x`、`dy=P.y-A.y`；`ex=dx/rx`、`ey=dy/ry`；`q=ex*ex+ey*ey`。`q<=1` 為 Authorized，`q==1` 包含在內，不加 gameplay epsilon。決策保留 P、signed error、normalized error、q 與 authorized。
- **生命週期／replay**：`ManualSwingPreview::authorization` 是 optional per-command value，消費前不存在，消費當下只算一次。移動 live aim、pause／step 不重算；Complete 保留，reset／新球清除。Owner 不依賴 PlayerAim object；同一 recorded command snapshot、startup Data、Native／motion versions 與 pitch truth 支援 deterministic replay，沒有承諾跨平台 bit identity。
- **兩種 truth**：授權 Pending／Authorized／RejectedSpatial 與原 `ManualGeometry` Pending／NoSwing／Contact／NoContactInWindow 獨立保存。RejectedSpatial 不 short-circuit query，不改 BatContact、bat path 或 ball path。**RejectedSpatial + Contact** 是合法且刻意保留的開發觀察，physical overlap 不自動等於 accepted gameplay hit；**Authorized + NoContactInWindow** 也成立，permission 不保證 physical contact。
- **畫面**：既有 fixed-text cached panel 新增「瞄準授權：等待／通過／超出範圍」，與「本球結果」三列分開；拒絕授權不是第四種 geometry 結果。沒有 correction、Contact ability/stat、quality、sweet spot、foul／fair、Ball Response 或最終 Hit／Miss。

### 可重現 fixtures 與人工 review

`hit_authorization_test` 使用正式 Compact（T=30）及固定 pitch：

| Commit tick | Immutable aim A（m） | 授權 | Raw geometry |
|---|---|---|---|
| 441 | P≈(−0.000000449712, 0.775000631809) | Authorized，q=0 | Contact |
| 441 | (P.x + 0.26×1.01, P.y)≈(0.262599527836, 0.775000631809) | RejectedSpatial，q≈1.02009999752 | Contact |
| 456 | P | Authorized，q=0 | NoContactInWindow |

441 pair 的 time=1.98058356422 s、u=0.479573283832、normal≈(0.328227907419, 0.290680110455, 0.898761093616)、relative velocity≈(−10.745803833, −8.63688278198, −64.9047622681) m/s **逐值完全相同**，比既有 tolerance 更嚴。測試逐 tick 比較 whole bat pose／barrel transform、ball position／velocity、geometry existence，並比較 contact time／u／normal／relative velocity／surface point／bat velocity／radii。Ball37／bat33 mm 及 physical solver 未改。

Focused tests 另含中心、兩軸精確 q=1、略超界、signed vectors、真正 PlayerAim 在 schedule／commit 後移動、reset／新球／Complete、pause／step、30／60／120 Hz／backlog、tick420+100 ms→boundary436/backlog8→consumed445；也涵蓋 RejectedSpatial + NoContactInWindow、Pending authorization + NoSwing，以及面板獨立顯示。既有 tests／tolerances 保留。

本機 ignored evidence 在 `build/hit-authorization-s1/`。實際 Release app screenshot `authorized-contact.png` 顯示「通過＋碰到球」，本次 OS input consumed **444**（不是數值 fixture 441），aim=(0,0.775)。`authorized-contact.json` 保存 window title。畫面已檢視，非合成圖。Computer Use 的短按方向鍵沒有讓 app 的 held-key aim 取樣移動，因此未取得可信的 RejectedSpatial + Contact screenshot；沒有以 mock screen 補作。部分未 foreground 的 capture 得到遮擋視窗，已以 foreground capture 覆寫，不列作 evidence。

人工操作：預設 B；R 回中央，Space 開始，約開始後 1.84 s 按 J（以 title 的 consumed commit 為準，Compact 436–452 目前會 Contact）。下一球前按住右鍵讓 aim.x≈0.263～0.30 m、aim.y≈0.775 m，再以同樣節奏揮棒，應同時看見「超出範圍」與「碰到球」。精確 commit 441／456 由 Native test 重播；一般 OS 手按不保證同一 tick。P 暫停、`.` 單步保留原控制，paused 不接受新 J。截圖與測試不代表 human feel acceptance 或端到端 latency 測量。

### 建置與執行紀錄

使用 agent 自身 `exec_command`／PowerShell、cwd `C:\astra-dev\pawapuro` 核對 clean main／HEAD；fetch 後 origin/main 仍為 `6a482dc041f230ff44409276c3899defccc6a0f2`。首次 git fetch/pull 在 `.git/FETCH_HEAD` 遇到 sandbox `Permission denied`，單次 per-command 核准 fetch 成功；未改權限或歷史。

完整 Debug／Release build 使用既有 VS 2022 x64 developer environment、UTF-8、CMake/Ninja，沒有安裝／修復環境。首輪 compiler C2039 是 staging test 欄位遷移時多寫一層 `player_aim`，已修正；首輪 authorization test 的 Take fixture 在仍 Playing 時呼叫 start，已補 reset。兩者是正常 compiler／test failure，不是 runner 問題，也沒有放寬測試容差。

Release 完整 CTest **17/17 通過**（約 41 s）。Debug 完整 suite 約 583 s，既有 16 項通過；新測試的上述失敗修正後，authorization／panel／staging 直接執行均通過，並以 CTest 重跑這三項，**3/3 通過**（36.06 s）；合併結果涵蓋全部 17 項。`debug-ctest.log` 保留首輪失敗，`debug-final-ctest.log` 記錄修正後結果；`debug-final-build.log`／`release-build.log` 保存建置。實際 Release app 已啟動、從 Ready 完成 Compact swing、確認新 panel 的 Waiting／Authorized 與 Contact／Complete 保留狀態，並透過 Esc 關閉；未宣稱本輪有 Debug GPU validation 或 human playtest acceptance。
