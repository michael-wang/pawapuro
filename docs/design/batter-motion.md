# Batter Motion S0 — Reference Study & Motion Brief

2026-09-16｜Motion Brief 已獲 Michael＋Julia review accepted。Batter Authoring S0 已獲 Michael＋Julia human review accepted；Batter Runtime S1 static bind import candidate 待 human review。Runtime animation／collision／contact presentation 尚未開始。

## Reference scope／provenance

唯一主要 reference 是 Michael 提供的 [ref-batting-perfect.mp4](<C:/Users/USER/Videos/螢幕錄製內容/ref-batting-perfect.mp4>)。不替換成其他投打影片，不取用遊戲的模型或 animation data；只分析打者、以接觸閃光與鏡頭離開作時間標記。

| 項目 | 本次確認 |
|---|---|
| Container／codec | MP4（`mp42`）／AVC（`avc1`） |
| Capture | 2874×1612，30 fps，350 frames；MP4 sample time table 為 350×1000／30000 s |
| Playback provenance | Michael 補充確認：**YouTube 0.5× playback 後螢幕錄影**；此資訊來自提供者，不是 decoder 可推導的原遊戲 FPS |
| Duration | Video track header 11.666633 s；350 格以 30 fps 顯示約 11.666667 s，差異只是 header 時間單位，不是額外動作 |
| Index／timestamp | 本文件全部採 **0-based capture frame**；f0=0.000 s，時間為附件播放時間 `frame / 30`，不是 Pawapuro authoring frame |
| SHA256 | `a010199e0f67b806b8dd03340c1c7578cc14e674bbca5dab8566ea2a48356b88` |
| 已實際檢視 | 全片每秒總覽；f105–195 的 swing 區域解碼，重點連續檢視 f137–178；f180–245 的收勢抽樣，f244／245／246 核對鏡頭離開邊界 |
| 可分析區段 | Ready 可見於片頭；主要準備／短步／swing 約 f135–171（4.500–5.700 s）；follow-through／finish 至 f245（8.167 s）；f246（8.200 s）離開打者 |
| 視角限制 | f0–171 為捕手後方偏置視角，f172 起切入打者 hero 視角；不能跨 cut 比 screen-space 速度或直接把畫面位移當 world translation |
| 工具／觀看界線 | 既有 Blender 4.5.13 VSE **僅解碼影片**，Pillow 排 contact sheets；沒有載入／保存 character .blend 或做 authoring。實際完成逐格影像檢視，未進行連續 1× 播放觀看或音訊聆聽 |

30 fps 只證明附件 capture cadence；已確認錄影時 YouTube 播放倍率為 **0.5×**。以下 timestamp／interval／秒數全部只描述這份半速 capture（檔案本身以 1× 播放），不代表原遊戲 authoritative timing；也不直接乘倍率就當作 animation 規格。原遊戲 simulation／animation FPS、上傳素材內部是否另經變速及重複取樣原因仍未知。保留 phase ordering、spacing 與 relative speed contrast；不因 provenance 補充重做整份研究。

**造型核對：實際 reference 已是大頭、球形手與 detached shoes 的 Q 版角色，並非真人式角色。** 本文不依授權文字的該項描述補入不存在的 anatomy。

### Handedness 與 body-relative 詞彙

影像判讀為 **左打**：綜合完整 Ready 畫面的本壘／投手位置、打者站區、背面球衣與轉身後揮棒方向，站在左打區，右側為面向投手的 lead side。這是影像判讀，未讀取原遊戲角色設定；沒有把 HUD 的「右」或單純 screen right 當成打擊慣用手證據。

與 Pawapuro 的 left-handed batter 目標一致，**不需把這支 reference 反向鏡射**：lead/front foot=右腳、rear foot=左腳。後文先以 lead／rear 描述，不照抄像素座標；未來若另獲 right-handed 素材，需 mirror 支撐／開身／器材 trailing 關係，不能只 mirror 截圖。

## Timeline：Source Observation 與 Motion Interpretation

Load 與 Stride 在這支素材中重疊，沒有可靠分離的長蓄力段；仍分列事件以便下一輪 authoring 說清楚因果。下表的 plant／lead 是由畫面判讀，並非讀到原作骨架或接地狀態。

| Event | Capture frame／timestamp | Source Observation：實際可見 | Motion Interpretation：本輪判讀 |
|---|---|---|---|
| Ready | f120／4.000 s（片頭已有） | 棒接近直立，雙球形手聚於頭／肩旁，鞋接近並部分重疊；軀幹只有小幅變化 | Compact ready；長等待不是整段 load |
| Load 開始候選 | 約 f135–149／4.500–4.967 s | 小幅軀幹／鞋間關係變化；直到 f149 前後前鞋才清楚與後鞋分離 | 可能先卸前腳、保留後腳支撐；**不能可靠指定一個獨立 load 起始格**，也沒有大抬腿證據 |
| Stride／lift 明確 | f149–158／4.967–5.267 s | 前鞋從重疊輪廓移出，向投手側短移；f156–158 可見鞋底與地面之間的間隙；手仍靠近頭側 | 小幅 lift／短 stride，rear support 為主，身體與腳共同蓄力而非大跨步 |
| Front-foot plant 候選 | f158→159／5.267→5.300 s | 前鞋降低到地面附近，前移大致停止；後鞋開始換朝向，手／棒降至側後方 | 把 f159 視為可讀 plant 的第一個候選 sample；精確接地 tick 無法由單視角確認 |
| Turn／bat lag | f159–163／5.300–5.433 s | 手在頭下／身旁，棒頭仍留在手的後上方；後鞋鞋底逐漸露出，球衣／肩線開始改向 | 支撐承接開身，hands／bat 先保留 trailing；pelvis 被球衣遮住，**不能證明骨盆比胸口早幾格** |
| Torso 明顯轉動／主要 bat acceleration | f163→166／5.433→5.533 s | 球衣背面／肩線轉向，棒從斜後方跨到身前，遠端 spacing 突增；後鞋 heel 抬起／pivot 輪廓更明顯 | 主要爆發集中於此，身體帶手，棒頭快速追上；不是全段等速搬手 |
| Contact-area 候選 | f164–170／5.467–5.667 s | 棒掃向本壘附近；f166–170 棒呈較接近水平、近似延伸姿勢 | 可作「高速穿越未來可擊球區」的路徑線索，不能從投影推出正式 3D collision truth；近似停留原因未知 |
| Visible contact marker | f171／5.700 s | 首見明亮 contact flash；打者仍在延伸姿勢 | 只作 presentation 時間標記，**不把 flash 的格數等同球棒實際接觸時刻** |
| Early follow-through | f172–180／5.733–6.000 s | 鏡頭變更後，手／棒繼續上繞，軀幹持續轉；可見後鞋 pivot／鞋底，沒有 contact 後全身立刻停住 | 動勢由穿越延伸轉入繞身上收，不能以 cut 隱藏缺少的承接 |
| 大幅／hero follow-through | 約 f180–195／6.000–6.500 s | 棒沿頭／肩上方繼續繞行，f186 接近橫過頭側，f195 再轉斜；頭／軀幹與鞋朝向仍變 | 寬的 follow-through arc 消化動勢；單視角不能指定唯一「3D 最大旋轉」格 |
| Recovery／last visible finish | f204–245／6.800–8.167 s | 棒逐步降到肩側，鞋前後分開；一鞋仍露出斜鞋底，最後數格身體變化小；沒有回到 Ready | 是可讀 finish／hero settle，不是已證明恢復雙腳平底的 ready。需保留動勢收束，不能用靜止尾段補長度 |
| 鏡頭離開打者 | f246／8.200 s | 畫面改為外野；上一格 f245 仍有打者 | 後續 recovery 不可見，不補寫額外落腳／回 Ready |

### Bat speed／timing：可用的關係與不可用的數字

**以下 0.333／0.133／0.100／0.400／2.467 s 均為 YouTube 0.5× screen recording 的 capture 時間。** 原 frame index 與觀察不變，不能直接作為 Pawapuro clip duration／event timing。

- Ready 的長等待（片頭至約 f147）不能算作 load；load 太微小，無法可靠獨立量出時長。可確認的短步約 f149→159，**10 個 capture intervals／0.333 s**，與 load 重疊。
- f159→163 約 **4 intervals／0.133 s**：棒已放低但遠端仍留後，提供緊湊的 lag 輪廓。
- 最大可見 forward sweep 約 f163→166，**3 intervals／0.100 s（包含 4 個端點影格）**：斜後方 → 跨身 → 身前延伸。這是畫面上方向／spacing 的集中改變，沒有估算 m/s，亦不指定 Pawapuro 必須用 3／6 格。
- f166–170 近似延伸姿勢不能略掉；其後 f171 才見 flash。不能把整個 f159→171 的 0.400 s 都說成均勻高速，也不能把 near-hold 自動診斷為 hit-stop。已知 YouTube 0.5× 播放，但 near-hold 是否還包含錄影重複、素材內部變速或原作演出，仍未知。
- f172–195 延續大 arc；約 f204 以後可見 spacing 縮小、棒逐步 settle。從 flash f171 到最後打者 f245 相隔 **74 intervals／2.467 s**，包含收勢與 hero presentation，不全是快速動作。f172 已 cut，故不跨視角量 bat pixel displacement。

建議保留的 presentation 關係是 **compact preparation → short step／plant → 器材留後 → 短促 forward sweep → 較長的繞身／settle**。Capture 中的疑似 hold 不列為 authoring 必須照搬的 motion；下一輪以無特效 1× review 判斷，不預定總長。

### Head／hands／feet 的觀察限制

- **Head／gaze**：Ready 到 stride 的 helmet 朝向相對穩定；f163–171 球衣轉向更明顯時，帽沿仍大致朝來球／本壘側，頭沒有與 torso 一起完整甩轉。眼睛多被視角遮住，「看著 contact-area」是意圖推論，不是 eye tracking。f178 以後頭傾斜／朝向與 finish 共同改變，但 camera cut 使「何時開始追結果」無法精確定格。
- **Shoulder → hand → bat**：手先 compact 留在肩／身旁，f159–163 bat barrel 落後，f164–166 快速穿過。可判讀分段 lead／lag，不能由這些輪廓證明整套 anatomical kinetic chain。球衣腰線只作 body turn cue，沒有把它當 pelvis transform。
- **Attachment**：Ready／加速可見兩球形手聚在握把，不需 fingers。收勢有遮擋，無法確證後段有沒有單手離棒；不新增 bat release 語意。下一輪以連續持棒為首版候選，避免因看不到另一手就讓 bat 自行漂移。
- **Feet**：前腳短步、plant 後後腳 heel-up／pivot 可讀；鞋重疊與 hero camera 使 world-space 接地／零滑動無法量化。沒有證據要求後腳另跨一步，也沒有證明最後 rear sole 必須完全壓平。這不是 foot articulation／IK 的需求。

## Supplemental 真人 reference：只補 pitcher-relative preparation timing

Michael 提供 [20260916-0648-23.6760813.mp4](<C:/Users/USER/AppData/Local/Packages/Microsoft.ScreenSketch_8wekyb3d8bbwe/TempState/Recordings/20260916-0648-23.6760813.mp4>)，不是取代上面的主要遊戲 reference。實測 decoder：MP4、1456×1196、30 fps、143 格／約 4.7667 s；SHA256 `a5d959b52674e5174d821e18069648aac1b077597daff018a159a0cfb97f5aa4`。補充影片的原始拍攝 FPS／播放倍率未提供，**不把遊戲影片的 0.5× 自動套到這支影片**；時間仍只標 capture。

視角是投手後方朝本壘的轉播畫面，投打者同時可見；前景投手部分遮住打者下肢。本次實際檢視全片總覽定位，解碼 f10–70，檢視 f10–48 隔格及 f46–70 連續影格；只讀到主要 bat acceleration 的起點作 plant 順序核對，不研究接觸／收勢。沿用既有 decoder，未進行 character authoring；本次也是影格檢視，沒有宣稱正常速度連續觀看。所有 f／秒數只屬於本 supplemental，不能與前節遊戲 f 合併。

| Supplemental capture | Source Observation：同一影格中的投手／打者 | Michael／Julia authoring interpretation：本角色候選 |
|---|---|---|
| f20–28／0.667–0.933 s | 投手抬起前腿至高點；打者雙腳仍近地、棒維持肩後，只有小幅準備變化 | **支持** early pitcher leg lift 時 batter mostly quiet／ready；quiet 不等於全身 freeze |
| f30–38／1.000–1.267 s | 投手抬腿下降、身體開始前移、雙手分開／投球臂往後帶；打者降低重心並開始前腳卸重，抬腳輪廓在 f38 前後變清楚 | **支持**較晚 gather 與 pitcher commits forward／arm loads 相對應；不是投手一抬腿就同步大幅啟動 |
| f40–48／1.333–1.600 s | 投手繼續 stride／投球臂進入前甩準備；打者前膝明顯升高、後腳仍在地，手棒保留在後側 | 明顯 lead-foot lift 屬同一 gather，不能把 detached foot 做成獨立裝飾動作 |
| f50–58／1.667–1.933 s | 投手手臂前甩、身體前折；f56 可見球已離開投手。打者前膝在 f50–54 仍高，f56–58 開始明顯下降展開 | **部分支持並需修飾**「approaches release → stride／準備 plant」：是出手前後的轉換窗口，不能寫成 release 前已完成 stride 或 plant，更不能逐格同步綁定 |
| f58–64／1.933–2.133 s | 打者前腳下降／展開到地面附近，約 f60–64 已可讀成建立支撐，棒仍主要留後；精確 sole 接地被投手遮擋 | Plant 在主加速前成立的可讀關係受支持；這是區間判讀，無法指定 exact contact tick 或量測零滑動 |
| f64→65／2.133→2.167 s | 前腳已在低位支撐輪廓，下一格手棒向前的 spacing 明顯增大 | **支持 plant before main bat acceleration**；不推出世界速度、碰撞時刻或 pelvis／chest 的精確發動差 |

**結論：支持 late-gather 候選 style，但第三項只有出手前後的相對區間證據。** 這是此支 reference 與 Michael 對未來 Pawapuro 左打者的選擇，不是所有現代打者的通用規則。Brief 以 lead／rear 與 pitcher phase 表達，未把真人腳抬高幅度、解剖姿勢或 capture frames 搬成 authoring 規格；抬腳幅度仍待下一輪 review。它也不是 runtime trigger／同步控制設計。

Evidence 留在 ignored `build/batter-motion-s0-study/supplement/`：`pitcher-relative-timeline.jpg`（同格投打關係）、`gather.jpg`（f10–48）、`plant.jpg`（f46–70）。均標 0-based capture frame／timestamp；source clip 不複製或 commit。中斷前僅讀文件與影片目錄，沒有 supplemental 驗證或未提交修改；上述觀察均在本次收到正確附件後完成。

## Motion Brief：Left-handed Full Swing 候選

以下是 **Pawapuro authoring intent／可刻意加強的可讀性**，不是宣稱 reference 逐項證明的內部 mechanics。沿用既有六條 [Character Motion Rules](character-motion.md)；不增加 anatomy、stretch ratio 或新 solver。

| Phase | Support / Contact | Body Intent | Lead | Lag / Trailing | Attachment | Momentum Next |
|---|---|---|---|---|---|---|
| Ready | 雙腳有穩定地面關係，rear 左腳可承接 load | Pitcher early leg lift 時 mostly quiet／ready、注意投手 | 視線／準備意圖 | Bat 保留在後肩附近 | 球形雙手讀成握住同一把手 | 轉入小幅 load，非全身僵直 |
| Load | Rear 支撐，lead 右腳逐漸卸重／抬起 | Pitcher commits forward／throwing arm loads 時才明顯 gather | Body load 帶前鞋 | Hands／bat 保留 compact | 握把隨手群，不漂移 | 接短 stride，不停在蓄力 pose |
| Stride | Rear 支撐，lead lift→短移→下降 | Pitcher 接近 release 至出手前後展開、準備 plant；不要求 release 前已落地 | Body intent／lead foot 同步協調 | 胸口與 hands 保留短暫落後 | Bat head 留後，手不先外拋 | 前腳 plant 接住前移 |
| Plant / Turn | Lead 在 main bat acceleration 前建立支撐；rear 可卸 heel／pivot | 前移轉為開身旋轉 | Pelvis／torso 的較大轉動（需下輪證明） | Hands 留身旁、bat barrel 更後 | 握把 path 連續 | 以 turn 帶出 arm／bat acceleration |
| Bat Acceleration | Lead 穩、rear 隨 turn 轉，不任意滑走 | 爆發集中 | Shoulder／hands 被 body 帶過 | Bat 遠端短暫 lag 再快速追上 | 手與把手同一路徑 | 快速穿進 contact-area，不在前方煞停 |
| Contact-area | 支撐繼續承接轉動 | 穿越可擊球區，不把 contact 當目標停格 | Hands／bat sweep | 身體較慢節奏仍有重量 | Bat endpoint 清楚，無碰撞假定 | 無論有無球都接完整 follow-through |
| Follow-through | Lead 承接，rear pivot／unload 需可讀 | 軀幹與手棒繼續繞身上收 | 既有動勢續行 | Arm／bat 可比 torso 晚 settle | Rubber arm 保持連續 curve，bat 仍被手帶動 | Arc 漸收、頭開始跟 finish |
| Recovery / Finish | 雙腳位置能解釋平衡，rear 可保留 toe support | 收束成穩定 finish，不要求返回 Ready | 身體先減速 | Bat／hands 最後小幅 settle | 不新增放棒／換手事件 | 動勢確實消散，再 hold 最終 pose |

為 Q 版可加強的是 **plant 與手棒 lag 的輪廓差、短促加速與慢 settle 的對比**。不可用任意伸長手臂、穿頭／穿帽、把 bat 瞬移或讓 detached shoes 各自漂移達成；手與棒 endpoint 必須清楚。大頭只保留注意力與適量後續轉動，不新增 huge-head balance law。

## 下一輪 Batter Authoring S0 的 human-review criteria

1. 先看無球、無 flash／sound／camera cut 的完整 1× swing：動作本身是否有力量？固定視角下也應成立。
2. Ready／Load compact 且可區分；pitcher early leg lift 時大致 quiet，commits forward／arm loads 時才明顯 gather／lead-foot lift。這是本打者候選 style，不是通用法則；抬腳幅度另驗，不照搬真人。
3. Rear support → lead lift／short stride → plant 可讀；pitcher 出手前後展開，main bat acceleration 前建立 plant，不硬鎖 plant=release。Planted shoe 不滑，身體前移與 detached feet 屬於同一意圖。
4. 大 body turn 有帶動手棒的因果；pelvis／chest／hands 不只是同一曲線乘不同振幅。Reference 不足以證明的 pelvis timing 由本輪新 authoring 的畫面驗收。
5. Hands 靠身、bat head 留後再追上，有清楚 lead／lag；不靠診斷箭頭才看懂。
6. 爆發集中在短段，1× 能讀出加速對比，逐格仍是連續 arc、沒有 teleport；不照抄 0.5× 遊戲 capture 或倍率未知的 supplemental 秒數。
7. Bat 持續穿越 contact-area，沒有球也不中途煞停；不把視覺接觸標記冒充碰撞 truth。
8. Torso、head、hands／bat 與 rear foot 各有後續節奏；完整收勢後才 settle，不能靠加尾端 hold 假裝有 recovery。
9. 超大頭不隨 torso 剛性甩成陀螺；rubber arm 輪廓保持 continuous elastic action line，手／把手關係可信。
10. 以左打 body-relative 關係檢查 lead=右／rear=左；先驗單一 full swing；本次 authoring 已授權，runtime／bat-ball collision 尚未授權。

## Future Contact Presentation Notes

- 直接可見：f171 contact flash，f172 換成 batter hero 視角；一直保留打者至 f245，f246 才離開，並非 perfect contact 後立刻追球。
- Michael 提醒 flash／文字／camera／sound 共同放大 contact。影格確認前三項存在；本次未聆聽音訊，sound 的具體效果不列為已查證。
- 「球的 gameplay truth 可能已在前進，presentation 仍留在打者」是未來可用的分離意圖，不是影片證明的原作 physics 實作。沿用 S3 已接受的 gameplay completion／presentation completion 分離思路。
- 不在此定 camera API、hit-stop duration、VFX／audio architecture、碰撞或球路。

## Open questions／review gate

- **已補足（Michael provenance）**：遊戲影片是 YouTube 0.5× playback screen recording，先前 capture 秒數不再標倍率未知；這項不是 supplemental 影像驗證所得。
- **已補足（supplemental observations）**：原 brief 缺少投打同步準備的直接證據；本片支持 early leg lift 時 mostly ready、投手前移／arm loads 時 late gather、plant 先於主加速。出手附近的 stride／下降只支持一個跨 release 的窗口，不支持 exact phase locking。
- **仍未知**：原遊戲 authoritative timing／FPS 與 f166–170 near-hold 原因；supplemental 的原始拍攝／播放倍率；精確 foot-contact tick、world-space 零滑動、Pawapuro 應採的 lift 幅度／phase 時長。沒有直接將 capture 秒數換算成 authoring keys。
- Pelvis 相對 chest 的精確發動差、精確 plant tick、收勢另一手是否放開與最終 rear-foot 接觸狀態，單視角／遮擋不足以查證；首版候選不為這些未知新增系統。
- **Brief 與 S0 authoring baseline 已接受**：compact Ready、late gather、lift／short stride、plant before acceleration、body turn→hands／bat lag、短促加速、contact-area pass 與完整 finish 均獲 Michael＋Julia 接受。下一 gate 是 Runtime S1 static import review，不是 swing playback。
- **不需新增 Character Motion Rule**：support／detached coordination 是 Rule 1／2，器材 lag 是 Rule 3，elastic arm 是 Rule 4，持棒是 Rule 5，follow-through／finish 是 Rule 6。此 caller 沒有暴露無法涵蓋的新 Motion Truth。

Evidence：ignored `build/batter-motion-s0-study/` 的 `reference-timeline.jpg`、`pre-contact-sequence.jpg`、`contact-area-sequence.jpg`、`follow-through-hero-sequence.jpg`，全部標 source timestamp／0-based frame；crop 僅為看清打者，cut 前後仍是不同 source camera。原片不複製進 repo，影格／解碼腳本／metadata 留在 build、不 commit。

上述 reference study／amendment 只修改文件，沒有 production code／asset 變更；**未執行 C++ build、CTest 或 GPU validation**，未做 Blender authoring、rig、keys、GLB、runtime 或碰撞。


## Batter Authoring S0：已接受的 first baseline

可編輯 source、GLB／TOML、操作與完整驗證由 [batter README](../../pawapuro/batting/batter/README.md) 擁有；不重做 reference study。本次 `ref-batting-front.mp4` SHA256 與上列 supplemental 相同，使用該持久路徑；遊戲 reference 仍是 YouTube 0.5× capture，沒有將其秒數搬成 keys。

本次 authoring frame **1-based**、60 fps／fps_base=1，delivery start=0，單一 non-looping `swing_L` 至 f225／3.733333 s。Ready 保持安靜至約 f65，明顯 gather f72／1.183 s 對應 pitcher commit-forward；右前腳 f88／1.450 s 抬至鞋底 0.32 m，f99／1.633 s 展開短步，總前移 0.15 m。f107／1.767 s 建立 plant，晚於 pitcher release guide，早於主要 bat acceleration。

Pelvis 先打開，chest 另有落後的曲線；手棒 f107–116 保留靠身／barrel 留後，約 f116–124 集中 sweep。`contact_area` f121／2.000 s 是 authoring-only 代表 pass，barrel 自然穿越目前 zone，非 gameplay contact truth。f134／2.217 s 接繞身、f165／2.733 s 後持續 settle 至 f225；head 不剛性跟胸，後腳 f115 起 toe pivot／heel lift。Grip 全程持棒，沒有兩手 solver／bat release。

這些是 **本角色第一候選的 authoring interpretation**；不是 reference 證明的精確 pelvis/chest tick 或通用打擊規則。Source 的 lift 幅度／phase 時長現在有實際候選，原影片 timing、精確足底接觸與其 near-hold 原因仍未知；Michael＋Julia 現已接受本版 lift、lag 與 settle；原 reference 的未知仍保留。

Evidence：ignored `build/batter-authoring-s0/` 的 `swing-batting-1x.mp4`、`swing-side-1x.mp4`（完整 60 fps／1×）、兩視角 contact sheets、plant→acceleration f103–126 連續影格、hands／feet close-ups、`motion-diagnostic.jpg`。首格與 source 預設均為 f1 Ready。已解碼核對 FPS／225 格，已檢視影格及全 source 數值；未宣稱完成正常速度連續自看。

已知問題：簡化 torso 轉向輪廓偏淡；下巴附近 rubber arms 疊影、早期 follow-through 的手棒被大頭遮住。支撐、匯出與 attachment 檢查通過不代表動作力量感已過關。Khronos 0 errors、全 225 格 source／GLB／round-trip 通過；細節及容差見 README。未執行 C++ build／CTest／GPU，沒有修改 production、pitcher 或 staging；六條 Motion Rules 不變。其後 S0 baseline 已通過 Michael＋Julia human review；此 acceptance 不包含 runtime animation、collision、hit quality、contact presentation 或 M1。S1 static import 的實測與 gate 見 [batter README](../../pawapuro/batting/batter/README.md#batter-runtime-s1--static-bind-import-candidate)，source 三檔未重新保存／匯出。
