> 文件定位：2026-09-13 的研究快照，於 2026-09-14 納入專案 docs/research/。原研究內容與來源保留；原文「沒有修改專案」描述的是當時研究回合。本次 milestone 的現行決策見 [Batting Feel 設計](../design/batting-feel.md)，交付與驗收見 [M1](../milestones/01-batting-feel.md)。本檔提供背景，不代表所有列出的能力都必須於 M1 實作。

**Jai Design Adoption Review — Pawapuro 輕量級 3D 引擎**

研究日期：2026-09-13。範圍：技術研究與架構審查；本報告是建議，沒有修改 Pawapuro 專案、建立引擎專案、安裝工具或撰寫 production code。

**決策摘要**

C++20 仍是合理的 bootstrap language。保留 Direct3D 12、HLSL、SDL3、Dear ImGui、Blender → glTF / GLB，以及執行期間修改 data、script、asset 的方向。Lua 5.5 保留為可熱重載的玩法行為工具，但不把所有 gameplay 強制放進 Lua。

Jai 對這個專案最有價值的啟示，是以實際遊戲檢驗工具、讓資料與資源生命週期清楚、降低修改到觀察結果的成本，以及把人能理解整條執行路徑視為品質要求。C++20 可以取得其中大部分收益。Jai 原生的型別資訊、編譯期執行、語法樹操作、受編譯器支援的 macros、build interception，則不值得在 C++20 重造。

我們應維持一個長期能獨立工作的 C++ 引擎，同時讓特定 subsystem 將來能換成 Jai。Migration 是日後根據證據做出的選擇，不是現在的重寫承諾。

**證據範圍與可信度**

本次先查閱 repository 的 [README 與章節目錄][readme]、[Table of Contents PDF][toc]，再深入第 8、11、21–23、25、26A/B/C、29、30A/B、31、36 章，並查閱 compiler、debugging 章節及 [Performance appendix 的 DOCX 原檔][performance]。對關鍵機制另外閱讀 repository 中的完整 example source；沒有 Jai beta compiler，因此沒有自行編譯或執行，不能將教材附上的預期輸出當成此次測試結果。

Repository 取樣固定於 commit 19cb4b7acb0de2798c769f9ad73313a4d15f4056（該 commit 日期 2026-06-11）。報告中的 repository 連結固定於此版本，避免日後內容更新後無法重查。附帶的 PDF 目錄與 README 的章節覆蓋不同；較後面的研究以 README 連結及實際 Markdown 章節為準。

下文區分三種來源：

- **Blow 本人的可核對表述**：以有訪談者與受訪者標示的公開訪談為主要依據。
- **教材整理與轉述**：功能說明、作者的 priorities 排序、歷史效能敘述，以及教材標示為 Blow 說法的引文；它們不是官方 specification。
- **Example source 與本報告判斷**：source 能顯示範例實際使用的方法；是否適合我們的引擎，是本報告的工程判斷。

Blow 在 2021 年的訪談中明確談到：用具有真實複雜度的商業遊戲與 graphics engine 檢驗語言；重做一個系統數次後，理解增加，實作可以更簡單、更容易交接。他也描述自己隔一段時間回看程式仍感到難以理解的問題。這直接支持「Pawapuro 驅動引擎」與「人類能理解實作」的方向。這不代表他認可本報告所選的每一套 library。[本人訪談][interview]

教材將 productivity、performance、safety 排成優先次序；本報告把這當成作者對 Jai 的概括，不把它升格成經核對的官方優先序。README 一方面宣稱 examples 持續測試，另一方面具體版本註記只明確說明程式碼到第 8 章在 beta 0.2.024 上測過。不能因此保證後續每個 API 與旗標適用於目前 compiler。[設計介紹][priorities]、[README][readme]

關於「Order of the Sinking Star 發行後逐步公開 Jai」，本次查閱的教材提供了這個預期，但不能據此確定可取得日期、授權、支援承諾或 API 穩定性。Migration 的啟動條件應是實際可取得並驗證的工具鏈，不是遊戲發行日期的推算。[前言][preface]

**重要概念逐項審查**

以下各項都回答：根本問題、目前相關性、收益來自思想或 compiler、C++20 能否簡單取得收益、最小方式、不模仿的界線，以及 migration 適合度。

**1. 以實際工作驗證設計與 priorities — ADOPT NOW**

根本問題是工具在 demo 中成立，遇到真正遊戲的資料量、修改頻率與除錯需求卻失效。對 Pawapuro 高度相關：我們需要可觀察的投打實驗，而不是先完成一份通用引擎功能清單。這項收益主要來自設計方法，與語言無關。[本人訪談][interview]

C++20 的最小方式，是讓每次新增引擎能力對應一個可重現的遊戲需求，例如修改重力後重投、顯示碰撞法線、檢查動畫與球棒碰撞時間。只有一個使用情境時，具體函式與資料結構通常足夠。不要因 Jai 有強大的抽象工具，就把教材裡展示的每個 pattern 變成引擎基建。

Migration：直接保留工作方法、實驗案例與判斷依據。它們比語法更值得保存。

**2. Modules 與可見性 — APPROXIMATE IN C++20**

根本問題是控制名稱、依賴與修改影響範圍。引擎、Pawapuro native code、Lua integration、renderer 和 asset import 都真實需要邊界。教材區分 #import 的模組範圍與 #load 的檔案載入，以及不同 scope directives；Jai 的具體語意需要 compiler，但模組化本身不需要。[Modules][modules]、[Scope][scope]

C++20 可以用普通 header / source、namespace、少量 build targets 達成。公開宣告只暴露使用者真正需要的資料與操作，實作保留在 source；依賴版本由專案固定。不要為模仿 Jai 而第一天導入 C++20 modules、動態 plugin system，或替每個模組製作 interface、factory。

Migration：模組依賴若已清楚，將來可以一次替換一個 implementation。今天仍可使用普通 C++ 呼叫，不必先讓所有模組通過 C ABI。

**3. Memory allocation、allocator 與 ownership — ADOPT NOW**

根本問題不只是 malloc 的時間，而是誰擁有資料、誰釋放、哪些 copy 或 allocation 發生在熱路徑。Pawapuro 的場景、資產、每球紀錄、GPU 資源、背景載入結果都有不同生命週期。教材區分短期、逐 frame、長期且 owner 明確，以及 owner 不明的配置需求；這個分類是可以直接採用的思想。[Memory][memory]

最小 C++20 方法：先按 lifetime 決定 owner；同時出生、同時死亡的 CPU 資料才使用 arena；獨立生滅或可變長度資料可繼續使用 vector、string、unique_ptr 等適合的具體容器。Arena 只需對齊配置、容量／失敗政策、reset 或 mark，以及 high-water 記錄。也可評估標準 monotonic buffer resource，但必須明確選擇 upstream 行為。

不要為統一形式打造 allocator interface 家族、每個 subsystem 的自製 heap、通用 relocating heap 或自製標準函式庫。只有現在確有多種 allocator 必須經相同 API 使用時，才考慮 allocator protocol。

Migration：owner、lifetime 分組及容量量測可保留；allocator 的具體程式碼通常重寫即可。不必追求與 Jai allocator 結構二進位相容。

**4. Temporary storage — ADOPT NOW**

根本問題是大量短期結果的配置成本與釋放雜務。字串格式化、載入中間資料、debug draw、軌跡取樣很相關。21.1 example 實際呼叫 temporary allocator，並在 reset 後顯示 used bytes 歸零；它不會替程式推導所有引用何時失效。[Temporary-storage example][temp]

C++20 的最小方式是明確的 scratch arena，加上借用範圍。區分函式／工作 scratch、CPU frame scratch、資產 generation storage、GPU upload storage。後三者不能共用一個「frame 結束就清掉」的規則。

跨非同步工作的結果必須由接收方擁有或複製到更長期儲存；GPU upload 要等對應 fence 完成才回收。Arena reset 不會自動執行 C++ destructor：scratch 優先放 trivially destructible 資料，其餘物件先正確結束生命週期。

不要模仿成所有函式偷偷使用 TLS temporary allocator，也不要認為加了 arena 就沒有 dangling pointer。教材對某些 beta 版本的 overflow／allocator 行為有歷史描述；我們要明訂自己的失敗政策。

Migration：非常適合；保留 lifetime contract，將 allocator 機制改成實際版本的 Jai API 即可。

**5. defer 與清理 — APPROXIMATE IN C++20**

根本問題是多個返回或錯誤路徑漏掉清理。GPU、檔案、mutex、Lua VM、音效裝置都需要。Jai 的 defer 是語言功能，清理就近的思想則可以移植。[defer][defer]

C++20 優先用既有 RAII owner、lock_guard 等局部 scope 管理。只有確有重複且無合適 owner 的清理程式時，才需要小型 scope guard。不要為模仿 Jai 而全面禁用 destructor、unique_ptr、RAII，或創造巨大的 DEFER macro DSL。

RAII 也不是 GPU 同步：CPU wrapper 死亡前必須先滿足 GPU 對資源的最後使用條件，必要時延遲 release。

Migration：用 Jai defer 或 subsystem teardown 取代局部機制；保留資源的實際 owner 與完成條件。

**6. Data-oriented design、arrays 與 SOA — ADOPT NOW；自動 SOA 生成 DEFER UNTIL JAI**

根本問題是資料佈局與實際運算不匹配，導致不必要的間接存取、記憶體流量及難以批次處理。這是我們需要考慮的設計思想，但不代表所有資料都應 SOA。

26.10 的 SOA example 利用 type_info 與 #insert 生成每欄一個陣列；使用端仍逐欄存取，並明寫 AoS → SOA 的拷貝。它不是對任意程式透明的自動資料重排證據。[SOA source][soa]

C++20 現在採普通 struct 與連續 vector；一顆球的位置、速度、spin 一起更新時，AoS 很合理。未來大量軌跡候選、粒子或可批次運算資料經 profiling 證實有益時，直接寫對應 SOA。std::span 用於不擁有的範圍，明確約束來源資料的 lifetime。

不要建立能轉換任意 struct 的模板 SOA、proxy reference、泛型 iterator／expression framework；也不要把 data-oriented design 等同於 ECS。CPU struct layout 與 HLSL buffer layout 要分別核對，不能推論 memcpy 一定相容。

Migration：資料與迴圈通常很好逐段改成 Jai。只有多個真實案例形成相同生成模式後，再評估 Jai 的 metaprogramming 是否值得用。

**7. Polymorphic procedures / structs — APPROXIMATE IN C++20**

根本問題是同一演算法或容器只因型別、固定參數不同而重複。Jai 由 compiler 做 specialization，這是語言能力；減少已存在的重複則與語言無關。教材也轉述 Blow 對過度 polymorphism 的編譯成本與可理解性疑慮；此段是教材轉述，不是本次獨立核對的原始發言。[Procedures][poly]、[Structs 與節制建議][polystruct]

C++20 用普通 overload、少量簡單 template、既有容器即可。模板應對應清楚的重複模式；不要用 template 參數承載會在執行期間調整的球速、drag、動畫 timing。

不模仿 #modify、任意型別合成或 traits DSL；不因「未來會換實作」先建立 CRTP、type erasure、policy template 組合。這些不是目前 Pawapuro 所缺的能力。

Migration：具體資料處理函式與小型泛型容器容易逐段改寫；避免讓 public API 要求 caller 參與 C++ template specialization。

**8. Context — APPROXIMATE IN C++20**

根本問題是 allocator、logging 等環境服務需要沿呼叫鏈傳遞與局部改變。Jai 將 context 隱含傳給一般 Jai procedure；它不是一個任意 OS thread 都自動安全取得的全域 singleton。#c_call 邊界不攜帶一般 Jai context，example 必須建立並 push context 才能呼叫相關程序。[Context][context]、[Callback example][callback]

對我們的 scratch allocation、worker logging 有需求；對「每個函式可隨時存取任何 engine service」沒有需求。C++20 的最小方法是只向需要的函式傳入 Scratch、Logger 或少數具體參數。Worker 的工作資料明示 owner、輸出位置與取消狀態。

不要建立包住 renderer、world、audio、AI、filesystem 的萬能 EngineContext，或用 TLS service locator 模仿 compiler 隱含傳參。Context 的便利也不能消除指標 aliasing 和 thread-safety 問題。

Migration：日後可用 Jai context 承載適合的環境服務；跨 C callback 時另行建立正確 context。業務資料仍應維持明確傳遞。

**9. Reflection 與 runtime inspection — APPROXIMATE IN C++20**

根本問題是同一欄位在編輯、顯示、驗證、存檔中重複描述。對我們高度相關。Jai type information 降低逐欄生成程式的成本，但不能自動知道一個欄位的單位、合理範圍、套用時機或保存政策。26.29 example 生成序列化呼叫並得到簡單文字；它沒有提供 schema evolution、穩定 ID 或 migration 規則。[Metaprogramming][meta]、[Serialization source][serialization]

最小 C++20 做法是明寫 PitchSettings、ContactSettings、CameraSettings 等 game-owned 資料、解析／驗證函式與 ImGui 面板。若 UI 和持久化已多次重複相同 scalar 描述，再加僅支援實際型別的小表：名稱、單位、範圍、存取方式、套用時機。

這個小表是「調參資料的描述」，不是全 C++ type system 的 reflection。不要掃所有 header、解析 AST、自製 property graph、通用 object database 或任意物件 serializer。直接手寫少量重複比把每個欄位藏在多層生成機制後更好。

Migration：保留檔案 schema、欄位語意與 validation；把重複的存取程式換成 Jai 生成內容是合理的候選。就算有 reflection，仍需要人工設計持久化格式。

**10. #run、#insert、Code 與編譯期生成 — DEFER UNTIL JAI**

根本問題是編譯時已知的資訊仍要手寫、外部生成或執行期間計算。Jai 把編譯期執行、型別資訊與 compiler workspace 放在一起，主要收益高度依賴語言及 compiler。[Metaprogramming][meta]、[Applications][applications]

我們目前只需少量真正不可變的表格、shader／asset 外部工具呼叫；C++20 的 constexpr、static_assert 及既有工具足夠。Runtime tuning data 必須保持 runtime data。#run 不等於 script hot reload，#insert 不等於任意 native state 能在執行時換版。

不要建立 C++ AST translator、自製 type database、general-purpose code generator 或編譯期 interpreter 來取得 Jai 的語法體驗。若日後真的有大量重複 schema，狹窄、獨立、輸出可閱讀的 generator 可以重新評估；不必教條式等 Jai 才能生成任何檔案。

Migration：Jai 公開且版本經驗證後，這是最值得評估的原生能力之一；使用範圍仍由實際重複決定。

**11. Hygienic macros 與控制流程抽象 — DEFER UNTIL JAI**

根本問題是普通函式不方便表達某些呼叫端控制流程。26.7B example 展示受控制的 caller-scope 存取與 return 行為；這些依賴 Jai compiler，不能用 C preprocessor 等價重現。[Macros][macro]、[Macro source][macroex]

我們需要 profiling scope、有限迴圈和批次操作；C++20 的普通函式、lambda、for 與 RAII 足以處理目前案例。已採用 profiling library 的小型 instrumentation macro 可以使用，不必因此禁用所有 macro。

不要自製語法 DSL、變更 caller 控制流程的巨集、反射註冊巨集家族或多層 variadic macro。教材也明確提醒宏可能變成難以維護的混亂；「最後手段」是該作者在此章的工程建議。

Migration：只有一般函式仍無法清楚表達的重複模式，再用 Jai 原生 macro。語言公開不會使每種 macro 都值得採用。

**12. Integrated build system — APPROXIMATE IN C++20**

根本問題是 build 規則分散、修改回饋慢、開發者難以知道實際編譯了什麼。Jai 可在同語言 metaprogram 中設定 workspace、檔案、build options；這降低了語言與 build 工具間的落差，但沒有消除外部 SDK、linker 或 shader compiler 的依賴。[Integrated build][build]

C++20 最小做法是簡單 CMake + Ninja 設定、固定依賴版本、單一可重現入口、Debug 與帶 symbols 的 optimized 設定。避免 gameplay 數值修改觸發 native build；shader 和資產處理也要避免無關的全量重做。

量測 clean build、修改單一 gameplay source、修改公共 header、link、啟動及回到可投球狀態所需時間。不要把超快 clean compilation 當成必須全面禁用 incremental build 的理由，也不要重寫 build system 來模仿 Jai。

Migration：若 Jai 的 build integration 對混合專案有實際收益，可逐步改成 Jai orchestrate 部分 C++ 工具；不必一次替換整套 build pipeline。

**13. Build manipulation 與 Plugins — DEFER UNTIL JAI**

根本問題是將規則檢查、registration、程式資訊與 instrumentation 整合到 compiler 所掌握的語意之中。Jai 的 compiler messages、typechecked events、notes 和 plugins 是 compiler extension 工作流。[Build manipulation][buildmanip]

第 36 章的 Plugins 指 metaprogram 使用的 Check、Polymorph_Report、IProf 等；不是一套遊戲 runtime DLL plugin 或資產 hot reload 的架構。這個名詞差別會直接影響設計判斷。[Plugins][plugins]

C++20 現在用 compiler warnings、既有 profiler、明寫的註冊清單與 build 工具。不要自製 Clang plugin、全專案 AST 掃描器或編译器 hooks。少量重複註冊尚未構成維護一套 compiler toolchain 的理由。

Migration：日後有 Jai 才評估其原生 hooks。Observability 的需求現在就做，底層用現成工具，不必等 compiler instrumentation。

**14. C interoperability 與 C++ 保留 — ADOPT NOW，限有實際價值的邊界**

根本問題是新語言仍要使用 OS、graphics、audio、asset、AI 等既有程式庫。這項需求對我們非常真實。Jai 支援外部 C 呼叫，repository 還有 BuildCpp + Bindings_Generator 的 C++ example，包含 constructor、destructor 與 virtual-call 使用。[C interop][c]、[Bindings build source][cppbind]、[使用端 source][cppmain]

這支持「可保留一部分 C++ binary」，但不是「Jai 能無條件理解任意 C++」。範例生成內容涉及平台名稱修飾與 ABI；複雜 template、inline、compiler／library 版本仍需實測。

今天使用成熟 C library 即可直接受益。真正出現跨語言或獨立 binary 邊界時，使用少量具體 C ABI 函式：opaque handle、固定寬度數值、pointer + count、結果碼、callback + user data，明定資料借用與由誰釋放。避免跨界傳 STL container、exception、RTTI、vtable 物件。

不要為 migration 把每個內部函式包成 C，或把所有 subsystem 拆成 DLL。普通 C++ module 今天已足夠；第一個真實 Jai caller 出現時才加必要 shim。檔案型資產工具甚至用 CLI 與資料檔即可，不一定需要 ABI。

Migration：這是最實用的漸進路線。C++ 與 Jai 可以長期並存；手動少量 binding 也可能比維護全自動 binding system 更便宜。

**15. Threads、工作 ownership 與非同步 — APPROXIMATE IN C++20**

根本問題是載入、解碼或 inference 不能阻塞主要互動流程；同時 thread 不能任意讀寫活躍的 world。教材展示 threads、thread groups、mutex、每 thread context 與 temporary storage，但不是「語言自動解決 concurrency」。[Threads][threads]

C++20 的最小方式是先保留清楚的主要更新順序，當載入或其他工作需要背景執行時，用少量 std::jthread、mutex／condition_variable、bounded queue 與明確的工作結果 ownership。並非一開始就需要 general job graph、fiber scheduler、work stealing 或 lock-free queue。

教材的 channels example 明說是 single-threaded 簡化版本；不能當 production channel 抄用。章節「鎖順序避免 deadlock」的全面性說法也過強，而且另有 beta 檢查暫停的註記。鎖排序只處理符合條件的鎖循環，不能保證 callback、join 或 GPU fence 等待鏈都不會死鎖。

Migration：傳遞資料與結果的模型可保留，worker 實作可單獨改寫。Jai context 或 SOA 不會替 rendering／inference 提供硬體隔離與 deadline 保證。

**16. Performance 與可觀察性 — ADOPT NOW；教條式配置 DO NOT ADOPT**

根本問題是工程判斷脫離可觀察成本。對我們重要的是 input latency、穩定 frame time、工作完成時間、allocation spike 與理解成本。這是語言無關的方法。

Performance appendix 包含歷史 chess benchmark、避免 GC／RAII 的論述、關閉檢查及 compiler option 清單。它沒有構成跨硬體、工作負載與 toolchain 的比較實驗，不能證明 Jai 對 Pawapuro 有固定百分比效能優勢；列出的選項也不是可直接複製的通用 Release preset。[Performance appendix][performance]

C++20 最小方式：Tracy／PIX 或同級工具、帶 symbols 的 optimized build、資源名稱、CPU／GPU timing、記憶體 high-water、每球資料版本與 replay trace。追蹤 p50／p95／p99、尖峰及可重現案例；依目標機器訂預算。不要先關掉所有檢查、strip symbols、全面 uninitialized storage、強制每個 struct 64-byte alignment 或開 fast-math。

Migration：維持相同場景、trace、量測條件與正確性標準，才能知道改用 Jai 改善了什麼。固定 timestep 不代表跨 compiler／CPU 浮點 bit-exact；必須另訂重現層級與容差。

**四類結論**

**ADOPT NOW**

實際遊戲驅動工具；可追蹤的資料流；明確 owner／lifetime；合適的 arena／scratch；按 workload 決定資料佈局；具體模組邊界；在真實外部邊界保持乾淨資料契約；把編譯、reload、frame time 與記憶體行為變成可觀察成本。

**APPROXIMATE IN C++20**

普通 header／source 模組；局部 RAII；span 與具體容器；少量 template；窄而明確的 context 參數；手寫 inspection／validation；有實際重複才加入小型欄位描述表；CMake + Ninja；少量背景 worker 與明確 queue。

**DEFER UNTIL JAI**

通用 reflection、任意 AST／Code 操作、hygienic macro、任意 struct 自動 SOA、廣泛的型別合成、compiler message interception、compiler plugins，以及和 compiler 深度整合的生成工具。Deferred 是「不要在 C++ 重造它」，不是「目前所有相關需求都不能解決」。

**DO NOT ADOPT**

因 Jai 沒有 RAII 就禁用 C++ RAII；以 TLS singleton 偽裝 context；把 DOD 等同 SOA 或 ECS；一律手動管理／一律 arena；全引擎 C ABI；無限泛型化；把 compile-time execution 當 hot reload；為了快就全面移除診斷；照抄 beta flags、benchmark 結論或教學同步範例。

**重新審查現有 stack**

| 項目 | 決定 | Jai 研究後的具體調整 |
|---|---|---|
| C++20 | 保留 | 以具體 struct、函式、局部 RAII 為主；控制 template、include 與 ownership 擴散；不用「像 Jai」作為成功標準。 |
| Direct3D 12 | 保留目前方向 | 直接、可追蹤的資源與同步管理；沒有實際第二 backend 前，不寫一套通用 RHI。這是 Windows／GPU 需求的選擇，不是 Jai 對 D3D12 的背書。 |
| HLSL | 保留 | Shader 資產與 host language 分開；明確管理 buffer layout 和 shader interface。 |
| SDL3 | 保留 | window／input 等成熟 C API 可沿用；不為 migration 重寫平台層。 |
| Lua 5.5 | 保留，縮小邊界 | 用於需要執行期間換行為的玩法／測試編排；不強制每個 native gameplay 物件進 VM。 |
| Dear ImGui | 保留 | 先寫真實調參與觀測面板；不以它為起點發展完整 property／reflection framework。 |
| Data-driven development | 強化 | 明定 units、validation、default、資料版本與修改套用時機。 |
| Data／script／asset hot reload | 保留，明定交易與 lifetime | 候選版本驗證成功後才發布；舊資源待使用完成再退役；失敗留在可運作版本。 |
| Blender → glTF / GLB | 保留 | 作為 interchange；支援需求明確的 mesh／skin／animation subset，gameplay 設定分開。 |

SDL 有適合其他語言呼叫的 C entry-point 路徑；Lua 5.5 也以 C API 嵌入。這兩者有利於混合語言，但 Lua 自己的 GC、錯誤傳播與 reload state 仍須管理。[SDL 文件][sdl]、[Lua 5.5 manual][lua]

Jai 不足以成為現在重新更換 D3D12／HLSL 的理由，也不能取代 graphics、音訊、動畫、glTF parsing 或 inference runtime。為了自己建立引擎而重寫所有成熟 library，不符合「只處理當前實際問題」的原則。

**Native、game、script 的邊界**

Engine native 提供平台、rendering、通用 geometry query、resource／asset lifetime、animation evaluation、debug draw 與量測。Pawapuro native 擁有棒球相關物理模型、投打狀態及具體玩法資料；Lua 擁有需要頻繁換版的行為編排。Game code 可以是 C++，不必因為「引擎不能知道棒球」就全部放到 Lua。

球速、drag、spin、碰撞係數、球棒路徑與 launch／exit 指標的語意留在 game layer。只有一項操作確有一般性，例如 shape sweep 或通用接觸資料，才升為 engine 能力。不先建萬用物理／ECS framework。

Lua function 與持久 state 分開，native 資源經有 generation 的 handle 使用；用高層 command／event 呼叫，避免逐欄、逐物件跨 VM。碰撞與固定步長核心保持 native CPU、可重放且不依賴 inference。未來 Jai 若讓 native 修改回饋足夠快，可以評估減少 Lua 的責任；不要預先承諾移除 Lua。

**Hot reload 的最小可靠契約**

這部分是依 Pawapuro 需求提出的架構建議，不能宣稱 Jai compiler 已替我們解決。

- Data：解析候選 → 驗證完整設定 → 在指定 update 邊界發布。Camera 可以下一 frame 生效；重力、drag、COR 等每球參數預設下一球套用，或由明確命令重置當前投球。這能同時達成即時調整與可重現實驗。
- Script：先 compile／load 新 module 並驗證 API，於安全更新點換函式；持久 state 顯式傳入。只支援目前需要的 state migration；舊 coroutine 可結束或在重投時重建，不承諾保留任意 stack／closure。
- Asset：背景 import 成候選 generation，驗證 mesh／skeleton／animation 相容性並安排 upload；確認可用後換 handle 指向。舊 CPU 使用者結束且 GPU fence 完成後才回收，錯誤時保留舊版本。
- Shader：新版本編譯與 pipeline 建立成功後才換；resource binding 或 vertex layout 不相容時拒絕或重建依賴，不直接覆蓋活躍物件。
- Native code：保留正常編譯＋快速重啟＋還原實驗狀態。沒有證據顯示 native reload 是主要瓶頸前，不做 DLL 任意 state patching、vtable 重接或全 heap pointer 修復。

重投應保存 initial state、輸入、random seed、固定步長、物理設定、script／asset／程式版本。若未來 AI 影響高階策略，記錄其被採納的結果與 simulation tick，replay 不重新等待 inference。暫停、single-step 與軌跡／碰撞可視化應讀同一份實際模擬狀態。

**保留 migration 自由的實際方式**

**Language-independent data**

保留 glTF／GLB、HLSL source、帶 schema version 的 JSON／TOML 類調參檔、明確定義的 replay／測試輸入。若出現 runtime cache，包含來源 hash、importer 版本與明確欄位定義；cache 可失效重建，不是永遠相容的保存格式。

不要保存 std::vector／std::string 的內存、pointer、RTTI 名稱、隱含 enum ordinal 或整塊 C++ struct dump。Native scalar、ABI、shader layout 和磁碟格式是不同契約；必要時明訂寬度、endianness、對齊與 matrix convention。

Blender exporter 支援特定的 mesh、skinning、transform／shape-key animation 路徑，並不保留任意 Blender 行為。固定實際 Blender／exporter 版本，必要的 constraints 效果 bake 成支援的結果；球的物理與玩法設定不交給 Blender exporter 定義。[Blender glTF 文件；查閱頁為開發版文件][gltf]

**值得維持清楚的 subsystem 邊界**

| 邊界 | 現在的實際價值 | 將來可採的跨語言方式 |
|---|---|---|
| Asset importer／cooker → runtime data | 將 authoring 資料與 runtime 資料分離，支援驗證與快取 | 版本化檔案；必要時 CLI，不必 DLL。 |
| Game simulation → renderer | Renderer 擁有 GPU 資源，遊戲提供繪製資料 | 有真實混合語言需求時，包少量批次 C ABI；不要逐 draw 重造全部 D3D12。 |
| Lua VM → native game／engine | VM 不擁有任意 native pointer，行為和 state 分離 | 既有 Lua C API 與具體 bindings。 |
| Background loader → main loop | 隔離工作 lifetime 和發布時機 | 明確 request／result 資料；跨 thread 本身不要求 C ABI。 |
| 未來 inference → game | 較低頻、非同步、允許過期與取消 | C API worker 或 process message；有實際模型後才定具體契約。 |

一般 engine 模組間現在仍採直接 C++ 呼叫。語言獨立的資料契約與可觀察行為，比今天加入一個空的 IRenderer／IPhysics／IInferenceBackend 更能保留未來選擇。

**哪些 library／code 能保留**

- SDL3、Lua、cgltf、miniaudio：有 C API／C 實作的路徑，可用 Jai FFI 與相應 bindings 呼叫原 binary；版本、callback 和 allocator 契約仍需驗證。[SDL][sdl]、[Lua][lua]、[cgltf][cgltf]、[miniaudio][miniaudio]
- ONNX Runtime：有 C API；未來可保留 native inference runtime。其 C++ wrapper 的存在不要求引擎永久使用 C++。[ORT API][ort]
- D3D12 renderer：可保留為 C++ subsystem；日後也可評估 Jai 的 COM bindings。保留它不會妨礙先改遊戲模擬。
- Dear ImGui，以及若日後採用的 Jolt／ozz 等 C++ library：優先保留已工作的 C++ integration；有實際 caller 時再考慮適配 shim 或經驗證的 bindings。Repository 的 C++ example 是可行性線索，不是所有 library 的認證。
- GLM 等 header-only C++ 型別／template：不能當成可直接 import 的 C binary API。跨語言處只傳明確數值資料；不必為此今天重寫整個 math library。
- 純資料更新迴圈、Pawapuro native rules、軌跡實驗工具、特定 importer：相對適合先逐段改用 Jai。Resource-heavy renderer、外部 library glue、已驗證 native physics 可留在 C++，沒有必要為「全 Jai」而重寫。

**Migration 啟動條件與順序**

先確認 compiler 可取得、授權可用於專案與 CI、目標 Windows／debugger／FFI 工作，以及相關版本可固定。再選一個有明確輸入輸出的實際 subsystem，驗證：

1. 相同測試資料的結果與既定容差。
2. Debugger 能否追蹤 Jai、C++、callback 與生成碼。
3. Incremental 工作流程的整體回饋時間，而非只比較 compiler 宣傳數字。
4. Allocation／GPU 資源是否保持正確 lifetime，混合邊界是否引入額外 copy。
5. Human review 與 AI 修改時，需理解的上下文是否真的減少。

順序可以是獨立工具／importer → 一個 simulation 模組 → 更大範圍 gameplay；renderer 與 host 不必先搬。若收益不明、debugging 退步、binding 維護昂貴或版本不穩定，維持 C++20。C++20 必須是完整可持續的路線，不能設計成等 Jai 才能補完的半成品。

**GPU compute 與 local AI：Jai 沒有替我們解決的部分**

保留顯式 GPU 資源 lifetime、命名、timing 與 budget 記錄，足以為未來 compute 工作留下位置。現在不需要實作通用 inference abstraction。

真正加入 local AI 時，採低頻 asynchronous request／result、bounded queue、過期結果處理與上限，避免 simulation 等待。Worker thread 可以避免主流程被 CPU 呼叫阻塞；獨立 process 能改善故障隔離與部署，但兩者都不等於把同一 GPU 的 VRAM、bandwidth 或執行時間隔離。

D3D12 priority 是排程能力，不是一般遊戲可以倚賴的硬 realtime 分區；GLOBAL_REALTIME 有權限與 driver／硬體條件。VRAM budget 會變動，也不是能用 queue priority 保留的固定配額。[Queue priority][queuepriority]、[Residency budget][residency]

工程上先量測最壞可接受場景，再控制 model size、同時存在的 session、warm-up、batch／工作粒度與提交頻率；必要時暫緩 AI、降低 workload 或移往 CPU／NPU。不能承諾任何 backend 都可立即中斷已提交的 GPU kernel。CPU／GPU／NPU 切換也受模型 operator 與 execution provider 支援限制。

ORT 的 C API 與 provider 機制可減少自製整合工作，但不同 provider 有不同限制。例如目前 DirectML EP 文件不允許同一 session 多執行緒同時 Run。選 D3D12 不代表 rendering 與 inference 自動共享同一套 scheduler。[ORT DirectML][ortdml]

以上結論是 graphics／inference 架構判斷，不應歸為 Blow 或 Jai 的已解決問題。

**AI-assisted development 的影響**

適合 AI 的方向也應適合人類：少量相關檔案、可搜尋的具體名稱、直接呼叫、清楚 owner、固定的重現資料、短而可核對的 build／reload 步驟。大型 generic framework 往往使小修改必須載入更多基礎設施與生成規則。

C++20 的工具與程式庫可立即使用，但 template error、長 include 鏈、C++／Lua 重複型別資料會增加理解成本。Jai 的 compiler 能力可能減少這些重複，卻可能把複雜度移到 metaprogram；必須檢查生成結果是否能搜尋、單步與定位來源。

沒有此次研究支持的數據能量化兩種語言的 token 成本差異，也不能預測未來 AI 對公開 Jai 的熟悉程度。應以完成同一修改所需檔案、錯誤修正輪數、build 時间與人類 review 成本評估，不以生成碼行數最少為目標。

**Jai Design Adoption Matrix**

Complexity 指本專案建議方案的相對複雜度；「高，避免」表示在 C++ 重造該能力的代價。Decision 針對這一列的具體做法。

| Jai concept | Problem solved | Relevance to our engine | C++20 approach | Complexity | Future Jai migration | Decision |
|---|---|---|---|---|---|---|
| Reality-driven design | 工具與實際工作脫節 | 極高；Pawapuro 驗證 | 每項能力連到具體實驗 | 低 | 直接保留原則 | ADOPT NOW |
| Modules / scopes | 依賴與名稱蔓延 | 高 | header／source、namespace | 低 | 逐模組替換 | APPROXIMATE IN C++20 |
| Lifetime-first allocation | owner 與回收不清 | 極高 | 明確 owner、分 lifetime | 低 | 保留契約 | ADOPT NOW |
| Arena / temporary storage | 短期配置與釋放負擔 | 高 | 具體 scratch、容量與 reset | 低至中 | 改用 Jai allocator 機制 | ADOPT NOW |
| defer | 錯誤路徑漏清理 | 高 | 局部 RAII／既有 owner | 低 | 可換為 defer | APPROXIMATE IN C++20 |
| Array views | 借用與拷貝混淆 | 高 | span／string_view＋lifetime | 低 | 改為 Jai views | APPROXIMATE IN C++20 |
| DOD / measured SOA | 記憶體流量與運算不匹配 | 高；SOA 視 workload | 連續 AoS，必要時明寫 SOA | 低至中 | 資料與迴圈容易移植 | ADOPT NOW |
| Generic SOA generation | 重複生成各欄陣列 | 現在低 | 不寫任意 struct 轉換器 | 高，避免 | 真有重複再用 #insert | DEFER UNTIL JAI |
| Polymorphic procs / structs | 有實質重複的型別變體 | 中 | 簡單 template／overload | 低 | 選擇性改寫 | APPROXIMATE IN C++20 |
| Context | 環境服務傳遞 | 中 | 窄的顯式參數 | 低 | 可用 Jai context | APPROXIMATE IN C++20 |
| Type info for inspection | UI／欄位描述重複 | 極高 | 具體面板；必要時小表 | 低至中 | 保留 schema、替換生成部分 | APPROXIMATE IN C++20 |
| Full reflection / AST | 通用程式語意操作 | 現在低 | 不自製 reflection compiler | 高，避免 | 驗證後用原生能力 | DEFER UNTIL JAI |
| #run / #insert | 編譯期計算與生成整合 | 局部 | 小型 constexpr、現成工具 | 高，避免通用重造 | 值得優先評估 | DEFER UNTIL JAI |
| Hygienic macros | 呼叫端控制流程抽象 | 現在低 | 函式／lambda／RAII | 高，避免語法模仿 | 僅在必要時用原生 macro | DEFER UNTIL JAI |
| C interoperability | 重用原生程式庫 | 極高 | 原生 C API；實際邊界才 shim | 低至中 | 保留 binary 與資料契約 | ADOPT NOW |
| Integrated build | build 分散與回饋慢 | 高 | 簡單 CMake＋Ninja、量測循環 | 低 | 可逐步換 orchestrator | APPROXIMATE IN C++20 |
| Build hooks / Plugins | compiler 語意檢查／生成 | 現在低 | 現成 warnings／profiler | 高，避免自製 hooks | 等原生工具可用 | DEFER UNTIL JAI |
| Threads / work ownership | 阻塞與跨工作生命週期 | 高 | 小量 worker＋bounded queue | 中 | 保留 request／result 契約 | APPROXIMATE IN C++20 |
| Profiling / observability | 無法定位成本與錯誤 | 極高 | CPU／GPU timing、replay、symbols | 低至中 | 跨語言保持同一標準 | ADOPT NOW |
| 套用 no-RAII、全關檢查等教條 | 沒有已證實的共同問題 | 負面 | 依 C++ 語意及量測選擇 | 不應投入 | 不當 migration 目標 | DO NOT ADOPT |

**建議長期放進 AGENTS.md 的 10 條原則**

以下是提議文字，未寫入專案。

1. 引擎能力必須由 Pawapuro 的實際需求驅動；棒球語意留在 game／script，已證實通用的操作才進 engine。
2. 新增 abstraction 前，說明它目前消除了哪個實際重複或錯誤來源；沒有證據時保留具體資料與直接呼叫。
3. 先說明資料 owner、借用期限與釋放條件，再選容器或 allocator；CPU frame、background task 和 GPU fence 有不同 lifetime。
4. 優先讓人能追蹤完整資料流與控制流程；避免隱藏配置、全域 service locator、無必要的 virtual／template／macro 層。
5. Data-oriented design 由資料存取模式決定；不預設 ECS、SOA、job system 或任何 design pattern。
6. 縮短「修改到可觀察結果」的整體時間；可調數值維持 runtime data，量測 native build／reload／重現成本。
7. Hot reload 必須先驗證候選，再於明確邊界發布；失敗可保留舊版，退役資源須等所有使用者完成。
8. 保持可重放的 simulation 核心；輸入、參數與版本可追查，非確定性 inference 不進每 tick 必經路徑。
9. 使用 C++20 的簡單既有能力，不重造 Jai compiler；持久資料與真實跨語言邊界保持明確，沒有需求不新增 migration layer。
10. 效能與架構主張要有可重現證據；保留必要診斷，檢查生成碼，註解解釋取捨，不能以 AI 能產生大量程式碼作為增加複雜度的理由。

**Bonus：如果 Blow 明天加入，他可能會挑戰什麼？**

以下是根據其公開設計取向做的推測，不是本人評論或引言，也不表示他會反對所有自製工具。

- 用來管理十幾個調參欄位，卻先寫的 C++ reflection、AST parser 或 bindings generator。
- 只使用 D3D12，卻先寫支援想像中各種 graphics backend 的 RHI、factory 與 plugin protocol。
- 為修改球速而開發任意 native heap migration、DLL state patching；數值原本就能放在 data。
- 每個玩法物件同時存在 C++、Lua、editor、serializer 四套映射，然後再寫工具維持四套同步。
- 尚未量出瓶頸，卻先做 ECS、fiber job system、全域 event bus、lock-free queue 或強制 SOA。
- 沒有具體模型、延遲目標與 VRAM 數據，就先做可替換所有 CPU／GPU／NPU 的 inference framework。
- 名義上為 Jai migration，實際卻讓每次內部呼叫多一層 ABI、virtual interface 或反序列化。
- 為了「自己做引擎」而重寫 window、audio decoder、glTF parser 等已滿足需求的程式庫。

他的反問未必會是「為什麼不用某個 library」，也可能是：「你們現在到底遇到了什麼問題？這段基礎設施讓那個問題更容易理解了嗎？」

**供 review 的最終方向**

維持 C++20／D3D12／HLSL 的具體實作路線，保留 SDL3、ImGui、glTF 與有限 Lua。從第一天採用明確 lifetime、可觀察資料流、可重放實驗與短調參循環。把 Jai 視為日後值得實測的新 implementation 選項；其 compiler 級能力不在 C++ 重造，也不成為目前引擎才能完成的前置條件。


[readme]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/README.md
[toc]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/Table%20of%20contents.pdf
[preface]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/00A_Preface.md
[priorities]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/01B_What_is_Jai_-_more_in_depth.md
[modules]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/08A_Modules_-_Structuring_the_code_of_a_project.md
[scope]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/08B_The_scope_directives.md
[defer]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/11A_Allocating_and_freeing_memory.md
[memory]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/21A_Memory_Allocators_and_Temporary_Storage.md
[temp]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/examples/21/21.1_temp_storage.jai
[poly]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/22A_Polymorphic_procedures.md
[polystruct]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/23A_Polymorphic_arrays_and_structs.md
[context]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/25A_Context.md
[meta]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/26A_Metaprogramming.md
[macro]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/26B_Macros.md
[macroex]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/examples/26/26.7B_macros_basics.jai
[applications]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/26C_Applications_of_Metaprogramming.md
[soa]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/examples/26/26.10_soa.jai
[serialization]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/examples/26/26.29_code_struct_member.jai
[c]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/29A_Interacting_with_C.md
[callback]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/examples/29/29.3_c_call.jai
[build]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/30A_Integrated_build_system.md
[buildmanip]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/30B_Manipulating_the_build_process.md
[cppbind]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/examples/30/cpp_library/first.jai
[cppmain]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/examples/30/cpp_library/main.jai
[threads]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/31A_Working_with_threads.md
[plugins]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/book/36A_Plugins.md
[performance]: https://github.com/Ivo-Balbaert/The_Way_to_Jai/blob/19cb4b7acb0de2798c769f9ad73313a4d15f4056/Appendix%20D%20-%20Performance.docx
[interview]: https://www.notion.com/blog/jonathan-blow
[sdl]: https://wiki.libsdl.org/SDL3/NonstandardStartup
[lua]: https://www.lua.org/manual/5.5/
[cgltf]: https://github.com/jkuhlmann/cgltf
[miniaudio]: https://miniaud.io/
[ort]: https://onnxruntime.ai/docs/api/c/c_cpp_api.html
[ortdml]: https://onnxruntime.ai/docs/execution-providers/DirectML-ExecutionProvider.html
[residency]: https://learn.microsoft.com/en-us/windows/win32/direct3d12/residency
[queuepriority]: https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_command_queue_priority
[gltf]: https://docs.blender.org/manual/en/dev/addons/scene_gltf2.html

