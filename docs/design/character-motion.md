# Pawapuro Character Motion Rules

v0.1｜2026-09-16｜待 Michael＋Julia design review。

本文件定義超現實造型下仍須可信、可讀的 motion relationships，以及完整動作 authoring 前的推理方式。v0.1 由目前右投手的實作需求與 human feedback 長出；只有新的真實 caller 才新增規則，並非完整的角色身體力學理論。打者與守備動畫未來可沿用這些問題，但本版不預定其動作規格。

**Reality is a consultant. Gameplay is authoritative.** 真人參考協助理解動作因果，Pawapuro gameplay readability 決定表現。這不改變 [Batting Feel](batting-feel.md) 的 Native simulation／Data 邊界；本文件也不定義 anatomy、skeleton specification、Blender 操作、runtime architecture 或 pose library。

## 三層模型

### 1. Motion Truth：玩家必須相信的動作關係

誰正在支撐、身體正在蓄力／跨步／加速／回穩、手是否持球、動勢由哪裡帶向哪裡，以及 gameplay event 後是否延續，都是 Motion Truth。它是 Pawapuro 世界中服務 gameplay readability 的動作因果，不等於真實人體物理或精確力學計算。

### 2. Visual Representation：角色如何表達這些關係

Detached feet、large flat feet、rubber-like continuous arms、spherical hands、oversized head 與誇張 equipment 是目前採用並接受的角色語言。沒有可見 legs／elbows／fingers 是造型選擇，不是等待補完的人體細節；也不需要以「內部藏著真人解剖」解釋。

**Visual topology 不等於 Mechanical / Motion topology。腳與 torso 可以視覺上分離，但只要參與同一個 coil／stride，motion 就仍必須連動。** 風格方向接受不代表每個姿勢的輪廓已完成 polish；目前 bind-pose debt 見末節。

### 3. Implementation Mechanism：目前用什麼方法表達

Detached foot transforms、hidden bend joints、skin weights、grip node、authored contact intervals 與 release marker，都是目前 authoring sample 使用的方法。它們可更換，只要仍表達同一個 Motion Truth；不因此要求腳永久 parent 到 torso，或把目前 Blender／GLB hierarchy 升為永久角色架構。

實際資產與工具契約由 [pitcher README](../../pawapuro/batting/pitcher/README.md) 維護。S0 authoring motion 已接受；S1 runtime 只顯示 static bind mesh，尚未播放或 skinning。Authoring 使用某種機制，不代表 app 已整合它。

## 六條 v0.1 rules

### Rule 1 — 支撐關係優先（Support is authoritative）

即使沒有可見腿，support foot 仍須看起來能承重；支撐期間不能任意滑動或漂移。Lift、airborne、landing、re-support 應可區分，落地必須重新建立支撐，而非鞋子到達終點就算完成。是否承重由 Motion Truth 決定，不由 mesh 是否連在一起決定。

右投手的既有例子是後腳支撐蓄力、前腳接住跨步，再讓卸重的後腳跟進落地、恢復支撐。這說明支撐轉移，不規定其他動作也採同一套步法。

### Rule 2 — 分離不等於獨立（Detached does not mean independent）

Authoring 先問：**「這個 detached part 現在屬於哪個 body action？」** 投手 torso coil 時，抬起的前腳必須隨同一個蓄力意圖收進、再連續展開成 stride；不能只有 torso 轉動，腳仍沿獨立的 world-space 路徑上升。

連動是姿勢、路徑與時序的關係，不要求腳完整複製 torso 旋轉，也不指定永久 parenting。

### Rule 3 — 動勢依意圖傳遞（Motion propagates through intent）

投手的可讀關係是 support／body coil → pelvis／torso opening → throwing-arm acceleration → release → follow-through。部位應有 lead／lag，手端的加速要看起來由較大的身體動作帶出，而非各自搬到指定位置。

不要讓所有部位同時啟動，或只把同一條曲線乘不同振幅當成時序差；區段可以重疊承接，不必逐段停住。這是 motion causality 的要求，不是 anatomical kinetic chain 或人體角度／速度規格。

### Rule 4 — 彈性肢體保持連續流動（Elastic limbs preserve continuous flow）

Arm 是一條 elastic action line：shoulder 是起點，hand 是主要遠端，兩者之間保持連續、柔軟且可讀的 curve。可以使用 hidden joints／bones，但 visual silhouette 不應明顯讀成 upper arm＋硬 elbow＋forearm。管狀肢體的壓扁、尖折或塌陷會破壞這條動作線，固定骨長或匯出通過不能代替輪廓檢查。

Acceleration 可以透過 lag／whip 表現，follow-through 可以延遲 settle。Rubber arm 未來可承載更誇張的 arc 與節奏；目前不定 stretch ratios、squash conservation 或 runtime deformation feature，等 gameplay caller 證明需要才擴充。

### Rule 5 — 持有關係由語意成立（Attachment is semantic）

沒有 fingers 仍可清楚表達持球、持棒或伸入 glove；不需要模擬 finger contacts 才算「拿住」。投手持球時，hand／grip 應清楚擁有並帶動球，不能靠球獨立漂移、突跳或任意消失掩飾 attachment。

出手事件前是持有，事件後才交由球的 simulation 更新，持有與釋放的界線必須可讀。**這是待 runtime integration 實現的交接契約，不是已完成狀態：**目前 S0 輔助球跟隨 grip、marker 後隱藏；S1 app 球仍由獨立 reference pitch 運作，沒有 hand attachment／release integration。未來 glove／bat 可沿用語意問題，本版不定 catch 或 batting implementation。

### Rule 6 — 動勢必須有去處（Momentum must resolve）

Release、contact、catch 等事件不應自動等於全身停止。右投手已證明：release 後 torso、throwing arm 與 rear foot 仍須各自延續並共同進入 recovery，玩家才會相信前面的力量存在。

累積動勢須以可讀的 follow-through、recovery、plant、recoil 或 settle 收束；不能用結尾停格冒充收勢，也不能為固定 clip 長度截斷必要的承接。這些是可能的去處，不是每種棒球事件的固定收勢清單。

## Motion Brief：完整 authoring 前先回答因果

新的角色 motion 類型在 full animation authoring 前，先寫一份短 Motion Brief。每個主要 phase 一列，填到能說清楚相鄰區段如何承接即可，不要求人體角度／速度表。

| Phase | Support / Contact | Body Intent | Lead | Lag / Trailing | Attachment | Momentum Next |
|---|---|---|---|---|---|---|
| 區段 | 誰支撐／接觸，何時轉移？ | 身體主要動作與方向？ | 哪個部位先動？ | 哪個部位落後／跟隨？ | 物件由誰持有，何時交接？ | 動勢下一段去哪裡？ |

Brief 可放在相關 task、README 或 review notes，作為作者與 reviewer 的共同推理依據；它不是 runtime Data、TOML 或 gameplay DSL，不建立 template engine 或檔案生成系統。

### Reference 使用

新的複雜運動類型若尚無已證明的 body mechanics／Pawapuro motion relationship，full authoring 前應先實際檢視至少一段完整、連續且來源可信的主要 motion reference，理解 phase ordering、support changes、lead／lag、motion arcs 與 follow-through。不是所有任務都強制 web research；可用已取得的可信來源，既有已接受動作的局部修訂也不必重新廣泛研究。

- 記錄來源、實際觀看區段／視角、正常或慢放；已知的拍攝 FPS、播放 FPS／倍率分開記，未知就標示未知。幾張 pose 只能支持姿態觀察，不能證明完整 timing；慢放播放秒數不直接當真實動作時間。
- 以一個連貫的主要動作為依據；若混用其他 motion style 的局部參考，說明差異與承接理由，不能無聲拼接成缺乏因果的動作。
- Brief 明確區分「來源直接觀察」「作者推論」「為 Pawapuro readability 刻意誇張」。真人 anatomy 不因參考而成為必要條件。
- 無法存取或觀看就如實記錄，不宣稱看過。可先標出候選姿勢與未確認時序；不能把它們當成 reference-verified 的完整動作模型。

## Technical invariants 與 human review

既有 caller 已證明有價值的檢查包括：planted support 不滑、airborne foot 不穿地、landing 後恢復支撐、目前 fixed-length limb 不異常伸縮、attachment node 不漂移、event 前後動作不突然歸零，以及 deformation 不出現明顯尖折／塌陷。依本次動作與現有工具選用；固定骨長是目前機制的 guard，不是禁止未來經授權的 elastic 表現。

**Technical checks 只能抓 regression／不可能狀態，不能宣告動作有力量、重量感好或有 Pawapuro 味道。** Human motion review 仍是必要 gate，要看完整連續動作、正常速度及無診斷標記的輪廓；數值、漂亮 pose 或慢速檢視都不能取代。這不授權 generic biomechanics checker／motion validation framework。

## Design note 與目前 Character Style debt

### Huge-head balance：暫不升為 law

頭部尺寸主要服務 gaze、emotion 與 silhouette，不要求按真人 mass distribution 推導平衡或跌倒條件。目前以整體 silhouette、support 與 recovery readability 判斷；沒有真實 gameplay caller 證明需要 mass table、center-of-mass solver 或完整 balance theory。

同樣暫不定義 stretch ratio、squash conservation、spine rules、gait cycle、jump／fielding mechanics 或 batting kinetic-chain specification。新的真實需求出現後再提出有界規則，不為未完成角色預建理論或系統。

### S1 acceptance 與延後的 polish

Michael＋Julia 已接受 S0 的單一 pitch clip authoring motion，以及 S1 static GLB runtime import：正式 GLB 在實際 app 的基本位置、尺度、左右／朝向、顏色、grounding 與 scene integration 可接受，renderer 不需為此擴張。S1 接受不代表 animation playback、skinning、rubber-arm 動態造型、release／ball attachment、dynamic occlusion、early-flight readability 或 M1 通過。

S1 bind-pose human feedback 留下兩項 debt：

- Feet footprint 視覺上偏小，站立感與 support readability 不足。
- Arm silhouette 仍有可讀成 elbow joint 的折角，與 rubber-arm visual language 不完全一致。

這些是目前 runtime bind pose 的回饋，不能推論所有 animated poses 都有同樣問題；不是 S1 importer failure，也不撤回 S0／S1 接受。待未來 S2 runtime animation 能在真正 batting camera 播放後，再做 Character Motion／Style Polish review。本輪不改 feet size、mesh、weights 或 motion。

下一技術切片仍是 S2，**尚未授權、尚未開始**。本次只交付 v0.1 文件，停在 Michael＋Julia review gate；技術實測仍由 [environment](../development/environment.md#s1-static-pitcher-glb-runtime-import2026-09-16) 擁有。
