# Hit Authorization S0 — Gameplay Contact Model

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
