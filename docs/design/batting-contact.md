# Hit Authorization S0 — Gameplay Contact Model

2026-09-17；**design candidate，待 Michael＋Julia review**。僅定責任與 normalized examples；沒有 production authorization、input、correction solver 或 ball response。Bat Contact S0／S1／S2 已接受的 physical truth 保留，M1 未完成。

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
| Player Aim / Intent | 玩家希望送到 strike-zone／contact plane 的位置；未來附 input tick、mode 與版本 provenance | 不是 bat centerline、capsule 或 rendered mesh |
| Hit Authorization | 依 mode、Contact 與 spatial aim error，允許或拒絕一次有效接觸嘗試 | 不擴大物理半徑、不保證實際命中、不給 Perfect |
| Contact Resolution | 授權後，在有限合法候選內以 continuous geometry 找 time、barrel location u、normal、relative motion | 不決定能力、不憑空指定接觸點 |
| Contact Quality | 未來解讀 aim、修正成本與真實接觸訊號 | 本輪無 score、weights、sweet spot 或等級門檻 |
| Ball Response | 未來消費接觸資訊，產生 exit velocity、launch angle、spin、foul／fair | 本輪不產生球路回應 |

Authorization 是必要 permission，**不是充分接觸保證**。允許嘗試但 budget 內無解，必須回報無可信 solution，不能補造 hit。反過來，未授權的 physical overlap 仍是 geometry 事實，不自動成為 gameplay accepted hit；未來 caller 必須解決畫面相交卻不承認 hit 的演出一致性，不能隱匿事實。本輪既有 S1 event 完全不加 gate、不改行為。

## Normal／Power 與能力

黃色 ellipse 應表達 **gameplay authorization region**；中心是 player intent，dx／dy 是 relevant pitch point 相對中心的 aim error。比較點究竟採何 contact plane／crossing，以及 input 鎖定時刻，由實際 caller 決定，保留既有 strike-zone truth。

`q = (dx/rx)^2 + (dy/ry)^2`：q=0 中心，q=1 邊緣，q>1 外側。rx／ry 尚無公尺值，UI 不必永遠使用解析橢圓。這是 mode-specific normalized design coordinate，不是從 reference 像素反算的公式。

- **Normal**：較大 authorization ellipse、較多 rescue 空間。Contact 第一版只影響其 spatial aim 範圍；不預設擴大 timing window、增加 Power 或改 response。
- **Power**：顯著較小 region、較少 correction budget；需要更精準的 intent，Power 的較高 upside 留給未來 response。Power 不放大 aim region。
- **硬邊界**：Contact／mode 都不改 physical ball sphere 或 bat capsule，不用 hidden giant bat／radius multiplier。Power 的取捨不應靠縮小 physical bat，否則把操作能力混成器材尺寸，破壞同一 physical truth。

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

未來 quality 可讀：normalized aim error q、correction used、segment u、contact normal、normal closing speed、relative velocity、eventual player timing error。本輪不合成分數、不給權重；normal.y>0、interior u 或高速本身都不宣告高品質。

## 最小 Data／owner 提案與未決事項

只需 batting-specific 概念：`BatterAbility { contact, power }`、`SwingMode { Normal, Power }`、derived `AuthorizationRegion { rx, ry }` 與 `CorrectionBudget`。能力尺度、兩 mode 的 derived mapping、budget 維度與單位尚未決定；不新增 production TOML、stats system、modifier stack 或 reflection。

未來 Pawapuro Native 擁有可 replay 的 authorization／resolution 判定；Data 擁有經 caller 證明需要的 tuning，Engine 不擁有棒球能力語意。Intent 值及決策結果由該次打擊 owner 保存到其結果／replay 使用者完成；solver scratch 只活於單次查詢，不改現有 animation／simulation owner。這是 ownership 提案，尚未新增資料結構或 sequencing。

真正 player-input caller 出現後才能定：aim space 與投影／crossing sample、何時鎖定 intent、swing commit／timing error 定義、能力尺度及 rx／ry、Power region 的 Contact 關係、修正上限與可達性、無解或未授權 overlap 的一致演出、quality／defensive outcome 的消費方式。Ball Response 數值另案決定。此次 review 只確認責任邊界，不用先決定 balance。

## 保留的 physical truth 與驗證

- S0 continuous closest approach 保留；moving ball／barrel 不能只測離散 frame。
- S1 sphere／capsule earliest-entry event 保留；ball37／bat33 mm、endpoint 合法性、sub-tick time、normal、relative motion 與原球路皆不改。
- S2 phase map 保留；phase 改變 u／normal／relative velocity，超前側到 +60 ms 仍 Contact、邊界未知。不重跑 study，不推導 input window。詳細數字仍由 [batter README](../../pawapuro/batting/batter/README.md#bat-contact-s2--timing-to-contact-geometry-map2026-09-17) 擁有。
- 本輪只改 Markdown；檢查 diff、連結、來源 hashes 與 evidence metadata。未跑 C++ build／CTest／GPU，也沒有 production 行為變更。沒有新增 Character Motion Rule。

### 可重查 evidence

本機忽略目錄 `build/hit-authorization-s0/`：`video-metadata.json`、`sources.json`、`decode.py`、原解析度抽樣 PNG、`overview.jpg`（全片概覽）、`contact-full.jpg`（f49／52／54／55）、`contact-crop.jpg`（f46–58，局部不含完整游標）。連續 f35–65 PNG 保留；`contact.jpg` 是拼圖產物，但工具未成功顯示該大圖，不將它列為已觀看證據。原始媒體不複製進 repo，以下識別供其他工作階段查核：

- `ref-batting-contact.mp4`：11381641 bytes，SHA256 `544808db37957d5525e44ae5c937b9bc4314ad451665bcd50ab288d93aa2ae7c`。
- `pawapuro-batting-ref-contact.png`：3885175 bytes，SHA256 `05f94daf09987a325b3b61dc3f091de99613716a8fd3b0dcdb9807c2f5a90368`。
- `pawapuro-batting-ref-power.png`：3963638 bytes，SHA256 `6f6d361eb317866d8002c535178bc95e17e7bd601949416dde1e9ee8b1d18a8f`。
