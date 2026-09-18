# M1：Batting Feel

更新：2026-09-16。狀態：M1 Step 1 交付 1 已實作並完成靜態場景驗證；另已完成靜態 staging calibration／啟動 Data；交付 2 的 reference flight／pause／single-step 已完成，本輪已加入交付 3 的同球重投及進壘驗證畫面，Step 1 整體仍待使用者 review，M1 尚未完成。層級與時間契約以 [Batting Feel 設計](../design/batting-feel.md) 為準。

## 完成的定義

**玩家經歷投手動作、來球、揮棒、接觸、擊球飛行與全壘打後，會主動想再打一球。**

完成需要同時通過體驗驗收與可靠性驗收。能顯示球、命中球或跑完所有 implementation steps，都不足以宣告 M1 完成。這是一個可玩、可調的 Windows 3D 投打短循環，不交付通用引擎產品。

以下控制方式、測試人數與數值門檻是可 review 的初始提案，不是已驗證事實。若要改驗收方法，先記錄理由與新條件再測，不在看到結果後改門檻來宣告成功。

## In scope

| 範圍 | 本次最小交付 |
|---|---|
| 角色與操作 | 一名投手、一名打者，先固定同一左右手組合。玩家扮演打者，投手由 scenario 控制。鍵盤／滑鼠：簡單接觸平面瞄準＋單鍵揮棒；揮棒接受時鎖定本次 aim。Controller、集氣與多種揮棒模式延後。 |
| 投球 | 一種直球與一種可辨識曲球／變化球預設，使用可調 Native 模型；同一動作至少可比較兩種節奏。保留速度、spin 與出手位置的可追查因果，不做完整球種庫。 |
| 動作 | 投手抬腿／停頓／跨步／旋轉／出手可讀，球從手部正確釋放；打者啟動、加速與 follow-through 連貫。用最低可用 Q 版比例 mesh／rig／clip，無完整角色製作系統。 |
| 擊球 | 空揮、擦碰／弱擊、扎實擊球及 sweet-spot 接觸可以產生；接觸品質與飛行結果分開記錄。好 timing 不保證好球，沒有接觸不產生命中。 |
| 場地與飛行 | 一個簡化球場，有地面、本壘、界線、可辨距離的地標與外野牆。玩家能看見弧線、落地、撞牆或越牆；無守備與跑壘。 |
| 全壘打 | 界內、落地前越過有效外野牆且高於牆頂才成立；需有實際越牆證據。短 camera／聲音演出可跳過，不能只用 distance 數值判定或每次好擊都強制 HR。 |
| Feedback | 空揮破風聲與延續動作；弱／扎實／sweet 接觸的聲音、畫面與 camera 反應可辨；飛行視角與 HR 高潮。正常遊玩不用 debug HUD 也可判讀。 |
| 反覆試打 | 一個練習 scenario，可重投同球或下一球；一個明確操作可略過結果演出並回到準備，不要求重啟程式。 |
| 開發回饋 | 具體調參面板、Data／Lua／本次資產的安全 reload、pause、single-step、碰撞／軌跡檢視、每球紀錄、同版本 replay 與來源定位。 |

接觸平面 aim 是控制提案，第一個切片先固定 aim，只檢驗 timing；在接觸穩定後才加入二維瞄準。音效、動作和速度感須在可玩循環中提早試，不集中留到最後當 polish。

資產仍未指定。動作與音效取得是本次交付依賴，後續需列明製作／可合法使用的來源、Blender 匯出版本與最小 sample。灰盒能解鎖開發；不能替代最終動作與聲音驗收。若缺資產，標記對應階段未通過，而非刪掉它。

## Out of scope

完整球賽、計分局數、隊伍管理、守備 AI、跑壘、投手玩家操作、球種全集、生涯／對話／local AI、連線、跨平台、正式美術品質、大量球場與球員、完整動畫 graph／IK／retargeting、攝影機編輯器、asset database、通用碰撞 solver、ECS、job framework、全自動 reflection／bindings、native DLL 任意狀態 hot reload、Jai migration。

少量必要的場地結果規則可實作；它們不代表開始建立整套棒球規則系統。只有眼前出手／揮棒需要的動畫能力進入排程。

## 體驗驗收：關掉 debug HUD 後成立

先由開發者進行因果與參數實驗，再做小型實玩。建議最終至少 5 名測試者，其中至少 3 名未參與實作；先記錄棒球／棒球遊戲熟悉度。這是產品決策的質性證據，不是統計上的普遍保證。

流程：短操作說明與 10 球熟悉 → 10 球正式試打 → 告知必做測試已結束，可自行停止或繼續。不要要求或獎勵玩家為達門檻而多打。Debug 強制完美命中、固定結果演示不算自主試打。

| ID | 要驗證的體驗 | Review 時需要的證據與通過提案 |
|---|---|---|
| F1 | 出手前開始判讀 | 兩組動作節奏，使用相同釋放後球路，避免球速混淆。至少 4/5 能說出使用的動作線索，並辨認／指出出手節奏差異；錄影確認沒有靠 UI 倒數提示。 |
| F2 | 來球壓迫而可追蹤 | 直球兩種速度與變化球交錯。至少 4/5 能辨認明顯速度／彎曲差異，且未反覆因球消失或鏡頭遮蔽失去球；指出一個可改善的判讀線索。 |
| F3 | 空揮也有力量感 | 讓玩家自然揮空並另作數次練習揮棒；至少 4/5 對輸入反應與力量感給 4/5 以上，沒有必須擊中才有 feedback 的情況。 |
| F4 | 接觸是核心高潮 | 在受控補充案例中呈現弱／扎實／sweet 接觸，隱藏數字與品質文字，至少 4/5 能區分弱擊與 sweet，且對 sweet 滿足感給 4/5 以上。受控案例可確保看到效果，不計入自主續打。 |
| F5 | 球路本身可讀 | 五個固定且易區分的擊球案例，涵蓋短落地、長飛、撞牆、越牆；球未落地／碰牆前請玩家預測。至少 4/5 各答對 4/5 案例；沒有用結果標籤洩漏答案。 |
| F6 | 全壘打釋放情緒 | 受控案例確保每人看到 HR；至少 4/5 能看懂實際越牆且對高潮給 4/5 以上。演出可跳過，下一球操作不被鎖住。 |
| F7 | 想再打一球 | 必做結束後至少 3/5 自願再打至少 5 球；記錄理由與實際行為。至少 3/5 能描述下一球想改善或再次追求的具體體驗，而不只是完成測試。 |

六階段與續打意願都需達標，並由專案負責人確認整體節奏成立。小樣本數字只作本次 gate，不替代訪談；若某一段未過，保存失敗案例，回到該段做單一變因比較。早期階段不必等湊滿五人才能調整。

A/B 比較同一組輸入／投球 preset，一次只改一類變因（例如 bat acceleration、接觸音色或 camera kick）。交換展示順序，控制播放音量，記錄版本與順序；不用同時增加球速、音量、camera shake 來推測哪個改動有效。

## 可靠性與開發驗收

| ID | 驗收標準 | 最小驗證 |
|---|---|---|
| T1 | 同一輸入的結果與 render frame rate 分離 | 同一 build／平台／資產／resolved data／seed，重播至少 10 個案例，各 10 次，於 30／60／120 render FPS 下比較。接觸 tick、事件順序、Native state 結果一致；輸入使用已錄 tick 命令，不重新依 render frame 取樣。 |
| T2 | 出手與球棒可見位置對得上判定 | Pause／single-step 疊加 hand release、bat collision shape 與 contact point；兩組節奏及所有本次 clips 都沒有獨立 timer 漂移。插值誤差可解釋，不用移動碰撞體去追隨 render-only camera。 |
| T3 | 在宣告支援的球速／球棒角速度範圍內無穿透與重複命中 | 固定 fixture 覆蓋高速、旋轉球棒、擦邊、棒頭／棒尾、空揮、sweet spot；檢查跨 tick 接觸及最早有效接觸。與更細子步參考比較，初始容差提案：exit speed 差 ≤1%、launch angle 差 ≤1°；接觸／未接觸與 HR 類別不得翻轉。邊界案例另標示數值敏感性。 |
| T4 | 物理與回饋一致 | 一次有效接觸恰有一筆接觸事件及一次聲畫觸發；空揮沒有命中聲。幾何接觸／品質／飛行結果分開記錄；perfect timing 不繞過碰撞。HR、撞牆、先落地、界外與牆頂邊界各有可重播案例。 |
| T5 | 改值能快速重新試 | 球速、spin、drag、COR、bat speed、sweet spot、timing window、release timing、camera 均可改。合法 Data reload 在按 Reload 後 1 秒內顯示已驗證／pending；用新設定重投不需 native build，發布時間與本球版本可見。 |
| T6 | Reload 不破壞正在工作的版本 | Data 語法／範圍錯誤、Lua 語法／API 錯誤、asset 不相容／載入失敗均留舊版並定位來源。Scenario 與影響 simulation 的 clip 下一球才換；純 camera 值下一 frame 可見。明確 Save／Revert，不無聲改原檔。 |
| T7 | Asset／script reload 足夠用於本次迭代 | 至少一個 scenario、一個實際揮棒或投球 clip、一個 mesh／texture 變更可不重啟套用。記錄 fixture 大小；初始目標是 scenario 1 秒、最小動畫／mesh fixture 2 秒內準備完成，不含等待下一球的時間。Pending／failure 可見，舊 GPU generation 不提前釋放。 |
| T8 | 調整與異常可追查 | 每球面板顯示 Native module／build、Lua path／scenario／版本、Data path／key／override／resolved 版本、asset ID／generation、事件時間。Pause、single-step、軌跡、collision shape、法線與 bat path 可用；UI 關閉仍能遊玩。 |
| T9 | 短循環與狀態清理 | 結果或 HR 演出時一個操作後 1 秒內回到投球準備；完整 windup 仍保留。連續 100 球，不留上一球音效／camera／input／contact 狀態。重複 reload 後資源存量回到預期穩態，無逐球無界增長。 |
| T10 | 低延遲與 frame time 有記錄 | 指定測試 PC、GPU／VRAM、driver、音效裝置、解析度、同步方式與 build 後再驗收。初始目標 1080p／60 FPS：暖機後連續 100 球 p95 frame time ≤16.7 ms、p99 ≤20 ms，GPU p95 ≤12 ms 保留空間。一般重投、接觸、HR 切鏡納入；明確手動 reload 另列延遲，不混入或隱藏。 |
| T11 | 操作與 feedback 沒有額外流程等待 | 接受的揮棒輸入在下一 simulation tick 啟動，無額外 scenario 等待；contact 後第一個可呈現 frame 發出聲畫請求。記錄 input→tick→render／audio submission；如未量實際顯示／出聲，報告不能稱其為完整端到端延遲。F3/F4 的實玩判斷仍必須通過。 |

T3 支援範圍需在開始接觸實作時由實際 preset 與 data validation 明列，測上下限及組合；不能以未測的任意數值承諾不穿透。步長與容差若需改動，保留原案例及比較。T10 的 reference PC 目前未指定，因此尚無效能通過結論，不能以預期 GPU 規格代替實測。

提交 review 的證據只需一份簡短結果紀錄：build／data／script／asset 版本、reference PC、F/T 勾選與失敗原因、固定案例／replay、短影片、逐球量測及匿名實玩記錄。等真的測試時再建立這份紀錄，現在不建空報表系統。

## 小步 implementation plan

下列順序是逐步開發計畫；目前 Step 0 與 Step 1 交付 1／2 已完成，各後續交付仍須另行授權。每一步先做最小可見行為、展示與檢查，再決定下一步；任何一步太大，就拆成表內的小交付。新增 Engine capability 必須說得出它服務的是該步哪個 caller。

| 步驟 | 玩家／Pawapuro 的真實需求與最小交付 | 留在 Pawapuro 的內容 | 因而需要的最小 Engine capability | 當步檢查／刻意延後 |
|---|---|---|---|---|
| 0. 開工前核對 | 確認現有 compiler／SDK／依賴及操作方式，列出最低動作、音效與 glTF sample 需求 | 場景／控制與資產選擇 | 無新增 capability | 先讀取現況；缺項需另行處理。本輪不安裝；不以此建立包管理框架。 |
| 1. 同一球可看、可重投 | 先畫本壘／出手標記與球；再做明確準備提示→出手→飛到本壘→按鍵重投 | 固定直球的初始條件、簡單重力飛行、準備／出手／結束狀態 | Windows window／input、D3D12 最小 3D 顯示、時間／固定 tick、必要 math、簡單 debug 線點 | 首次可見交付是靜態場景；第二次是運動；第三次是一鍵重投及紀錄。沒有 Lua、模型管線、碰撞或通用 scene 架構。 |
| 2. 球離手前可判讀 | 先用幾何肢體做可讀抬腿／停頓／跨步，再對接一個最低可用投手 clip；球由手部 marker 釋放 | Pitch phase、release 規則、動作／球路參數與兩組節奏 | Caller 真正需要的 transform／pose 取樣；接入 clip 時才做最小 glTF skin／animation、資產 ownership；具體 ImGui 調參與 Data reload | 用同一釋放後球路比較兩種節奏，single-step 查手／球。先支援一個實際資產，不做所有 glTF extension／動畫 graph。 |
| 3. 空揮也有力量 | 先固定 aim，按鍵啟動可見 bat path，再接最低可用打者動作、加速與 follow-through，提早加入破風聲 | Swing phase、bat path／speed、input acceptance、破風聲觸發規則 | 複用 pose／render；引入一個實際需要的 audio playback 路徑及輸入時間記錄 | 完全不需球也能試空揮；先通過輸入反應與 F3 早期試玩，不做 IK 或通用 audio event graph。 |
| 4. 第一次可信接觸 | 先靜止有限球棒 vs 飛球，再加入球棒移動／旋轉、最早接觸、一次回應、真實出球 | Bat-ball 模型、COR、接觸品質及出球計算 | 已明確一般性的 sweep／geometry helper 才進 engine；contact／normal debug draw、pause／single-step | 命中／空揮／高速擦邊 fixture，加最小接觸聲。此時已能重複投→揮→接觸→再投，不等完整系統。 |
| 5. 好擊值得追求 | 加簡單二維 aim，分離球心偏差／sweet spot／timing；調弱擊、扎實、完美接觸，協同聲畫與 camera | Aim→bat motion、sweet-spot 規則、feedback mapping 與 Data | 複用 audio；必要的短 camera offset／FOV 控制與畫面效果 | 一次改一個變因；固定 fixture 加自然試打，確認沒有 hidden auto-hit／加成重複。 |
| 6. 來球有壓迫與差異 | 在已有揮擊的循環中加入 spin／drag、變化球 preset，調 camera／FOV 與聲音 | Aerodynamics／球種公式、preset、難度與速度感設定 | 複用 math／debug trajectory／audio／camera；通常不需新 engine subsystem | 直球仍作對照；調參及 replay 可追查。尾勁先用本次可控模型解釋，不開發完整流體或通用力場引擎。 |
| 7. 看球飛與判讀結果 | 延續同一球的飛行；補地標、牆和界線，先做落地，再撞牆／越牆，調能保留球路的跟拍 | 場地結果、fair／foul、落地先後與 HR 判定 | 只需要的場地幾何顯示／query、camera target 取樣 | 玩家在結果文字前可預測；不用距離直接判 HR。不加入守備／跑壘／比分系統。 |
| 8. 高潮後立即再試 | 加短 HR 聲畫段落與略過；將已證實需要變動的下一球／camera sequence 移到小型 Lua scenario | Scenario 選球、流程轉換、事件消費與短 camera sequence | 最小 Lua 載入／錯誤回報；已存在的 command／event 接點；無新 generic scripting API | Native 判定 HR，Lua 只編排演出。Scenario 在下一球安全點 reload；不綁全部 native 型別，不做 coroutine state migration。 |
| 9. 迭代工具補齊 | 沿已存在的 clips／mesh／texture 補安全 asset reload；補完整來源顯示、保存與同版本 replay | 每球快照／紀錄、各概念的面板與 validation | Generation lifetime、對應 GPU fence 退役、必要載入流程；有卡頓才加小 worker | T1/T5–T11；不把 early steps 已有的紀錄／調參重寫成通用 editor。不能以此步為由將早期 debugging 全部延後。 |
| 10. 驗收並修最弱一段 | 固定候選版本進行 F1–F7 與 T1–T11，回到失敗階段做小步 A/B | 手感調整與玩法判斷 | 只修實驗揭露的缺口 | 完成判斷以雙重 gate 為準，不能用新增 engine feature 抵銷 F7 失敗。 |

步驟 2／3 的最低動畫匯入可分別先支援已取得的單一 sample；Engine 提供數學／取樣，而 release marker、bat path 與骨架用途仍由 Pawapuro 擁有。步驟 4 的 contact、步驟 7 的 HR 事件從出現當下就記錄；第 9 步只補足可保存與可比較能力。

## 建議第一個真正 coding task

**「一個能看見出手、看球飛到本壘，並一鍵重投相同球的 Windows 3D 切片。」**

它驗證投球尺度、基本時間與再試循環。這只是 M1 的第一個小交付，不能稱為 Batting Feel 已完成，也不能略過下一步的投手動作。

先做 step 0 的只讀環境核對，之後把 task 拆成可分別 review 的三個小交付：

1. 最小 native app 顯示本壘、出手位置和球，能正常結束；只有所需 window／input／D3D12 顯示與 camera。
2. 固定 tick 推進一顆固定初始條件的球，明確顯示準備與 release 提示；顯示目前 tick／球位置，可 pause／single-step。
3. 一個按鍵重投相同球，連續重投 20 次具有相同 release／到達 tick 與 trajectory 紀錄；同一畫面能追查初始條件的 Native owner。若還沒引入 Data，明示值目前來自 native fixture，不偽裝成 hot reload。

當步接受準則：能在目標 Windows 環境啟動與退出；球可見地飛向本壘；暫停不推進 tick、single-step 恰進一 tick；重投不留上一球狀態；能展示可重現的 20 次紀錄。過程不需要 Lua、完整 asset pipeline、球棒碰撞、ECS 或 editor。若 D3D12 啟動工作已大，第一小交付就停在可見靜態場景 review，不順手建立 renderer framework。

2026-09-14：使用者已授權並完成交付 1；Debug／Release 均成功建置、呈現靜態場景、resize／minimize／restore 並正常退出。Debug validation error 為 0；live-object report 未列出未釋放的 child resources。實測方式、限制與建置命令見 [開發環境／交付 1 紀錄](../development/environment.md)。交付 2 的球運動與交付 3 的重投／紀錄尚未開始；未驗收球飛行、固定 tick 或 20 次重投。

2026-09-14 staging calibration：固定 1280×720 windowed，校正 camera／外野深度／投手丘 reference，加入 batting 專屬的啟動 TOML 載入。這是交付 1 的延伸，不是新增 simulation 交付；決策見設計文件，建置與驗證證據見開發環境文件。

2026-09-14 後續 staging 校正：已完成右投 vs 左打構圖、1080p 固定視窗、獨立紅土分區與中央標尺；仍是交付 1 延伸，沒有開始交付 2。當前契約與實測設定分別由設計／開發環境文件維護。

2026-09-14 交付 2：已驗證 Ready → release → fixed-tick flight → Complete、pause／single-step、不同 render chunking 的權威 state 一致及單次 arrival。控制／時間／ownership 契約見設計文件，數值與實測證據見開發環境文件。交付 3 的 rethrow／replay loop、20 次重投紀錄與投手動畫均未開始。


2026-09-14 進壘 framing／同球重投：依本輪授權完成 provisional 好球帶框、水平置中與 Space 在 Complete 後重投。Debug／Release 各 20 次同物件逐 tick 軌跡比較通過，實際 app 各完成初投加 20 次重投，release／arrival 紀錄一致；pause／single-step 未退步。契約見設計文件、設定與畫面量測見開發環境文件；沒有修改球路物理，沒有開始投手動畫、打者或揮棒。這不是完整 replay framework，也不宣告 M1 體驗驗收完成。


2026-09-15：依本輪授權將既有框改為單一 authoritative gameplay strike zone，僅放大寬度。Gameplay-first 決策由設計文件維護，畫面量測／建置／重投回歸結果見開發環境文件；沒有新增好壞球判定或推進投手動畫等後續工作，M1 體驗驗收仍未完成。


2026-09-15 本壘／好球帶一致性：依本輪授權，home plate width 直接採 authoritative strike-zone width，depth 中央成為 evaluation plane；完成 screen-space rectangle 與同一 deterministic simulation 的 predicted-location 環。此為投球驗證迴圈的延伸，未開始好壞球判定、aiming、人物或揮棒。契約與實測分別由設計／開發環境文件維護，M1 尚未完成。


2026-09-15 靜態人物尺度／遮擋：依本輪授權縮小 ball marker、讓 prediction 環尺寸跟隨球，加入右投手與左打者（含靜態 bat）的程序式 Q 版 blockout。Debug／Release、CTest 與實際投球／重投通過，已擷取 Ready／mid-flight／Complete 供 review；證據見開發環境文件。這只是 composition fixture，沒有開始正式 asset／rig／animation pipeline、揮棒或新物理，M1 體驗驗收仍未完成。


2026-09-15 field readability／presence pass：依本輪授權放大靜態投手、提高並加寬 raised mound、小幅抬高 camera target，加入打擊區／界外白線、一／三壘與外野牆 blockout。使用者選擇保留 90° diamond 方向，允許壘包在目前視角外。Debug／Release、CTest、實際投球／重投與三狀態擷取完成，細節見設計／開發環境文件；未開始 animation pipeline 或新 gameplay rule，M1 體驗驗收尚未完成。


2026-09-15 pre-animation staging correction：依使用者回饋比較三個較低丘高後選 0.35 m，下移 authoritative zone 並維持其寬高；將站立投手替換為唯一靜態 release pose，對照後腳／跨步／前傾與原 release point。使用者選擇保留 0.30～1.25 m zone，頭身比例關係留待 review。這是正式 rig／animation 前的幾何驗證，沒有開始動畫、打者揮棒或修改投球物理；實測證據見開發環境文件。


2026-09-15 off-axis projection correction：正式 animation pipeline 前，將 camera 水平方向對齊 +Z，透過從 gameplay focus 推導的 horizontal lens shift 置中，消除打擊區 X 向橫線的斜視感。Debug／Release、CTest 與三狀態實際擷取完成，兩側後方白線 screenshot ΔY=0 px、zone center X=960；projection 契約與量測見設計／開發環境文件。沒有修改人物、球場、gameplay 或投球物理，也沒有開始 rig／animation。


2026-09-15 Character Style v1：在既有設計文件確立大頭、大腳、簡化身腳、球形手與 silhouette-first 原則，校正右投手 release／左打者 ready 靜態 blockout。Debug／Release、CTest、實際重投與三狀態擷取通過，證據見開發環境文件；camera／gameplay／投球物理未改，正式 rig／skeleton／animation pipeline 尚未開始。


2026-09-15 Character Style／紅中 fixture：比較較大扁腳候選後選定平面／高度雙倍率，帽子改成包頭 crown＋brim，手臂改單段連到球形手。正式 TOML reference pitch 依既有固定 tick 契約反推為 5 號位，Debug／Release、CTest 與實際 20 次重投、三狀態擷取通過；設計／驗證由既有文件維護，未開始 rig／skeleton／animation。


2026-09-15 Final Pre-Rig Character／Vertical Composition Pass：完成兩名 static fixture 的局部衣襬修形與既有 camera 兩個 Y 的 staging 候選，保留 baseline／torso-only／最終構圖及球路證據。Debug／Release、CTest 與實際 app／GPU regression 通過；**Michael＋Julia 已接受本輪靜態造型／構圖，Final Pre-Rig Pass 收尾**；接受項目與暫緩問題見設計文件，實測證據見 environment。下一步在新 Codex 對話規劃 Rig／Animation Pipeline，本輪不開始實作。Early-flight 背景對比與動畫遮擋仍待後續人類檢查；不代表動態球路、動畫、正式角色資產或 M1 已驗收。


## Right-handed Pitcher S0 交付（2026-09-15）

Michael 在 Julia review 後正式授權工具準備、authoring sample 與 export contract。已製作一名右投手的可編輯 `.blend`、GLB candidate、由唯一 marker 產生的 TOML、必要腳本與正常／慢速／batting-view 預覽；格式、round-trip 與局部契約檢查已執行，結果見 [environment](../development/environment.md)，操作見 [pitcher README](../../pawapuro/batting/pitcher/README.md)。

**Michael 判定原 S0 motion 未通過，不進入 S1。** 技術匯出成功不能取代人類動作接受。

S0 通過後仍須另行授權 S1：只把單一 GLB 的靜態姿勢接入 app，先驗證尺度／軸向／顏色與 ownership。S2 的 runtime pose／CPU skinning、S3 的共同 tick／release integration 仍未授權；不在本次修改任何 production C++、HLSL、CMake 或 staging Data。

### S0.1：Reference Study＋Pitching Motion Reblocking

已交付同一資產的粗動作候選：保留 S0 before；修正 coil 方向、pelvis／chest 的先後、固定長度手臂的前甩及右腳跟進落地；更新唯一 marker 對應的 metadata、整段匯出／接觸檢查，提供正常／慢速、同視角 before／after、連續影格與一張診斷圖。操作與證據位置由 [pitcher README](../../pawapuro/batting/pitcher/README.md) 維護。

**仍停在 Michael＋Julia review gate，未宣告 S0 motion 通過。** 參考影片實際觀看、Codex 正常速度播放自看因播放器工具未啟動而未完成；遊戲參考圖未出現在可讀附件。Timing 是候選，數值檢查與 contact sheets 不填補這些證據缺口。人類須檢查不用箭頭能否讀出蓄力、前腳支撐 → 軀幹 → 手臂、release 連續性與後腳落地回穩，並決定是否需下一次 motion revision。

本輪沒有授權或實作 S1～S3；通過這個 gate 後仍須另行授權下一個切片。沒有 production C++／HLSL／renderer／staging Data、打者動畫、碰撞或新場景。

### S0.2A：Closed Ready＋Coordinated Leg Lift

已交付封閉式 Ready、原手套幾何藏球與左腳協調蓄力的局部 candidate，附 ed3d8be S0.1 的同視角／60 fps／1× 比較與完整修正版。Frames 72–205 的 evaluated transforms／mesh 保持一致；frame 79 只診斷，未修 B。驗證結果與觀看限制見 [environment](../development/environment.md#s02a-closed-readycoordinated-leg-lift2026-09-15)，檔案入口見 [pitcher README](../../pawapuro/batting/pitcher/README.md)。

**停在 Michael＋Julia review gate，未宣告動作通過。** 正常速度播放自看與參考影片觀看未完成，不能用逐格圖片或數值檢查代替。B／C、S1～S3 均須後續明確授權；沒有 production runtime／renderer／staging 變更。

### S0.2B：Arm Deformation Fix＋Glove Direction

已交付獨立右臂／手套 candidate、f77／79／81 兩角度比較、Ready／coil 回歸、手套姿勢與正常速度預覽。Michael 對左腳抬起／蓄力連動的接受已記錄，逐格保護未改。Michael＋Julia 已接受 A 的 Closed Ready、藏球與左腳／蓄力連動，以及 B 的右臂 deformation 修正與跨步手套朝本壘。正式三檔已由接受的 review/s02b 原樣複製升至 S0.2B。C 的 release 後軀幹續轉／前折、右腳跟進並落在比左腳更靠本壘的位置、完整 follow-through／recovery 仍待處理及 review。S1 未開始，整支 pitch motion 與 M1 尚未通過。

既有匯出／motion 檢查與 B 的區段保護均通過；正常速度播放自看與 runtime／GPU 測試未完成。證據入口見 [pitcher README](../../pawapuro/batting/pitcher/README.md)，實測見 environment。Promotion 後既有驗證再次通過，保留 review/s02b 作為歷史驗收 artifact。完成後停止，等待下一個授權。


### S0.2C：Follow-through Rotation＋Rear-Foot Recovery

已從正式 `55489a9c` 的 S0.2B 完成局部 candidate（`review/s02c/`），在 Michael＋Julia review 接受後原樣升為正式 S0.2C。1–97 的 keys／bones／mesh 保護與 A／B 回歸通過；後段增加軀幹續轉／前折與右腳前移，最終右足心比左足心更靠本壘約 0.374 m。保持 205 格，右腳在 171 接地並保留後續回穩時間。交付正常速度 before／after、兩視角完整影片、接點／落地 contact sheets 及俯視診斷。

Michael＋Julia 已完成 A／B／C human review；正式 pitcher 三檔原樣升至 S0.2C，Right-handed Pitcher S0 的單一 pitch clip authoring motion baseline 通過。這只代表 Blender／GLB authoring baseline；app runtime animation、release integration、dynamic occlusion 與 early-flight readability 尚未驗證，正式遊戲品質與 M1 尚未完成，S1 未開始。 Michael 已實際正常速度觀看並接受 C 的旋轉／前折、投球臂延續、右腳跨前與 recovery 節奏；Codex 正常速度播放自看及 runtime／GPU 測試仍未完成。本次完成 promotion 後停止，等待下一個授權。


## S1：Static Pitcher GLB Runtime Import（2026-09-16）

已依本輪授權接入正式 S0.2C GLB 的 static bind mesh，取代 procedural pitcher，並交付 actual app startup／crop。Debug／Release build、CTest 各 3/3、實際 release／Complete、pause／single-step／20 rethrows 與 Debug GPU 檢查通過；詳細結果與可開啟證據由 [environment](../development/environment.md#s1-static-pitcher-glb-runtime-import2026-09-16) 維護，使用方式見 [pitcher README](../../pawapuro/batting/pitcher/README.md#s1-static-bind-pose-runtime)。

**Michael＋Julia 已接受 S1 human review。** 接受限於基本位置、尺度、左右手／朝向、grounding、vertex colors 與 scene integration，renderer 無須擴張；不代表動畫／skinning、rubber-arm 動態造型、release／ball attachment、dynamic occlusion 或 early-flight readability 已驗證。S0 authoring baseline 已接受，M1 尚未完成。S2 與 S3 的現況見下節。

[Character Motion Rules v0.1](../design/character-motion.md) 已獲 Michael＋Julia design acceptance。S1 bind-pose 的 footprint／elbow 輪廓 debt 由該文件維護，現在可透過 S2 runtime 做 Character Polish review。


## S2：Runtime Pitcher Animation Playback／CPU Skinning（2026-09-16）

**Michael＋Julia 已接受 S2 技術與 human review。** 已交付單一 `pitch_R` 的 240 Hz pose／CPU LBS、concrete PitcherMotion owner 與固定容量 dynamic triangle stream；啟動 Ready 是 authoring frame 1，Complete 停在 frame 205。Space／P／`.` 只控制 animation，球仍是獨立 reference fixture。

Debug／Release build、4/4 CTest、實際 app start／replay／pause／single-step／minimize／restore／Complete 已通過，Debug GPU validation 0 errors。八個 source review samples、release alignment、固定骨長、contact intervals 與舊 simulation tests 通過；完整數值與證據由 [environment](../development/environment.md#s2-runtime-pitcher-animation-playbackcpu-skinning2026-09-16) 保存。

Michael＋Julia 已確認 authoring motion 正確在 runtime 播放，同時確認 footprint／hard-elbow style debt 仍存在。v1A 的 1.25× support footprint 已通過 Michael＋Julia review 並原樣升為正式 asset；foot visual style 未完成，下一步 debts 由 Character Motion Rules 維護，Foot Shape v1B geometry 已通過 Michael＋Julia review 並 promotion，front-foot orientation 已診斷正確，v1C shoe silhouette 已 human rejected、未 promotion；Rubber Arm v1A 暫緩 promotion；Throwing Arm Whip Timing v1A 已 human accepted 並原樣 promotion，主要改善為 post-release timing compression 與速度對比，S3 未開始。v1A promotion 只升版 asset，camera／staging／physics 保持不變，未做球 attachment／release、render interpolation、GPU skinning 或 batter animation。

完成本輪後停止；後續 style polish 與 S3 都須另行授權，M1 未完成。

## S3：Pitch Delivery Accepted（2026-09-16）

已完成唯一 240 Hz `PitchDelivery`、held grip → release-once → 原 Native flight；Space／P／`.` 改為控制整個 delivery。Gameplay arrival 在 tick 479 立即成立，presentation recovery 到 816 才結束；不截斷 accepted follow-through。上節 S2 的獨立 reference ball／未開始 S3 是歷史交付狀態，現行契約見 Batting Feel。

Debug／Release build 與 CTest 各 5/5，含 delivery 邊界、20 replay、30／60／120 chunking、原 asset／motion／physics regression；實際 app 操作及 Debug GPU validation 通過。證據與數值由 environment 維護。正式 asset、camera、renderer、staging 不改，沒有 style polish。

**Michael＋Julia 已接受 S3 human review**：held ball 跟隨 grip、release tick／ownership、既有 ball flight／prediction／arrival、完整 follow-through 與 gameplay／presentation completion 分離均接受；右投手整體演出作為 power-pitcher baseline。Cyan diagnostic、dynamic occlusion、material／style 仍可保留為非 blocker debt，M1 未完成。

[Batter Motion Brief](../design/batter-motion.md) 已獲 Michael＋Julia 接受。Batter Authoring S0 左打 full-swing baseline 已獲 Michael＋Julia 接受。Batter Runtime S1 static import 已獲 Michael＋Julia human acceptance。S2 synchronized swing playback 已獲 Michael＋Julia human acceptance：一個 BattingPreview 240 Hz clock、兩角色 CPU skinning／共用 dynamic stream，保留球 479／投手 816 完成，打者及 preview 到 896。Debug／Release、各 8/8 CTest、實際 app controls 與 Debug GPU validation 通過，證據／誤差／成本由 [batter README](../../pawapuro/batting/batter/README.md#batter-runtime-s2--synchronized-swing-playback-candidate) 維護。這是 authored choreography preview，不定義正式 swing input；collision／contact presentation 未開始，M1 未完成。


Bat Contact S0 bounded continuous closest-approach probe 已獲 Michael＋Julia acceptance。Debug／Release 各 9/9 CTest 與本輪 actual Debug app／GPU validation 通過；數值與限定 geometry 意義由 batter README 維護。本輪不決定 hit／miss、半徑、response 或 input timing，M1 未完成。


Bat Contact S1 first continuous contact event 已獲 Michael＋Julia acceptance。第一 envelope／one-shot event 不改球；Debug／Release 各10/10 CTest、20 replay／30/60/120、actual app／GPU regression 通過。數值與 evidence 見 batter README；本輪不進入 response、sweet spot 或 player input，M1 未完成。


Bat Contact S2 timing-to-contact geometry map 已獲 Michael＋Julia acceptance：offline phase sweep、五例diagnostic與一次local refinement；Debug／Release各11/11 CTest、20 replay／30/60/120 map一致、正式runtime state不變。較超前側Contact到+60 ms仍未結束，未超出授權範圍繼續搜尋。Response／sweet spot／player input未開始，M1未完成。


[Hit Authorization S0](../design/batting-contact.md) 已完成 design／bounded reference analysis candidate，待 Michael＋Julia review。五層責任與未決 gameplay 值由該文件唯一擁有；physical S0／S1／S2 不變，沒有 production authorization、player input 或 response，M1 未完成。

Hit Authorization S1 已實作最小 Normal spatial gate 與獨立 in-game 授權狀態，保留 Compact default 及 geometry truth；具體契約、fixtures 與 review 限制由 [batting-contact](../design/batting-contact.md) 擁有。此交付停在 Michael＋Julia review，M1 未完成，不接續 correction／ability／quality／Ball Response。

Hit Authorization S1 已獲 human acceptance。Timing Interaction S1 已移除舊 development input domain、延伸具體 Normal motion，加入 trajectory-derived Z slab／swing potential 與 temporal intersection query；契約、數值 fixtures、presentation continuation 與實機 evidence 限制見 [batting-contact](../design/batting-contact.md)。本輪停在 Michael＋Julia review；follow-through 長度調整延後，M1 未完成，沒有進入 correction／Contact Quality／Power／Ball Response。

Follow-through Compression S1 將post-potential尾段重排至約1秒完整完成，保留world＋190 ms前綴與Timing Interaction S1，仍一球一次；動作契約、技術檢查與review evidence見 [batter-motion](../design/batter-motion.md)。停在Michael＋Julia的1×human review，尚未授權multi-swing，M1未完成。

Follow-through Compression S1 已由Michael接受為Normal baseline。Multi-Swing Intent S1加入同pitch的獨立SwingAttempts、完整finish後的live硬rearm與passage-exit新intent截止；不改motion／timing／spatial／physical tuning。Ownership、regression與human review evidence見 [batting-contact](../design/batting-contact.md)，hard reset手感待Michael＋Julia review，M1未完成，沒有進入recovery animation／correction／Contact Quality／Power／Ball Response。

Multi-Swing Intent S1已由Michael human accepted。Batter Attributes／Batter Card S1建立單一Michael startup profile與左下角卡片；grade／trajectory契約、future intent、tests與實機evidence見 [Batting Feel](../design/batting-feel.md#batter-attributesbatter-card-s12026-09-19)。本輪不讓attributes影響gameplay，不加入roster／lineup，M1未完成；停止等待Michael＋Julia review。

Batter Attributes／Batter Card S1已由Michael接受。本輪Gameplay Contact S1改由temporal＋spatial判定，raw geometry保留為diagnostic；Ball Response／gravity-only Batted Ball Flight S0開始接上既有profile。公式、ownership、fixtures與evidence由 [batting-contact](../design/batting-contact.md#gameplay-contact-s1ball-responsebatted-ball-flight-s02026-09-19) 擁有。仍無spin／drag／bounce／foul-fair／Contact Correction／Power mode；M1未完成，交付後停止等待Michael＋Julia review。
