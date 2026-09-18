# Hit Authorization S0 — Gameplay Contact Model

最新 runtime 狀態見文末 **Timing Interaction S1** 與 **Follow-through Compression S1**；Hit Authorization S1 已獲 human acceptance，下方 S0／Gate C 等歷史記錄保留。

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

## Timing Interaction S1（2026-09-18，待 Michael＋Julia review）

**Hit Authorization S1 已獲 human acceptance**：Michael 手動確認正確時機＋正確 aim、正確時機＋RejectedSpatial，以及正確 aim＋晚揮可未接觸。以下更新取代歷史段落的 runtime input／query domain，保留其 regression evidence。責任順序現在是 Player Swing Intent → Temporal Interaction → Spatial Authorization → Physical Resolution → Contact Quality → Ball Response；本輪只到前四層，沒有最終 Hit／Miss。

### 輸入與具體動作延伸

舊 `[432,496]` 是 development input／motion domain，不是正式 timing gameplay。Playing、focused／eligible、未暫停、已 rearm 且尚無 queued／committed swing 時，fresh J edge 依既有 `tick + pending_ticks + 1` 排程，抵達該 authoritative tick 立即 consumed。沒有等 release、buffer 到好時機或 clamp；一球一次。Ready／Paused／Complete、focus loss、held-key rearm 與 backlog 邊界規則保留。NoSwing 只在整個 attempt 完成且從未 committed 時確定，tick496 不再結案。極早揮棒可以先完成，投手與球仍繼續。

`IngameMotion` 保留 preparation、六個 source frames 的位置／旋轉 entry residual、rear-toe support、bat attachment、whole-pose／barrel 共用取樣、Compact remap 與完整 finish。小幅延伸為：在 source commit frame109 之前，front-foot plant 使用 commit＋6 frames，避免把舊 plant 公式外插成漫長懸空；109 之後原公式完全保留。支撐由既有 front-foot lift／descent 與 rear-toe anchor 解決，body intent／lead-lag 仍由原 authored swing 與 entry residual 銜接，momentum 仍由原完整 follow-through 收束；未重作官方動作、未加 IK／animation graph。

標準無出棒 attempt 的可 consumed 範圍為 **tick1–816**（release384），排在 Playing input boundary 的 backlog command 也可在之後消費，測試含820；runtime 不另設 gameplay commit 上限，僅防止無效／溢位 tick。Motion tests 額外覆蓋1、96、240、360、431、497、600、815、816、840，原432–496全部 fixtures／容差與 A／B mapping regression 保留。這不是任意巨大 tick 的精度保證。完整 Compact finish 仍為 commit＋446 ticks；follow-through 感覺稍長是已知調整項，本輪刻意不縮短。

### Data、時間與物理責任

- **Batting Interaction Region**：`[batting_interaction].half_depth_m=0.40`，以既有 evaluation plane 為中心的 Z-only slab，X／Y 完全不限制；它不是 strike zone、第二個 aim gate 或 physical reach。固定 pitch 目前 Z 速度為常數，以 immutable initial Z／velocity 推導兩面 crossing world time，不寫死 ball ticks。速度不同便有不同 duration。當前 plane Z=0.4318 m、slab Z=[0.0318,0.8318] m；entry=1.983233717173 s、exit=2.002433563945 s，duration=**19.199846772 ms**。Reference time=1.99283345044 s，仍來自既有 `predict_arrival` plane crossing。
- **Normal Swing Phase Potential**：`[swing_phase_potential]` 的 start／peak／end 為75／125／190 ms，全部以 consumed commit C 為相對起點。兩側使用 `smoothstep(u)=u²(3−2u)`，上升 u=(tau−start)/(peak−start)，下降 u=(end−tau)/(end−peak)；start 以前及 end 以後為0、peak為1。唯一有效 support 是 **S(tau)>0**，沒有另一份 contact-capable interval Data。Startup 檢查 finite、範圍與 start<peak<end。這些與 slab 深度都是 Michael 後續 playtest 的 development candidate，不是永久 balance。
- **Temporal overlap**：ball passage 與 `[C+start,C+end]` 相交；僅有端點相等不算 overlap。空交集仍播完整 swing，分為 Early／Late，不是 input rejection。Consumed 時以 deterministic pitch／Data 計算一次，pause／cadence 不重算，Complete 保留、reset 清除。
- **Timing transfer**：`timing_efficiency = integral(S(t−C), B_enter..B_exit) / (B_exit−B_enter)`，以 smoothstep 的精確 polynomial primitive `u³(1−u/2)` 在 peak 分段積分，避免 frame-dependent accumulation；只 clamp 浮點誤差至[0,1]，不設第二個品質門檻。`timing_offset_ms=(C+peak−ball_reference_time)*1000`，負為 power peak 早、正為晚。這是 deterministic development signal，尚未變成 Contact Quality 或球速。
- **Spatial Authorization**：沿用 immutable aim snapshot、prediction plane P、同一 .26／.13 m ellipse、signed error／q 與 q<=1，沒有把 live reticle 或 physical contact 偷換成 P。即使時間空交集仍保留此診斷。
- **Physical Resolution**：runtime 只在 temporal intersection 的已完成 fixed-tick 部分執行既有 continuous earliest-entry sphere／capsule solver；S=0 端點以相鄰 representable time 朝內處理，不加 gameplay epsilon。空交集不呼叫 solver；找到首次事件即停止後續搜尋。沒有 pre-release negative-time ghost pitch，也不搜尋球離開 slab 以後的位置。RejectedSpatial 不阻擋 raw geometry 診斷。歷史 `[commit,commit+64]` 僅留下作 offline regression；ball37／bat33 mm 與 solver 未改。Follow-through S=0 刻意不支援 batting interaction；現實偶發的非正常揮擊碰球不在此模型內。

### 畫面一致性與可重現檢查

Manual preview 的球在 slab entry 起使用與 contact 相同的 analytic trajectory，繼續通過 evaluation plane，於 slab rear edge 停住；這是小幅 presentation continuation，不改 ReferencePitch／PitchDelivery integration、release、原球路或 response。固定 pitch 的 Compact451／452 事件在 plane 後，現在位於實際顯示球路上，沒有「解析球已往後、畫面球卻停在 plane」的舊矛盾。Arrival Cue／BallAid 的原 phase gating 保留；pitch simulation 的 Complete 仍以 plane 為準，只有 manual displayed ball 繼續到 slab exit。此處不是一般化 pitch lifecycle，也未模擬 contact 後球路；完整 rear-slab 畫面可讀性仍待 human review。

面板獨立顯示「揮棒時機：等待／無重疊（早）／有重疊／無重疊（晚）」，保留空間授權列與原三種 geometry 結果。Title／log 有 consumed tick、ball／potential／overlap world seconds、efficiency、signed offset、spatial 與 raw geometry；不是 Perfect／Good／Bad。

固定 pitch、B Compact、aim=P 的 deterministic fixtures（秒；空交集記為「無」）：

| Commit | Potential start／peak／end | Temporal overlap | Efficiency | Offset ms | Raw geometry |
|---:|---|---|---:|---:|---|
| 96 | 0.475／0.525／0.590 | 無，早 | 0 | −1467.833450 | NoContactInWindow，0 queries |
| 433 | 1.879167／1.929167／1.994167 | 1.983234–1.994167 | 0.014755 | −63.666784 | NoContactInWindow |
| 448 | 1.941667／1.991667／2.056667 | 1.983234–2.002434 | 0.974447 | −1.166784 | Contact |
| 460 | 1.991667／2.041667／2.106667 | 1.991667–2.002434 | 0.023204 | +48.833216 | NoContactInWindow |
| 600 | 2.575／2.625／2.690 | 無，晚 | 0 | +632.166550 | NoContactInWindow，0 queries |

**Spatial S1 replacement fixture448**：A=P 與 A=(P.x+0.26×1.01,P.y) 分別 Authorized／RejectedSpatial，兩者 raw Contact data 逐值相同：time=1.9913564701 s、u=0、normal≈(0.101611606777,−0.163544073701,0.981289148331)、relative velocity≈(−1.27030932903,−3.71577167511,−72.503616333) m/s。Aim-only change 的 bat／ball path invariance 仍逐 tick 驗證。456仍為 Authorized＋NoContactInWindow。

**刻意改變的舊 fixture441**：舊 unrestricted contact=1.98058356422 s，早於 slab entry，現在有 temporal overlap 但 NoContactInWindow；test 同時證明原 solver 在舊 domain 仍找到該事件，沒有隱藏分類變化。新 Compact runtime432–496 sweep 為442–452 Contact（11例）；歷史 unrestricted Compact436–452（17例）與 Original432–442（11例）仍全部 regression，既有 refinement／容差保留。

人工 review：保持 B；R 回中央、Space 開始後，分別約 **0.400／1.80417／1.86667／1.91667／2.500 s** 按 J，對應上述五例；以 title 的實際 consumed tick 為準，OS 手按不能保證 exact tick。第一例在 release1.600 s 之前，應立即揮棒，之後球繼續接近本壘。P／`.` 可在出棒後暫停／單步觀察，paused 不接收新 J。要比較 rejected spatial，把 aim.x 移到約0.263–0.30 m、y維持約0.775 m，再按正常節奏出棒。Native tests 負責 exact command replay。

本輪實際 Release app 已啟動，Ready 畫面確認 temporal／spatial 狀態列分開且可讀；接續自動按鍵時 Computer Use 回報該視窗偵測到使用者輸入並中止操作，刷新後 app 仍 Ready、桌面由其他應用程式佔用，因此停止搶用視窗。**未取得可信的 early／normal swing 截圖或影片**，不以 mock／離線圖替代 runtime evidence，也未宣稱手感已驗收。數值測試不能取代 Michael＋Julia 的實機 review。

沒有 Contact ability/stat、correction、snap／magnetism、Contact Quality、sweet spot／barrel-u grading、Power、foul／fair、最終 Hit／Miss、VFX／SFX 或 Ball Response。本輪至此停止。

### 本輪建置與驗證

Agent 自身 PowerShell execution 核對 main／clean baseline，fetch 後 HEAD／origin/main 同為 accepted `b998a4c92c2add62a15b221a79a565a77dca5ce3`；提交前再次 fetch 亦無後續 commit。使用既有 VS 2022 x64／CMake／Ninja，完整 Debug、Release build 均通過。Release 完整 CTest **18/18 通過（35.56 s）**；之後補強舊441明確 regression 與 tuning valid-override 測試，重新 build 並跑受影響兩項 **2/2 通過（1.53 s）**。最終 Debug 完整 CTest **18/18 通過（569.50 s）**。首次 Release test failures 是舊441預期與 interval-split fixture 尚未套用新 temporal intersection，已修正 fixture 契約並保留原 unrestricted solver regression，沒有刪除測試或放寬容差。

本機 ignored `build/timing-interaction-s1/` 保留 `debug-build.log`、`debug-ctest.log`、`debug-details.log`、`release-build.log`、`release-ctest.log`、`release-final-build.log`、`release-final-ctest.log` 與 `timing-fixtures.txt`。既有 physical solver、ReferencePitch／PitchDelivery、SwingTempo mapping、authorization evaluator 與 PlayerAim implementation 沒有變更；未宣稱本輪 GPU validation 或 early／normal 實機 playback 已完成。

## Follow-through Compression S1（2026-09-18）

Michael 已確認 Timing Interaction S1 的極早輸入可用，但post-potential follow-through過長。本輪只在world commit＋190 ms之後壓縮原authored tail，A／Compact前綴與既有Timing Interaction／spatial／raw geometry不變；初始完成候選為＋240 ticks（1000 ms），約675 ms的major silhouette只作visual review target。Reference whiff僅為約略視覺時序，非逆向內部規格。Mapping／支撐／attachment／momentum、regression與evidence由 [batter-motion](batter-motion.md) 擁有。早揮完成後hold，投手／球繼續；仍一球一次，沒有multi-swing／rearm、correction、Contact Quality或Ball Response，待Michael＋Julia的1×human review。

## Multi-Swing Intent S1（2026-09-18）

**Follow-through Compression S1 已由 Michael 人工接受**；約 1.0 秒的 Normal swing 是目前 baseline。本節接續並取代前節「一球一次」與 pitch exit 後仍可接受新輸入的 lifecycle 描述，沒有更改 accepted motion、Timing Interaction、Spatial Authorization 或 physical solver。

### Ownership 與重新出棒

一個 pitch 擁有 `0..N SwingAttempt`，沒有 `max_swings=2`。每個 consumed attempt 各自保存 command／immutable aim snapshot、TimingInteraction、HitAuthorizationDecision、raw geometry、contact event、實際 searched interval、query／contact count 與 dispatch tick。`ManualSwingPreview` 只另外持有一筆 pending command 與 optional active index；latest getters 是既有 UI／診斷的唯讀 view，不是另一份 authoritative state。重置新球才清空歷史；後一次出棒不覆寫前一次。

同時只有一個 active swing。Active 期間的 J 不 interrupt、不 buffer；完整 commit＋240 ticks 後清除 active 與 input armed 狀態。若下一個 simulation tick 仍嚴格早於 `ball_passage.exit_s`，**就在完成的同一 world tick，以既有 uncommitted preparation 路徑重新求 batter pose**。不是靜態 frame-1 Ready，不重設 world tick、投手或球；沒有 recovery clip、blend、IK 或 interpolation。這一刻不額外展示一格終點 hold；前一 tick 接近終點，完成 tick 直接顯示 preparation。若已關閉 live intent，則照原本方式持續 final hold。

Rearm 的 simulation 判斷用 T＋1，避免 render backlog 改變 pose replay；實際 fresh edge 仍依原契約排在 **tick＋pending_ticks＋1**，必須以該 authoritative target 的秒數嚴格小於 passage exit 才能排入。等於／超過 exit 都是 `RejectedPitchClosed`，不是成功 timing gate，也沒有 clamp。當前固定球路 exit=2.002433563945 s，因此最後可 consumed tick 是480；481關閉。480仍能真的揮棒並得到 NoOverlapLate，active swing 可在 exit 後完成。舊600／816／820等輸入 fixture 本輪刻意改為 lifecycle rejection；其純 motion／potential 測試仍保留，432–496的歷史 offline geometry regressions 也保留。

Held J 不會自動連揮，active 期間的按鍵不會留到下一次。完成後須觀察 release，再接受 fresh edge；focus loss／pause 取消 pending 並維持 held-key discipline。Tempo 在 start 時鎖定整球，各次 command 保留同一 A／B／finish Data。`NoSwing` 只代表整球 Complete 時 attempts 數為零；早揮空、rearm 或 latest NoContact 都不等於 NoSwing。

### 每次 pipeline 與 regression

每個 command consumed 時依自己的 C 計算 timing，再以自己的 aim snapshot 與既有 evaluation-plane pitch point 判定 spatial authorization；raw geometry 只搜尋該次 ball passage ∩ S(t−C)>0。RejectedSpatial＋Contact 仍可獨立觀察。75／125／190 ms、efficiency 積分／signed offset、±0.40 m slab、.26／.13 m ellipse、37／33 mm physical radii、continuous earliest-entry solver 與 post-plane presentation continuation 全部未改。

核心 fixture：第一揮80，左側 aim，被 spatial 拒絕且 NoOverlapEarly／NoContact；320完成並回到同 tick preparation（release384前），第二揮448使用新的中央 aim。448–494逐 tick whole world pose、grip／barrel／tip與球位置和 fresh single448完全相同；第二次 timing、efficiency、offset、authorization、geometry、query count、dispatch tick及contact time／u／normal／relative velocity／radii皆逐值相同。Contact time仍 **1.9913564701 s**，efficiency約0.974447，offset約−1.166784 ms；第一揮的 snapshot／決策到整球 Complete 仍未變。新的 live aim 在排程第二揮後再移動，也不影響該 command。

`multi_swing_test` 同時覆蓋 A／Compact、30／60／120 Hz與backlog、held／active press無buffer、focus／pause、cutoff、零揮，以及慢速測試球路自然容納四次出棒。正常448 Contact的finish688已在passage後，沒有需要另加 terminal-hit machinery 的重複候選。原 single-swing、protected-prefix、spatial independence與 solver tests 沒有放寬容差。

### Human review 與 evidence

Cached panel 增加「等待第一次出棒／揮棒中／可再次出棒／出棒機會已結束／出棒已排程」，保留 latest swing 的 timing、authorization與geometry；title有 `swings`、`active`、`rearmed`。暫停時不以「機會已結束」誤標，Paused由title表示。供應的 reference footage **只證明 pitch result 後硬切 Ready，沒有證明同一 live pitch 如何 rearm**，本輪是刻意的 hard live-pitch prototype，也沒有複製 off-balance result pose。

實際 Release app smoke 已得到兩揮：第二次 consumed452，`swings=2 active=2`，Authorized＋Contact；另一次 early80在320後回 preparation，title／面板顯示可再次出棒。真實截圖 `second-contact.png`、`rearmed-live.png` 與title JSON在 ignored `build/multi-swing-s1/`；UI截圖與title讀取並非原子操作，不宣稱完全同 tick。App 已正常退出。

同目錄 `multi-swing-1x-reconstructed.mp4` 是 **Native mesh／tick reconstruction 的離線近景render，非wall-time app capture**：Compact80→320 hard rearm→448，world0–816每8 ticks一格，30fps／103格，標示1×；pitcher／球不在render中，旁邊只有Native clock診斷。`rearm-phase-comparison.png`含80／319／320／321／448／478；已檢視比較圖，319→320手／棒明顯從頭旁終點跳回準備姿勢。影片已驗證decode fps／格數，未宣稱agent已連續1×自看。工具未提供可靠連續錄影API，因此沒有wall-time影片，不用離線圖替代實機smoke。

人工重現：B Compact、R置中，Space起球後約0.3秒按J；約1.3秒看硬重置，放開J，再約1.85秒fresh J。以title實際consumed tick為準；OS手按不是exact448 replay。Michael＋Julia需判斷硬切是否刺眼、重新取得控制是否好玩、1秒rearm是否合適；本輪不替人判定。**沒有 recovery animation、Contact Correction、Contact Quality、Power或Ball Response**，沒有新hit VFX／SFX；停在本次human review。

### 本輪建置與驗證紀錄

Agent自身PowerShell execution核對workspace／clean main，fetch後與提交前再次fetch的HEAD／origin/main皆為 `de58971957be0a502abb7e449662f19d131ef479`。VS2022 x64／CMake／Ninja完整Debug與Release build通過；完整CTest **Debug 20/20（524.62 s）、Release 20/20（34.81 s）**。最後僅修正Paused的cached lifecycle annotation，兩組態再次完整build，並各重跑受影響contact_panel **1/1通過**（Debug6.38 s、Release0.41 s）；沒有重跑未受影響的長物理suite或放寬容差。正常遷移期間曾修正舊upper-end assertion及文字腳本encoding／standalone mesh-dump編譯定義，並非runner／sandbox故障。

Ignored `build/multi-swing-s1/` 保存兩組態build／CTest logs、final-build／panel-ctest logs、Debug完整details、Native dump／render／encode scripts、frame CSV、影片decode metadata與實機截圖。Diff另確認IngameMotion／SwingTempo、TimingInteraction／HitAuthorization evaluator、staging Data、reference pitch／delivery與physical solver未變；沒有重新匯出官方motion。實機smoke是Release，未宣稱Debug GPU validation或hard reset手感已驗收。

## Multi-Swing human acceptance／Batter Attributes S1（2026-09-19）

Michael已人工驗證極早第一揮、live hard rearm、第二次獨立出棒及第二揮Contact，**Multi-Swing Intent S1 human accepted**。本輪沒有改動這個lifecycle、Timing Interaction、Spatial Authorization、reticle ellipse或physical solver。新增單一Michael profile與Batter Card純Data／HUD；domain／grade、未來Contact／Power／Trajectory責任與normalized spatial quality原則由 [Batting Feel](batting-feel.md#batter-attributesbatter-card-s12026-09-19) 擁有，尚未接入gameplay，沒有correction／quality／Ball Response。
