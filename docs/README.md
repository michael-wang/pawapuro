# Pawapuro 設計文件

更新：2026-09-14。狀態：已完成 M1 Step 0 與 Step 1 交付 1 的靜態場景驗證；另已加入靜態 staging calibration／啟動 Data；交付 2／3 尚未開始。

第一個 milestone 的成功是：**玩家打完一球後想立刻再打一球。** 引擎能力由這個投打循環需要的工作逐步產生。

## 閱讀順序與文件責任

| 文件 | 唯一責任 |
|---|---|
| [Batting Feel 設計與邊界](design/batting-feel.md) | 六階段體驗、Native／Data／Lua 責任、concept locality、時間與資料契約。 |
| [M1：Batting Feel](milestones/01-batting-feel.md) | 本次 scope、驗收方式、小步 implementation plan、第一個 coding task。 |
| [Jai Design Adoption Review](research/jai-design-adoption-review.md) | 2026-09-13 研究快照，保存來源、取捨與 migration 分析。 |
| [開發環境核對](development/environment.md) | 工具與依賴紀錄、建置／啟動命令、Step 1 交付 1 的實測結果與限制。 |
| [AGENTS.md](../AGENTS.md) | Coding agent 的精簡操作與工程規則；不重複架構文件。 |

最新使用者決策已整合至設計文件；milestone 只定本次交付範圍。研究快照不作目前排程或完整功能清單。三者有衝突時，以最新使用者決策為準，更新設計與 milestone，不回頭把歷史研究改寫成已批准的規格。

目前維持 C++20、Direct3D 12、HLSL、SDL3、Lua 5.5、Dear ImGui、runtime data／script／asset reload 與 Blender → glTF／GLB 方向。這不是第一個 coding task 必須一次引入的清單；已核對的工具與缺項見開發環境文件。

## 維護規則

- 行為與層級決策改設計文件；交付順序與驗收改 milestone。避免另建內容相同的 roadmap、architecture overview 或多份 checklist。
- 具體功能開始實作後，其 native、script、data 與短 README 靠近同一概念；此處保留跨概念決策並連向模組文件，不複製公式、參數清單或 API。
- 新的獨立決策只有在取捨已無法清楚放進這兩份工作文件時，才新增 decision record。
- 文件中的數字、控制方式與測試規模若標示「提案」，代表等待本次 review；沒有測試結果時不得宣稱已通過。
- 目前程式只提供靜態 Windows 3D 參考場景；後續球運動、simulation 與重投須另行授權。
