# Batting Feel：體驗與實作邊界

更新：2026-09-14。使用者已指定的目標與層級決策在此記錄；具體控制、範圍和驗收提案見 [M1](../milestones/01-batting-feel.md)。背景依據是 [Jai 研究](../research/jai-design-adoption-review.md)；本文件不重述語言比較。

## 體驗目標

**讓玩家打完一球後，想立刻再打一球。**

玩家應能理解上一球發生了什麼、感到自己能改善，並願意立即試下一次。完美接觸是最重要的瞬間，但之前的判讀、揮空的力量感、之後的飛行與全壘打，都決定這個瞬間是否成立。

先建立完整、可調整的短循環，再針對實際不成立的階段改進。可重現的技術實驗是工具；不能用 debug overlay、數值正確或一段自動擊球影片代替玩家實玩。

## 六階段設計

| 階段 | 玩家必須得到的感受／資訊 | Pawapuro 要控制的內容 | 需要觀察的失敗 |
|---|---|---|---|
| 1. 投手動作與節奏 | 球離手前已能從動作預備揮棒；出手可信 | 抬腿、停頓、跨步、旋轉、出手點、release timing 與 clip phase 的對應 | 球在手外生成、動畫與出手不同步、任意延遲讓判讀失去意義 |
| 2. 朝本壘飛行 | 球有壓迫感，速度差與曲線可辨，仍能追蹤 | 速度、spin 向量、drag／升力模型、位置；camera／FOV 與聲音的速度感 | 球難以辨識、camera 掩蓋軌跡、顯示位置與判定位置分離 |
| 3. 揮棒 | 按下後是有重量與力量的完整動作；揮空也成立 | 啟動、加速、bat path、bat speed、破風聲、follow-through | 輸入遲鈍、球棒瞬移、揮空立即截斷動畫 |
| 4. 接觸 | 球心＋sweet spot＋好 timing 的接觸顯著令人滿足 | 接觸時間、點、法線、相對速度、COR、接觸品質及聲畫回饋 | 聲音好像打中但物理沒有接觸；重複播放；差擊與好擊只差 HUD 分數 |
| 5. 擊球飛行 | 從起飛方向、速度與弧線預測落地、撞牆或越牆 | 出球速度／方向／spin、後續飛行、視角、可辨識場地尺度 | 一接觸就切掉球路、鏡頭追丟、結果只有距離文字 |
| 6. 全壘打 | 看到越牆並得到情緒釋放，同時想再打 | 有效越牆判定、球路保留、聲音與短 camera sequence、回到下一球 | 用距離門檻取代實際越牆、提早宣告、長時間不可跳過的演出 |

灰盒可用來檢驗早期因果；M1 完成時，投手與打者都必須有足以判讀、具有可愛比例的連貫動作。這不要求正式美術品質、完整臉部系統或大量動畫庫。

## Engine Native／Pawapuro Native／Data／Lua

| 責任位置 | 本次例子 | 判斷界線 |
|---|---|---|
| Engine Native | 時間、固定步長更新、輸入取樣、math、clip／pose evaluation、通用 geometry query、audio playback、camera primitives、rendering、debug draw、asset lifetime | 能力本身具有一般性，而且已有 Pawapuro caller。引擎不辨識投手、球種、sweet spot 或全壘打。 |
| Pawapuro Native | 棒球 aerodynamics、spin／球種模型、出手、bat path、bat-ball 接觸、sweet spot、exit velocity、launch angle、結果分類 | 即使包含物理或數學，目前仍是棒球需求。先在 game 概念模組實作，不能因「可能通用」移進 engine。 |
| Data | 球速、spin rate／axis、drag、COR、bat speed、sweet spot 範圍、timing window、動畫 phase／release marker、camera 與 feedback 參數 | 「值是多少」。參數有單位、範圍、default、owner、套用時機。公式與規則不偽裝成可執行 data DSL。 |
| Lua | 下一球 preset、練習 scenario、camera sequence、接收接觸／越牆事件後的流程與狀態轉換 | 「這次做什麼、接著做什麼」。發出高層命令，不能每 tick 積分棒球、運行碰撞或成為另一個 physics engine。 |

實用規則：每 tick 必須執行且 replay 要一致的邏輯優先 Native；需要頻繁改的值優先 Data；流程協調才優先 Lua。Native game 是正式位置，並非等待搬到 Lua 的過渡層。

引擎與 game 的分界靠具體函式、資料 ownership、依賴方向和 source location 成立，不要求 interface、factory、DLL 或全專案 C ABI。將來 Jai migration 沿已存在的資料與 library 邊界評估；M1 不建 migration framework。

## 目前靜態場景的 staging／Data 契約

本次延伸 delivery 1 做 staging calibration，**不是 delivery 2**。開發顯示固定為 windowed 1920×1080、16:9（原 1280×720 對使用者太小），不提供任意 resize、fullscreen 或解析度選單；保留既有 D3D12 resize 函式。尚不實作 DPI mode switching、letterbox／pillarbox。

基準情境為 **右投手 vs 左打者**，唯一 preset 為 `right_handed_pitcher_vs_left_handed_batter`。+Z 朝投手、+Y 向上；捕手視角的畫面右側（+X、一壘側）是左打者打擊區，畫面左側（−X、三壘側）是右打者打擊區。本輪 camera 位於 +X，右投手的 release reference 位於中央軸的 −X 側；這是固定 staging 契約，不是通用投球生物力學限制。

Camera 仍退到本壘後方以容納本壘，不能稱為打者模型內的真實眼睛位置；降低 camera 並讓視線接近水平，FOV 本輪維持 36°，以保留既有壓迫感並隔離此次 position／target 的調整效果。沒有 runtime 左右打切換；不移動投手丘／投手板的 X 或 Z 來湊構圖。

唯一的 authored staging 檔是 [pawapuro/batting/staging.toml](../../pawapuro/batting/staging.toml)，與 `staging.cpp/.hpp`、`reference_scene` 位於同一概念目錄。Engine 不接觸 TOML、投手丘、release 或 batter-side 語意；保留既有 `engine::Vertex` coupling，不增加 renderer abstraction。

| 位置 | 本輪決定與原因 |
|---|---|
| Data | Camera position（X 即 lateral offset）、target、vertical FOV；release position、球 marker radius；草地半寬／遠端 Z；投手丘底部／平頂 radius；本壘平面紅土與投手丘外圍視覺紅土 radius。這些都有實際 staging 調整需求。距離採公尺，FOV 採度。`camera.preset` 目前只接受上述完整對戰名稱，避免標籤與構圖意義不符。 |
| Native | 本壘尺寸、本壘尖端至投手板 18.4404 m、投手丘高度 0.254 m、投手板尺寸與丘中心位置；它們是空間參考，不能為了讓投手看起來更近而任意調整。2 m 高度標尺／0.5 m 刻度也是固定度量參考；標尺 X=0、Z=投手板中心，直接由既有尺寸推導，不提供會讓中央軸漂移的 Data offset。 |
| Native | 固定開發視窗尺寸、geometry 拓樸／分段數、顏色、細線厚度、近遠裁切、草地背向延伸至 z=-12 m；目前沒有反覆調整需求。Release 圓環尺寸從球 marker radius 推導，支柱底端依丘面高度計算，避免同一關係有多份可漂移設定。 |

`BattingStaging` 是 batting 擁有的一份具體、唯讀啟動快照。toml++ 僅在 `staging.cpp` 解析；完整 validation 通過後才讓 geometry／camera 使用，不把 parser node 傳入 Engine。只在啟動讀檔，沒有 file watcher、hot reload、Lua 或 property system。

缺少個別欄位時採 `staging.hpp` 的安全 defaults，並逐項記錄；這些 fallback 不必隨每次 authored Data 調參同步修改。整個檔案缺失、TOML 語法錯誤、型別錯誤、unknown key、非有限數值或超出 `staging.cpp` 的界限時，顯示檔案／欄位或語法位置的錯誤，拒絕啟動並正常回傳 exit code 1，不默默套用另一個完整場景。Camera 與 target 的界限分離，避免零方向或平行 up vector；mound top radius 的上限低於 base radius 下限，避免退化坡面。新增紅土半徑的界限讓兩塊區域保持草地間隔，且投手丘的視覺紅土半徑不小於任何允許的 raised mound 半徑。舊 preset 名稱或與本基準相反的 camera／release X 符號會明確拒絕，不建立相容／切換層。

球 marker 仍是刻意放大的視覺參考，不是物理球半徑。Raised mound 保留簡單平頂斜坡與既有高度／半徑；外圍較大的紅土圓盤只改變平面顏色與輪廓，不增加隆起高度，不作碰撞或 simulation 地形。本壘另有獨立紅土圓盤，兩者之間主要是草地，移除舊長條走道及其橫向刻線。投手板中央金色標尺提供身體中心軸／高度 context，青色 release 標記表示相對偏移，沒有投手模型。外野草地與稀疏色帶保留，不建立 terrain／stadium 系統。實際採用的 Data 值以 TOML 為準；畫面比較與驗證證據記於開發環境文件。

## Concept locality

未來布局採概念分組。下列是位置示意，**本輪不建立這些檔案，也不要求空模組先存在**：

| 概念位置 | 靠近擺放的內容 |
|---|---|
| pawapuro/pitching/ | pitching_sim.cpp、pitching_rules.lua、pitching.toml、動作／出手設定與 README.md |
| pawapuro/batting/ | batting_sim.cpp、batting_collision.cpp、batting_rules.lua、batting.toml、feedback 設定與 README.md |
| pawapuro/ballpark/ | 簡化場地與越牆規則、場地 data、README.md |
| pawapuro/practice/ | 練習 scenario、下一球流程、README.md |

只有概念開始工作時才建立所需檔案；不是每個目錄都必須有所有語言。Game 的 camera／audio feedback 設定靠近其用途，engine 只擁有通用的播放與取樣能力。模型等 binary 可依資產需求存放，概念模組保留可追查的 asset ID／來源路徑。打擊設定只有一份權威來源，不能又複製到集中式 scripts/、data/ 和 editor/ 三棵平行樹。

跨模組共享一份具體資料時，明定單一 owner，其他位置引用；不要為追求 locality 複製同一資料。Batting 面板也由 batting 擁有，只使用通用 UI primitives。

## 一球的明確資料流

1. Scenario 選擇投球／動作 preset，送交 Pawapuro Native；Native 驗證並在準備開始前建立本球設定快照。
2. Native 固定步長時鐘推進投手 phase。Engine 在指定時間評估 pose；Pawapuro 由固定 tick／事件時間判定 release，使用同一取樣規則的手部出手 transform 初始化棒球。
3. 輸入記錄經明定規則落到 simulation tick；Native 接受揮棒命令，依 aim 與 data 推進 bat path／swing phase，並產生可渲染的動作狀態。
4. Native 積分球路並處理接觸。一次有效接觸產生一筆有序記錄：時間、位置、法線、入射球／球棒速度、sweet-spot 偏差、出球速度／方向／spin、品質。
5. Presentation 讀取該記錄，安排聲音、畫面與 camera feedback；scenario 接收語意事件，協調後續流程。飛行和越牆仍由 Native 決定。
6. 本球結果、版本與輸入保留供重播；玩家要求下一球時清理本球狀態，載入下一份已驗證快照。

這是可直接追蹤的少量順序呼叫與資料記錄，不預先建立通用 event bus、command framework 或 reflection registry。正式遊玩與 debug overlay 讀同一份權威 state。

## 動畫、碰撞與時間

投手動作是判讀輸入，不是里程碑末尾才加的裝飾。出手 marker、動作取樣與 simulation 時間必須有一個可觀察的對應；變更 clip speed／停頓時，明確更新 phase 對應，不能另開一個與畫面無關的出球 timer。

揮棒接觸使用同一條已記錄的 bat motion。早期可用明寫的有限曲線／剛體球棒，之後對接動畫；最終顯示球棒不能使用另一套無關姿勢。動畫取樣可在 Native tick 進行，render interpolation 不回寫 simulation。碰撞若需要 substeps 或連續查詢，先處理當前球棒／球的有界問題；不要因此建通用 rigid-body solver。

Timing window 用於判讀／feedback，不能使沒有幾何接觸的球自動命中。Sweet spot、球心偏差、相對速度與 timing 必須能分別觀察；額外手感修正若被採用，要由 Data 明示並記錄，避免把同一優勢重複加成卻無法追查。M1 不做隱藏吸球、保證全壘打或自動修正瞄準。

固定步長的頻率是需用支持球速、球棒角速度與接觸案例驗證的值；不以提高 tick rate 取代防穿透設計。卡頓、pause、single-step 不得偷偷改步長或丟掉已接受的輸入。追趕上限或暫停策略要可見並可記錄。

Presentation 可使用不同播放速度與插值，但不能改出球結果。M1 預設不用會停止權威模擬的 hit-stop；若後續手感證明需要，另定時間／輸入與 replay 契約。Camera shake／FOV／音效強度只影響呈現，不偷偷改球速。實際顯示、audio device 和輸入延遲需區分，不能用 CPU timestamp 假稱已量到端到端 latency。

## 調參、reload 與 replay

| 修改種類 | 發布與失敗政策 |
|---|---|
| 本球 simulation／動作／碰撞參數 | 候選完整驗證後，下一球快照套用；需要立即比較時按「用新設定重投」。不混合半球新舊版本。 |
| 純 presentation 參數 | 可在下一 render frame 套用；介面標記即時 override。重播需保留可用於比較的 presentation preset。 |
| Lua scenario | 新 module 載入及 API 驗證成功後，於下一球／scenario 安全點替換；不保存任意 coroutine stack。失敗繼續舊版。 |
| 本次使用的 clip／mesh／texture | 候選 generation 驗證與 upload 完成後再替換；會影響手部出手或 bat path 的資產下一球套用。舊資料等 CPU 使用者及 GPU fence 完成再回收。骨架不相容則拒絕並報明原因。 |

先用明確的 Reload 操作取得可靠行為；自動 file watch 只在有實際摩擦時加入。面板 override 與磁碟值要分開顯示；Save／Revert 由人明確操作，不能在背景無聲覆寫原始設定。依賴一起改動的欄位先整組驗證。M1 的具體 reload fixture 與時限在 milestone 定義。

一球至少能保存：初始 state、tick 設定、已接受輸入及 tick、random seed／抽樣結果、resolved data、scenario 決策、Native build ID、script／asset 版本及事件序列。Replay 重播已接受的命令，不重新抽球種或執行可能產生不同選擇的 scenario。保存數值與穩定 ID，不 dump 指標、STL 內存或 Lua VM。

M1 重現目標是同一 build、相同平台／資產、相同 tick 輸入；不承諾跨 compiler 浮點 bit-exact。純 presentation 的 frame rate 改變不能改變這個 simulation 結果。缺少資產／build 版本時明確顯示不能精確重播，不能冒充原始版本。

## 可追查的工具

每球調參與 debug 面板應能回答「這個行為從哪裡來」：

- Native owner：概念模組及相關 source／procedure 名稱，另有 build ID。
- Scenario：Lua 檔案、scenario 名稱、版本；命令來自 live scenario 或 replay。
- Data：路徑、key、單位、磁碟值、目前 override、本球實際值、pending 值、版本及套用時機。
- Asset：clip／mesh ID、來源路徑、generation、目前取樣時間及 release／contact marker。
- Event：本球 ID、tick／必要的子步時間、release／contact／越牆事件及消費狀態；一次接觸只觸發一次回饋。

這些標籤可以在具體呼叫處明寫；source label 是定位線索，不是保證自動推導完整 call graph。只有 debug 面板需要 provenance，不把工程資訊塞入玩家正常流程。

## 保持小的限制

不預建 ECS、通用 physics engine、完整 editor、通用 reflection、通用 scripting framework、job graph、多 backend RHI 或全引擎 C ABI。不要因只有一個 caller 的 helper 可寫得很泛型，就把它升成 engine subsystem。

使用簡單容器與局部 RAII；scratch 只在有明確短期 lifetime 的工作使用。先量測，才決定 SOA、更多 threads 或 GPU compute。權威投打模擬留 CPU；local AI 不屬於 M1，也不能成為逐 tick 依賴。

功能無法使六階段更可玩、可判讀、可調或可查錯時，先不加入。若可玩性驗收未過，修正最弱階段，不以新增 infrastructure 代替玩法改進。
