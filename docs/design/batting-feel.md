# Batting Feel：體驗與實作邊界

更新：2026-09-16。使用者已指定的目標與層級決策在此記錄；具體控制、範圍和驗收提案見 [M1](../milestones/01-batting-feel.md)。背景依據是 [Jai 研究](../research/jai-design-adoption-review.md)；本文件不重述語言比較。

## 體驗目標

**讓玩家打完一球後，想立刻再打一球。**

玩家應能理解上一球發生了什麼、感到自己能改善，並願意立即試下一次。完美接觸是最重要的瞬間，但之前的判讀、揮空的力量感、之後的飛行與全壘打，都決定這個瞬間是否成立。

先建立完整、可調整的短循環，再針對實際不成立的階段改進。可重現的技術實驗是工具；不能用 debug overlay、數值正確或一段自動擊球影片代替玩家實玩。

## Gameplay-first 原則

**Pawapuro 是遊戲，不是棒球模擬器。** 真實棒球的尺寸、物理與規則是重要起點與可信度來源，但 gameplay rule 由遊戲性決定。當真實規則與可讀性、操作樂趣或遊戲性衝突時，可以有意識地偏離；每項偏離都必須說明 gameplay 理由，不能無意識地產生，也不把「更真實」自動等同於「更好玩」。

Gameplay-first 不等於越誇張越好；誇張必須改善 readability／feel。0.65 m raised mound 經使用者實玩指出過度像高台，應降低，而不是繼續靠丘高建立投手壓迫感。

好球帶是第一個明確例子：為探索更容易判讀、操作的進壘區，先以真實本壘板寬度的兩倍作為好球帶 candidate，並讓 Pawapuro 本壘 geometry 採相同 gameplay width。這是 game-design decision，不是 rendering hack；是否更好玩仍須實玩驗證。

**Gameplay-first 也必須內部一致。** Field geometry、gameplay rule 與 presentation 必須共同建立清楚的玩家心智模型，不能要求玩家猜哪一套才是真正規則。Simulation truth 與 presentation 可以不同（例如 3D 規則改用 2D overlay），但 presentation 必須忠實表達 gameplay rule。真實尺寸只作設計參考，不另維護一套平行的 physical／visual／gameplay 真相。

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

## 靜態 staging 基準／啟動 Data 契約

以下維護當前 staging 基準；球路物理不因 framing 改動，場地 presentation geometry 只依明確授權調整。開發顯示固定為 windowed 1920×1080、16:9（原 1280×720 對使用者太小），不提供任意 resize、fullscreen 或解析度選單；保留既有 D3D12 resize 函式。尚不實作 DPI mode switching、letterbox／pillarbox。

基準情境為 **右投手 vs 左打者**，唯一 preset 為 `right_handed_pitcher_vs_left_handed_batter`。+Z 朝投手、+Y 向上；捕手視角的畫面右側（+X、一壘側）是左打者打擊區，畫面左側（−X、三壘側）是右打者打擊區。Camera 改放在左打者對側（−X、三壘側），為畫面右側的未來左打者 foreground silhouette 預留空間，維持投手／來球視線。右投手的 release reference 仍位於中央軸的 −X 側；打者所在側與 camera framing 是兩件事，不改變真實棒球空間參考。

Camera 仍退到本壘後方以容納本壘，不能稱為打者模型內的真實眼睛位置；降低 camera 並讓視線接近水平，FOV 本輪維持 36°，以保留既有壓迫感並隔離此次 position／target 的調整效果。沒有 runtime 左右打切換；不移動投手丘／投手板的 X 或 Z 來湊構圖。

Camera 的 physical lateral offset 與 screen composition 是不同概念：位置留在左打者對側，target X 與 camera X 對齊，水平 optical direction 沿 +Z，不再用向右 yaw 強行置中。保留既有 vertical target／36° FOV，改由 horizontal off-axis perspective 將 gameplay focus 置中；因此世界 X 向橫線保持水平，而投手仍偏左、右側保留打者空間。這是正式 animation pipeline 前的 camera correction，沒有移動任何人物／球場或修改 gameplay truth。實測值與截圖證據見開發環境文件。

唯一的 authored staging 檔是 [pawapuro/batting/staging.toml](../../pawapuro/batting/staging.toml)，與 `staging.cpp/.hpp`、`reference_scene` 位於同一概念目錄。Engine 不接觸 TOML、投手丘、release 或 batter-side 語意；保留既有 `engine::Vertex` coupling，不增加 renderer abstraction。

| 位置 | 本輪決定與原因 |
|---|---|
| Data | Camera position（X 即 lateral offset）、target、vertical FOV；release position、球 marker radius；草地半寬／遠端 Z；投手丘底部／平頂 radius 與 height；本壘平面紅土與投手丘外圍視覺紅土 radius；authoritative gameplay strike zone 的 width／bottom／top。這些都有實際 staging 調整需求。距離採公尺，FOV 採度。`camera.preset` 目前只接受上述完整對戰名稱，避免標籤與構圖意義不符。 |
| Native | 本壘五角形的比例與由 width 推導的 depth／中央 plane、本壘尖端至投手板 18.4404 m、投手板尺寸與丘中心位置；它們是空間參考，不能為了讓投手看起來更近而任意調整。2 m 高度標尺／0.5 m 刻度也是固定度量參考；標尺 X=0、Z=投手板中心，直接由既有尺寸推導，不提供會讓中央軸漂移的 Data offset。 |
| Native | 固定開發視窗尺寸、geometry 拓樸／分段數、顏色、細線厚度、近遠裁切、草地背向延伸至 z=-12 m；目前沒有反覆調整需求。Release 圓環尺寸從球 marker radius 推導，支柱底端依丘面高度計算，避免同一關係有多份可漂移設定。 |

`BattingStaging` 是 batting 擁有的一份具體、唯讀啟動快照。toml++ 僅在 `staging.cpp` 解析；完整 validation 通過後才讓 geometry／camera 使用，不把 parser node 傳入 Engine。只在啟動讀檔，沒有 file watcher、hot reload、Lua 或 property system。

缺少個別欄位時採 `staging.hpp` 的安全 defaults，並逐項記錄；這些 fallback 不必隨每次 authored Data 調參同步修改。整個檔案缺失、TOML 語法錯誤、型別錯誤、unknown key、非有限數值或超出 `staging.cpp` 的界限時，顯示檔案／欄位或語法位置的錯誤，拒絕啟動並正常回傳 exit code 1，不默默套用另一個完整場景。Camera 與 target 的界限分離，避免零方向或平行 up vector；mound top radius 的上限低於 base radius 下限，避免退化坡面。新增紅土半徑的界限讓兩塊區域保持草地間隔，且投手丘的視覺紅土半徑不小於任何允許的 raised mound 半徑。Camera X 允許 −1.5～1.5 m 的 presentation 調整，不以其符號判定打者慣用手；舊 preset 名稱或錯側 release X 仍拒絕，不建立相容／切換層。

### 單一 authoritative gameplay strike zone

`staging.toml` 的 `[strike_zone]` 是 Pawapuro runtime 唯一的好球帶規則來源，由 Pawapuro Native／Data 擁有，目前保存在既有 `BattingStaging` 啟動快照，沒有另建規則 API。目前 tuning candidate 為 width **0.8636 m**（真實棒球尺寸參考 0.4318 m 的兩倍）、bottom **0.30 m**、top **1.25 m**。Pawapuro 本壘 gameplay width 直接使用同一欄位；五角形依原比例放大，depth=width，catcher-side tip 維持 Z=0、肩點 Z=depth/2、pitcher-side edge Z=depth。沒有另一份 plate width Data 或 aspect tuning。已有 Q 版打者後，zone 整體下移 0.20 m，保留 0.95 m 高度。使用者選擇保留此候選，頂邊與大頭下半部仍同高的關係留待 review；不為了避開頭部而把框底繼續壓到地面，也不調球路配合。

`BattingStaging::home_plate_depth_m()` 與 `strike_zone_plane_z()` 是具體推導：plane Z=depth/2，穿過本壘前後中央，並由 reference pitch、overlay 共用。目前 plane 恰好仍為 0.4318 m，是本輪倍寬尺寸推導的結果，不是保留舊前緣常數；改 width 時 plane 必須跟著移動。投手丘、其他場地與積分公式不變。

Authoritative 指單一規則來源，不代表數值永久定案；尺寸之後可依 Q 版打者模型、站姿與實玩結果調整。藍框直接呈現這一份規則，不畫第二個真實好球帶內框。未來 ball／strike judgement、pitcher targeting、batting aiming、pitch-location feedback 預設都使用同一份 Data；本輪尚未實作這些 consumers。只有實玩證明 called strike zone 與 bat reachable area 必須分開時，才引入第二個概念，不預建 physical／visual／guide／interaction 四套區域。

三值皆為公尺：width 允許 **0.25～1.5**，保留較窄與較寬玩法候選的試驗空間（含 0.75／0.86／0.95）；bottom 0.2～1、top 0.8～2，且 top 至少高於 bottom 0.2。這是有限、非退化的啟動檢查界限，不是正式棒球規則。沿用 finite／型別／來源診斷與 defaults，只在 startup load，沒有 hot reload。舊 `[strike_zone_reference]` 視為 unknown key，沒有外部相容需求或 compatibility layer。

### Screen-space overlay 與預測落點

Gameplay zone 中心 X=0、Z=`strike_zone_plane_z()`。Pawapuro 將四角投影到 camera，以投影結果的 min/max 組成 axis-aligned NDC rectangle；藍框水平／垂直，最後以 depth test／write 關閉的 draw 顯示，不受 3D 遮蔽。這是投影範圍的平面化表達，不是第二個判定區；原 3D 四角的透視 skew 不保留。位置、大小不存 pixel Data。厚度隨投影寬度縮放。

橘色空心預測環由初始 state 建立一次獨立的 `ReferencePitch`，呼叫相同 single-step／積分及 crossing evaluation；有 2 秒上限以明確報告未抵達異常。環中心直接投影 prediction 的 world evaluation position；半徑由 evaluation plane 上的預測位置，沿 camera-right 偏移一個 ball visual radius 後投影取得；維持 screen-space 圓形，stroke 向內畫，不另存 marker radius。球的視覺尺寸改變時，環自然跟著改變，空心中央避免遮住球。Prediction 不修改正在玩的 pitch。

所有 world→screen 計算使用同一 `batting_view_projection(staging, aspect)`：world／ball draw 的 matrix、好球帶四角、預測環與 arrival 診斷皆呼叫此函式，沒有 overlay 專用 camera。先用現有 LookAt 得到 view，再將 `(0, (bottom+top)/2, strike_zone_plane_z)` 轉成 view-space focus。近平面的 horizontal offset 為 `near * focus.x / focus.z`，左右界為 offset ± `near * tan(vertical_FOV/2) * aspect`，上下界對稱；使用 DirectXMath `XMMatrixPerspectiveOffCenterLH`。Focus 必須在 camera 前方。Lens shift 由既有 Data／gameplay plane 推導，不另存 authored shift 或 pixel coordinates，不增加 Engine camera primitive／framework。

幾何 focus 精確落在 screen X 中央；vertical pitch 使 zone 上下角深度略有差異，因此四角 bounding-box 中心容許小於約 1 px 的偏差。當前 Data 讓 target X 等於 position X；若日後刻意改 target X 產生 yaw，世界橫線就不再保證水平，不能靠 lens shift 消除該傾斜。

目前固定 camera／啟動 Data，Pawapuro 在建立場景時產生 overlay NDC vertices，與 world／球 vertices 放在既有 immutable buffer。下次以不同 camera Data 啟動會重新投影；沒有 runtime camera 變更或重新配置 overlay 的 framework。Renderer 只增加一個 depth-disabled PSO，沿用既有 shader、root constants 與 buffer，identity matrix 畫 NDC。兩個實際 caller 是好球帶框與 prediction 環，Engine 不知道其棒球語意。

Prediction 環目前在各 phase 都顯示，僅為 development／gameplay exploration tool；正式遊戲是否、何時顯示，或是否依能力模糊，尚未決定。沒有 aiming cursor、好壞球判定或通用 UI。

球 marker 的 startup Data 半徑目前為 **0.085 m**，是 gameplay presentation size，不是物理球半徑。Raised mound 保留簡單平頂斜坡，高度／半徑可為舞台感刻意偏離真實尺寸；外圍較大的紅土圓盤只改變平面顏色與輪廓，不增加隆起高度，不作碰撞或 simulation 地形。本壘另有獨立紅土圓盤，兩者之間主要是草地，移除舊長條走道及其橫向刻線。投手板中央金色標尺提供身體中心軸／高度 context，青色 release 標記表示相對偏移；靜態投手 release-pose blockout 以此對照手部與球的位置，尚未建立動畫與 simulation 的出手對應。外野草地與稀疏色帶保留，不建立 terrain／stadium 系統。實際採用的 Data 值以 TOML 為準；畫面比較與驗證證據記於開發環境文件。

使用者回饋場景整體稍暗，列為待人物／materials／lighting 進入後再 review 的 presentation issue；暫不為暗沉感調整顏色或引入 lighting／material system。目前只驗證靜態 blockout 的 foreground 遮擋，不代表正式模型／動作不會遮擋。

### Pawapuro Character Style v1

**角色不是要像真人，而是要讓玩家一眼讀懂力量、節奏與情緒。** 真實人體只作參考，重心與動作方向的可讀性優先。

- **Large head**：放大頭部，讓遠距離注意力、帽簷方向與未來眼神／情緒容易辨識。
- **Large, flat feet**：腳的 X/Z footprint 偏大、Y 偏扁，強調踩住地面的底盤感；以站穩、跨步、煞車與重心轉移的可讀性優先，不追求真實鞋型。
- **Oversized cap**：帽冠包覆大頭上半部，寬帽簷從前方伸出並表達朝向；不能像浮空圓盤。
- **Detached / simplified body-foot relationship**：身體與腳可分離或簡化連接；間距是風格特徵，並非待修的解剖缺陷。
- **Simple spherical hands**：球形／橢球手即可表達握球、握棒與移動方向，不做手指細節或 finger rig。
- **Continuous rubber-like arms**：手臂是 shoulder 直接連到球形手的連續簡化肢體，不要求可見 upper arm／forearm 或 elbow anatomy。早期 static fixture 使用單一直線 segment；目前 motion relationships 由 [Character Motion Rules](character-motion.md) 維護，hidden joints 不代表可見解剖。
- **Silhouette first**：未來投手的 ready、抬腿、跨步、旋轉、release、follow-through，以及打者的 ready、load、啟動、通過 zone、follow-through／失衡，都應能快速區分；不為靜態漂亮犧牲動作輪廓。
- **Exaggerated equipment allowed**：球、bat、glove、鞋、帽／頭盔可誇張，以提升 readability、impact 與 motion clarity，並保持世界內部一致。
- **Simple face first**：先用簡單眼睛、眉毛或帽簷／頭部方向；有實際情緒需求才擴充，不預建 facial animation。
- **Proportions serve animation**：比例以未來投球／揮棒的力量、重心與節奏判讀為準，不以縮小真人或 static concept art 作唯一標準。

先前用既有橢球／短圓柱驗證右投手 release 與左打者 ready 兩個固定姿勢。放大頭、鞋、球形手、bat，縮短軀幹並保留短褲與鞋之間空隙；手、帽與鞋的相依尺寸直接由共用比例計算。這不證明動態姿勢已成立。後續 S0 已完成右投手 authoring baseline；S1 app 改讀正式 GLB 的 static bind mesh（見末節），打者仍用此 static fixture；尚未接入動畫或 skinning。

### 靜態人物 blockout

右投手與左打者使用同一組簡單橢球／短圓柱比例，在既有 Vertex path 產生固定幾何，只驗證 composition、人物尺度與遮擋。投手現改為唯一的靜態 release pose：後腳在投手板區域、前腳跨向本壘，pelvis／chest 前移前傾，右臂展開至既有 release 附近，左手手套收在身前。打者仍位於捕手視角右側、身體朝本壘，球棒斜向後上方；Character Style v1 校正造型比例，保留 ready stance 與球棒端點。姿勢在 Ready／flight／Complete 都固定，沒有動作或揮棒軌跡，release reference 不隨人物移動。

`[pitcher_blockout]` 與 `[batter_blockout]` 各保留 `position_m` 與 `height_m`；height 是站立比例 scale，頭／帽放大後不再等於精確總高。投手原點是投手板附近後腳參考，打者原點是腳底高度參考。新增一份共用 `[character_style]`，只提供 head／hand scale、foot planar／height scale 與 bat thickness scale，因本次需要反覆比較且兩個角色應採同一造型語言。沿用 startup defaults／validation；軀幹、短褲、帽簷與 pose 端點的固定比例留 Native，不把每個部位變成 Data。角色只是 Pawapuro scene fixture，沒有新增 Engine API 或正式人物 pipeline。

### Final Pre-Rig：衣襬與垂直構圖

上衣在腰線附近保有寬度並略覆褲子上緣，避免收尖後堆疊兩個 primitive 的輪廓；不新增 pelvis／腰帶物件，不以整顆 torso 放大處理。固定衣襬比例留 Pawapuro Native：投手保留原前傾胸口截面，下緣轉成水平寬衣襬；打者保留原上半橢球，下半部接至略內收的寬衣襬。褲子仍可見，褲子與大腳之間的 detached 風格保留。

垂直 staging 只調既有 camera position Y／target Y：提高視點配合下俯方向，改善遠方投手帽冠與近景打者的垂直關係，讓既有外野牆頂上移。角色 scale／pose、場地與球路不變；horizontal yaw=0、36° FOV 及既有 shared off-axis projection 保留，不新增 vertical lens shift 或 pixel offset。實測 candidate 與 evidence 見開發環境文件。

Michael＋Julia 已完成本輪靜態造型／構圖 review，Final Pre-Rig Pass 到此收尾：

- **接受**：torso／shirt 與褲子的過渡、投手的 screen-space presence，以及目前 camera。
- **暫緩**：外野牆仍略低、上方仍有留白；Michael 決定暫時保留，不再調整 camera、牆高或新增場景物件。

接受範圍僅限本輪 static fixture／vertical composition；不代表動態球路、動畫、正式角色資產或 M1 已驗收。Early-flight 背景對比與動畫遮擋仍需後續人類檢查。下一步在新的 Codex 對話進行 **Rig／Animation Pipeline 規劃**，本輪不開始實作。

### Field readability／presence pass

本次只增強投打對決的舞台感：投手以既有 `height_m` 等比例放大，維持 Q 版頭身／四肢比例，不增加尚無需求的比例欄位。Raised mound 的 `height_m` 成為 startup Data，允許 0.125～1 m；平頂 radius 上限放寬到 2 m，仍小於底部 radius 下限。投手板、中央標尺及 release 支柱的落地高度直接使用丘高；投手腳底仍由角色 `position_m` 明示，本次一起調整到板面，單獨調丘高時須同步檢查站位。Release 的世界位置與整條球路不隨丘高移動。後續 pre-animation correction 比較 0.30／0.35／0.40 m 後選 0.35 m；radius／top radius／visual apron 不變，camera 也保留原設定以免破壞近景構圖。

前次 presence pass 只抬高 camera target Y，保留 position、target X/Z 與 36° FOV；以畫面確認本壘／腳底更靠下、好球帶水平置中、投手偏左與打者遮擋。上下 framing 是 presentation，沒有調整好球帶 rule 或球路來配合。

既有 Vertex path 加入左右打擊區白線、一／三壘低矮白色 blockout、兩側界外線及帶頂緣的初步外野牆。依使用者選擇保留標準 90° diamond 方向，允許一／三壘在窄 FOV 視野外，不壓縮位置換取入鏡；界外白線在打擊區外才開始顯示，避免交叉污染近景方框。這些元素只提供方位、打席尺度與外野邊界，不加入跑壘、界內外／全壘打判定或碰撞。白線尺寸、壘包位置／尺寸、牆距／高度先保留具體 Native fixture，尚無反覆調參證據；不建立 stadium／terrain／animation 系統。

靜態 release fixture 只在 `reference_scene.cpp` 直接列出比例化的腳、pelvis、chest、肩與手部端點，沿用橢球／短圓柱。手心在 release 後方 0.13 m、下方 0.035 m，端點從既有 release Data 推導，只有 presentation 依賴 simulation 初始位置，反向沒有依賴；不是 IK。對重合端點拒絕建立幾何，避免零向量 normalization。沒有新增 pose Data schema、joints、骨架、hierarchy 或 interpolation。前跨深度在 batting 視角受透視縮短，靜態對齊不代表動態 body mechanics 已驗收；正式 rig／animation pipeline 尚未開始。

## Reference pitch：固定步長與同球重投契約

這是 **Pawapuro Native 的 gravity-only reference fixture**，不是最終棒球模型。`batting/reference_pitch.cpp/.hpp` 擁有 Ready → InFlight → Complete、pause flag、tick、previous/current ball state 與固定步長欠帳；Engine 不知道投球、本壘或 release。保存唯讀 initial state，讓 Complete 後可重投相同 fixture。

- 初始位置直接取既有 `release.position_m`。投球初始條件 Data 是 `reference_pitch.initial_velocity_mps` 三維向量（m/s），不另存 speed 或 target。目前正式 TOML fixture 的設計目標為 **5 號位／紅中**，即 `(0, (bottom+top)/2, strike_zone_plane_z)`。保留 Z 速度，由既有 constant-gravity 固定 tick 與 crossing interpolation 反推 X/Y 後寫回 velocity Data；推導與實測見開發環境文件。Runtime 不另存 target，也不會在更改 release／zone 後偷偷重新瞄準；要維持紅中 fixture 時須重新計算並通過中心契約測試。省略欄位的舊安全 fallback 不保證紅中，沒有通用 targeting system。每軸範圍 X/Y=−5～5、Z=−60～−20 m/s，需 finite；這是此 fixture 的安全載入界限，並不保證每個合法組合都投進可見區域。
- Native 暫定 **240 Hz、dt=1/240 s ≈4.166667 ms**；重力 `(0,−9.80665,0)` m/s²。每 tick 用 constant-acceleration 更新 `p += v*dt + 0.5*g*dt²`、`v += g*dt`，保存 previous/current，不使用 render delta 積分。這個 Hz 尚未證明足以處理 bat-ball contact。
- App 以 SDL monotonic nanoseconds 提供經過時間。Pawapuro accumulator 使用整數 `ns × Hz` credit（每 tick 消耗 10⁹），保留不足一 tick 的餘額。每 frame 最多 **16 ticks**，為 30 FPS 與短暫延遲保留追趕餘裕；超額欠帳保留並顯示於 title 的 `backlog`，不 clamp／丟棄時間、不放大 dt。持續低 FPS 時會落後 wall time；Complete 後不再需要剩餘欠帳。
- Ready／Paused／Complete 不累積新 wall time。Pause 保留既有 fractional credit／backlog；單步是額外執行一個固定 tick，仍保持暫停，若跨平面則進入 Complete。App 每圈先推進舊狀態，再處理該 frame 的按鍵；live input 的接受邊界仍依事件輪詢，不宣稱不同 FPS 下相同人類按鍵時刻一定落在同一 tick。
- **Space** 在 Ready release，在 Complete 清除 tick／pending ticks／fractional credit／pause flag，將 previous/current 恢復 initial state 並立即進入 InFlight；app 同時重設 wall-clock 基準，排除上一階段的閒置時間。**P** 暫停／恢復 InFlight；**.** 僅在 Paused 單步；**Esc／window close** 退出。忽略 key repeat，InFlight（含 Paused）不接受再次 release。Minimize 時自動暫停進行中的球，restore 後需 P 恢復，不在背景補進最小化期間的時間。
- Evaluation plane 來自本壘 depth 中央的 `strike_zone_plane_z()`，在建構 pitch 時固定保存。球心第一次由 `previous.z > plane` 到 `current.z <= plane` 進入 Complete；tick／current state 仍保留完整積分結果。另以該 tick 的 previous/current 線性插值，取得 plane 上的權威 evaluation sample（位置、速度與 fractional simulation time），供 prediction／位置比較共用；不回寫 current，也不偷偷調整球路。這是單一 crossing sample，不是第二份 trajectory 或 generic CCD；重力軌跡在 tick 內用直線近似，理論 Y chord 誤差上限約 0.022 mm，另有浮點誤差。原 tick overshoot 仍小於 `abs(vz)*dt`；stderr 同時列 raw tick 與 evaluation sample，避免混淆。尚無 strike-zone 判定或球半徑接觸。
- Rendering 直接讀 `current.position_m`，不產生另一條球路，也不做 render interpolation／extrapolation。`BattingReference` 保存 vertices、球／overlay 的起點與投影量測值；Engine 用同一 immutable buffer 做 world、球平移、NDC overlay 三次 draw，以 root constants 設定 matrix／平移，沒有新增 vertex buffer 或 dynamic mesh。GPU fence lifetime 沿用原有 owner。
- Title 顯示 state、tick、位置及 backlog，只有文字變動才更新；stderr 只在 release／arrival 記錄摘要，不逐 tick logging。重現範圍為同 build／平台與相同初始 state、相同固定 tick 序列；不承諾跨 compiler bit identity，沒有 replay framework。

無 drag、spin、Magnus effect、pitcher animation、碰撞、hot reload 或通用 replay。此次同球重投與進壘 framing 沒有修改球路物理或 oversized marker 尺寸；實測 initial conditions／arrival 與限制記於開發環境文件。

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

## 右投手 S0：authoring 與匯出邊界

2026-09-15，Michael 在 Julia review 後授權最低右投手 authoring sample、Blender 工具準備、GLB／metadata 匯出與預覽。第一版已製作，但 Michael 判定 motion 未通過；後續只授權下述 S0.1／S0.2A authoring 修訂，**不進入 S1**。本節不授權 C++ importer 或後續 runtime。

本次選擇原創低細節 mesh／一副必要 armature／一段不循環 pitch clip，以既有大頭、扁腳、detached feet、球形手、連續簡化手臂及衣襬風格建立動作。隱藏 bend 與 weights 只解決目前肢體彎曲、衣襬連續性的實際需求，不引入真人完整骨架或 generic character。共同 tick／CPU skinning 是後續原則接受的方向，尚未實作。

`.blend` 是可編輯來源；唯一 release marker 由 exporter 產生 timing metadata。Grip 是持球球心，其 parent 是右手；初次生成與日常匯出分開，匯出不重建來源。局部檔案、座標／placement、格式 subset 與操作命令由 [pitcher README](../../pawapuro/batting/pitcher/README.md) 維護，不複製另一份通用 pipeline 文件。

角色比例 scale 在 authoring 時換成公尺，placement 不重複縮放；asset 必須對照既有 release Data，不能為取得對齊修改 staging／camera／球路。S0 的 source 與 GLB round-trip 誤差只代表 authoring 交付，**不等於 Native release 已對齊**。後續 Native sampled initial state 若與 regression baseline 不同，仍須另列差異 review，不隱藏 snap 或重算球速。

本次輔助球只在出手前跟隨 grip，marker 後隱藏；不製造第二條球路。Authoring camera 對照同一 staging 參數，但不包含 app 打者／球場，不能代替動態遮擋／early-flight 對比驗收。既有 horizontal off-axis camera、strike-zone truth、pitch sightline 與 deterministic Native simulation 邊界保留。

### S0.1 Motion reblocking

本次只修改同一名右投手的一段粗動作；原 S0 的 source 與預覽保留比較。沿用 mesh／weights／骨架／grip，改正肩部跟隨軀幹、固定骨段長度與分開的發動節奏。右投手 coil 讓右肩向後保留、左肩較靠本壘；開轉再帶動投球臂，不把原本一條 yaw 曲線乘大。這是本角色選定的動作方向，不宣稱所有真人投手都必須使用同一投法。

**研究範圍與可信度**：本輪讀了兩篇原始研究的 PubMed **摘要**：[Oyama et al., 2014](https://pubmed.ncbi.nlm.nih.gov/24944296/) 比較骨盆與上軀幹旋轉峰值的先後；[Fleisig et al., 2013](https://pubmed.ncbi.nlm.nih.gov/24466645/) 量測 pelvis／upper trunk 相對軸向轉動，投球的重要變化在前腳接觸附近。只用於理解相對時序，不抄角度／角速度，也不把群體研究當成此 Q 版資產的數值規格。[Drew Adams／Animation Mentor](https://www.animationmentor.com/blog/splining-made-easy-with-animator-drew-adams/) 的教學用於先決定 blocking、部位順序與 spacing，再檢查曲線是否沖淡節奏；不是投球影片觀察證據。

主要 motion video reference **尚未建立**。找到的可追查候選為 Paradigm Pitching 的 [Justin Verlander Slow Motion Pitching Mechanics (Third Base Line View)](https://www.youtube.com/watch?v=YWpc7tI74Hg)；瀏覽器工具啟動失敗，沒有觀看任何區段／逐格取樣。三壘側、慢動作僅來自標題；實際視角、完整收勢覆蓋、拍攝 FPS、播放 FPS 與慢放倍率均未確認。沒有用播放秒數換算真人時間，也沒有下載參考資產。本次 attachments 只有先前的文字，未收到 Michael 提到的遊戲參考圖，無法將其列為直接視覺觀察。

下表因此是**依 Michael feedback 與上述原則提出的 authoring 推論／Q 版誇張**，不是 reference-verified timing；實際 source 的姿勢與變化已有 evaluated transforms／影格證據，時間數值由局部 asset 文件維護。

| 區段 | 本版 motion brief |
|---|---|
| Ready → coil／抬左腳 | 右腳維持支撐，pelvis 向右腳上方偏移；左腳抬高，胸口／肩線轉成側身，持球手與手套在胸前下方折合。頭部小幅延遲，保持本壘方向。準備較慢；手臂拆開前仍繼續前移，不以全身定格分段。 |
| Stride／opening | 左腳向本壘跨出時 pelvis 一起前移並先打開；chest／右手保留落後，肩線尚未完全打開。手套向前後再回收，接到左腳支撐。 |
| Acceleration／release | 左腳接住前移；pelvis 開轉峰值早於 chest，胸口前傾帶動肩部；固定長度右臂在短區間沿弧線追上。Grip 隨右手穿過原世界 release reference，marker 是中途事件，手不在此停住。 |
| Early follow-through | 左腳支撐，胸口繼續開轉／前折，右臂往前下跨身；頭的前折較晚。右腳卸重離地、向後上帶起，與手臂節奏不同。 |
| Rear-foot follow／recovery | 右腳在空中向前跟進後落地，兩腳形成支撐；胸口、頭與手繼續回到較直立的平衡位置，沒有用結尾 idle hold 增加長度。大鞋與 detached 身體關係保留。 |

Authoring 使用分開的 shape-preserving 曲線；投球階段以 FK 方向的角度插值避免方向向量正規化造成速度尖峰，再烘焙到原 LINEAR GLB 契約。沒有新增 IK、retargeting、animation graph、runtime animation 或球路。原 camera／角色比例／場地／strike-zone truth／simulation 初始條件不變。旋轉可讀性、重量感、投影遮擋與正常速度節奏仍須 Michael＋Julia review。

### S0.2A Closed Ready／Coordinated Leg Lift

2026-09-15：依 Michael 指定投法，Ready 胸口朝 game −X（三壘側），頭保持注意本壘；合手期間原手套包覆右手 grip 上的球。左腳在離地期間隨蓄力轉入自身右側、coil 集中於手套下方；維持 detached feet，不新增 solver 或改 hierarchy。這是明確的人類動作要求，本輪未另研究其他投法，也未取得可觀看的參考附件。

範圍限 frames 1–49 與 50–71 的接回過渡；72–205 的 evaluated 動作保留，包括既有 frame 79 局部凹折。Camera／比例／mesh／weights／rest、唯一 release marker 與 simulation 初始條件不變。球仍由 hand_R／固定 grip 帶動，只在原 release marker 後隱藏；藏球由幾何位置達成。

本版仍是待 Michael＋Julia review／可能再修正的 candidate。B 的手套方向／出手前右臂，以及 C 的追加左旋／右腳跨前均未實作，不進入 S1。局部操作與證據由 [pitcher README](../../pawapuro/batting/pitcher/README.md) 維護，實測與診斷由 environment 維護。

### S0.2B Arm Deformation／Glove Direction

Michael 接受「抬腳尤其左腳現在很棒」：保留該左腳／蓄力連動，不擴大解讀為整支投球通過。本輪固定整段 body／head／feet 與右手／grip 軌跡，僅修右臂 tube 的 pose roll／局部 weights，以及左臂 50–96 的展開／指向／回收。手套指向以 world game −Z 為準，不用 chest-local forward；97 起接回既有左臂。LBS／rest／hierarchy／topology、唯一 release marker、camera、staging／Native 物理均不變。

兩個局部策略比較後，選用協調 roll 加肘附近有限混合 weights；不新增骨骼／solver／corrective shapes，也不換 skinning。Saved source candidate 與操作入口由 [pitcher README](../../pawapuro/batting/pitcher/README.md) 維護。Michael＋Julia 已接受 A 的 Closed Ready、藏球與左腳／蓄力連動，以及 B 的右臂 deformation 修正與跨步手套朝本壘。正式三檔已由接受的 review/s02b 原樣複製升至 S0.2B。C 的 release 後軀幹續轉／前折、右腳跟進並落在比左腳更靠本壘的位置、完整 follow-through／recovery 仍待處理及 review。S1 未開始，整支 pitch motion 與 M1 尚未通過。 `review/s02b/` 保留驗收基準。


### S0.2C Follow-through／Rear-Foot Recovery（authoring 已接受）

以已正式 promotion 的 S0.2B 為 baseline，只修改 release 97 之後：pelvis 較早延續開轉，chest 帶動原局部手臂收勢並繼續旋轉／前折，頭部較小幅跟隨；身體前移與右腳空中跟進重疊，最後右腳比固定左腳更靠本壘。保留 detached feet，不加入 IK／grounding／solver；不是分開補一段胸口轉動與一段腳滑行。

本候選維持 1–205、60 fps、release 97；1–97 的 evaluated bones／mesh、左腳全段、B weights／rest／hierarchy、camera／scale／staging／Native 球路不變。右腳實際仍在 171 落地，contact metadata 保留原區間；171 後身體繼續回穩，不增加 idle hold。局部數值與畫面入口見 pitcher README／environment。

Michael＋Julia 已完成 A／B／C human review；正式 pitcher 三檔原樣升至 S0.2C，Right-handed Pitcher S0 的單一 pitch clip authoring motion baseline 通過。這只代表 Blender／GLB authoring baseline；app runtime animation、release integration、dynamic occlusion 與 early-flight readability 尚未驗證，正式遊戲品質與 M1 尚未完成，S1 未開始。 `review/s02c/` 保留為歷史 human-review artifact；此次 promotion 不改 motion／timing／資產契約。


## S1：Static Pitcher GLB Runtime Import（2026-09-16）

本輪授權並實作單一正式 pitcher GLB 的 static bind/rest mesh transport，取代 procedural pitcher；Michael＋Julia 已接受 S1 的基本位置／尺度／左右／顏色／grounding 與 scene integration，renderer 無須擴張。S0 authoring motion 已接受，S1 不套 frame 1 或假造 Ready，不執行 animation／skinning。App 的 Ready 是球的 simulation state，角色本身保持 bind pose。

- Engine `rendering/static_glb` 只處理實際需要的 GLB container／primitive、POSITION、COLOR_0、indices 與 parent-composed static node transforms；回傳 owned triangles／node 資料，不含棒球語意。現有 Pawapuro caller 是此能力的直接需求，沒有 asset manager 或 scene graph runtime。
- Pawapuro `batting/pitcher/static_pitcher` 指定 mesh／grip 契約，負責一次 X reflection、一次 winding reversal、scale=1 與既有 staging pitcher placement。公尺已在 authoring 烘好；`height_m` 不再縮放 asset，`pitcher.toml` 不加入 runtime 第二份 placement truth。Bind grip 只作方向／hierarchy 檢查，不連接球。
- File bytes、cgltf parse tree 與 accessor 暫存僅在同步讀取期間存活，cgltf tree 先於 backing bytes 釋放。回傳資料自有；pitcher vertices 複製至 scene 後即釋放暫存，scene 活到 main scope 結束。既有 D3D12View 初始化複製至 immutable vertex buffer，renderer 擁有 GPU resource 並於 fence 完成後釋放，沒有借用 parser pointers。
- Imported pitcher 是 ordinary world vertices；ball translated range 與 overlay range 不變，static batter／camera／field／Native simulation 保留。缺檔或不支援的顯示格式直接 startup fail，包含 source path、owner 與原因，沒有 procedural fallback。D3D12View／HLSL 不改。

實際 subset／啟動入口見 [pitcher README](../../pawapuro/batting/pitcher/README.md#s1-static-bind-pose-runtime)，依賴與本輪驗證見 [environment](../development/environment.md#s1-static-pitcher-glb-runtime-import2026-09-16)。Runtime animation／skinning、rubber-arm 動態造型、release／ball attachment、dynamic occlusion 與 early-flight readability 尚未驗證，M1 未完成。下一技術切片 S2 尚未授權／開始，S3 未開始。後續 animation authoring 的動作關係與 Motion Brief 由 [Character Motion Rules v0.1](character-motion.md) 擁有；該文件待 design review，並記錄延後至真正 batting camera 播放動畫後再評估的 style debt。
