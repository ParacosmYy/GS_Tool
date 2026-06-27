# 02 - 开发工作流与三轴交付标准（PRD/Specs 执行线）

> 适用范围：除纯文档外的所有工程/文档线任务。

---

## 一、任务主线与触发

### 1.1 主线判定（按先后顺序）

1. 构建线：构建/启动/脚本/依赖修复
2. Bugfix 线：明确缺陷修复
3. PRD 线：新增能力、行为变化、新类新增、跨模块改造
4. 技术债线：重构、解耦、清理、性能优化
5. 仅文档线：约束/README/PRD/追踪文件

> 优先级高于直觉：若同时命中多线，以“能否让用户成功启动并闭环”为第一优先判断。

### 1.2 未满足主线不执行

- 未有 PRD/Specs 即做功能线开发：禁止。
- 新增跨模块类、接口、协议未做架构审查：禁止。
- 串口工站新增未按 `docs/serial_station_architecture.md` 约束：禁止。

---

## 二、PRD / Specs / LOOP 约束

### 2.1 何时必须写 PRD

以下必须有 PRD：功能、新类、行为变更、跨模块影响、性能目标变化、并发/部署策略变更。

### 2.2 轻量执行流程（无论规模）

1. 明确目标：范围、边界、非目标、验收标准、三轴目标（`E/U/D`）
2. 列出风险：失败条件、回退路径、依赖前置
3. 落地计划：改文件列表、验证命令、串行/并行分工
4. 执行：最小步提交变更
5. 验收：测试+状态更新+文档映射
6. 回归与总结：更新约束映射和下一步
7. 计分：按门禁计算本轮是否可加分

### 2.3 LOOP 机制（失败处置）

- **Debug**：复现→回归点→补齐证据
- **Doctor**：只读静态体检（依赖、接口、风险点）
- **Simplify**：拆分/抽象减少复杂度（禁止扩大改动面）

- 测试命令：`uv run test-embeddebug-py`
- 启动命令：`uv run start-embeddebug --smoke` 与 `cmd /c EmbedDebug.bat --smoke`
- 通过门槛：Python/PyQt smoke 退出码为 0，且无启动阻断报错
- 未满足则本轮自动记 `+0`，不得进入加分闭环

---

## 三、可量化验收版（AI 可执行）

### 3.1 量化验收矩阵

| 维度 | 入门门槛 | 关闭条件 |
|---|---|---|
| E（工程） | 0~100 | Python 测试、启动或打包链路有证据 |
| U（用户） | 0~100 | 主流程关键路径有可见反馈与恢复路径 |
| D（设备） | 0~100 | 替身或真实链路验证之一满足 |
| 约束一致性 | 0~100 | 相关约束文档条目在本轮更新 |
| 连接稳定性 | 0~100 | 重连、超时、错误可观测 |
| 可维护性 | 0~100 | 层级边界清晰、测试覆盖对应逻辑 |

单轮可计分上限 100 分，不是 1 分，但项目目标分每次 commit 只加 1 分。

### 3.2 Given / When / Then 计分门禁

- Given：有 PRD 且任务为实现线
- When：按约束顺序读齐并提交最小变更
- Then：Required gate 全通过、证据齐备、约束更新、得分 +1

- Given：少一项 Required gate
- When：尝试提交
- Then：commit 禁止，得分 +0，必须补齐后重试

### 3.3 闭环失败降权规则

- 连续两次闭环失败，下一轮必须先补齐失败清单。
- 连续三次失败，必须提出拆分和风险收敛动作。

### 3.4 每轮固定最小考核（高效率）

每次关闭/收口前必须完成：
1. `uv run test-embeddebug-py`
2. `uv run start-embeddebug --smoke`
3. `cmd /c EmbedDebug.bat --smoke`
4. 未满足时本轮不得记 `+1`

---

## 四、闭环与多视角自检（统一引用 09-closed-loop）

> ⚠️ **本节原定义的 "S-I-P-E-V-R-L 七环" 和 "A-1/A-2/D-1/D-2/D-3/P-1/U-1 七角色子代理闭环" 已全部废除**。
>
> 历史问题：单 Agent 会话（如 ZCode）无法自动派发 7 个并行子代理，导致门禁永远 +0（幽灵门禁）；
> 且同一项目存在 3 套互不引用的闭环编号（02/06/serial_station_architecture 各一套），AI 不知跑哪套。
>
> **统一权威定义见 [09-closed-loop.md](09-closed-loop.md)**：
>
> - **§一.1 5 视角自检**（取代旧 7 角色子代理）：架构 / 实现 / 测试 / 产品 / 用户，由当前 Agent 一次性打勾
> - **§一.2 6 门禁**：test + smoke + bat + lint + check-constraints + 行数
> - **§五 commit message 模板**（取代旧多版本）
> - **§六 并行规则**：单 Agent 默认串行，多 Agent 并行需用户显式批准

### 4.1 日常与复杂两种节奏

- 日常开发：每个 commit 运行一次 [09-closed-loop §一](09-closed-loop.md) 的 5 视角 + 6 门禁。
- 复杂重构：每个阶段结束再运行一次补充闭环。

### 4.2 量化闭环输出（强制）

每次关闭闭环必须输出（详见 [09-closed-loop §1.3](09-closed-loop.md)）：

1. 5 视角打勾结果（架构/实现/测试/产品/用户）
2. 6 门禁通过情况
3. 三轴 E/U/D 当前值与说明
4. 评分变化（+0 / +1）
5. 未满足项及修复优先级（P0/P1/P2）

### 4.3 门禁失败 → LOOP

按 [docs/superpowers/LOOP_PROTOCOL.md](../superpowers/LOOP_PROTOCOL.md) 分层处置（详见 [09-closed-loop §三](09-closed-loop.md)）：

- 环境/启动失败 → Doctor（只读体检）
- 可复现缺陷 → Debug（最小修复 + 回归）
- 重复/过大 → Simplify（拆分，不扩面）

GO 循环 20 轮或 30 分钟触发安全刹车，必须进 LOOP，不允许盲目继续。

---

## 五、工作流分层（每次迭代都要写）

### 5.1 三轴状态定义（必须每轮记录）

- `E`（工程）: 设计/实现/测试/启动/打包完整度
- `U`（用户）: 主流程可达性与反馈体验
- `D`（设备）: 单测/替身/虚拟/真实硬件

#### 最小可发布口径
- 工程：至少 `E3` 且对应 Python 测试或启动证据
- 用户：至少 `U3`（主流程可用）
- 设备：至少 `D2`（替身/模拟）才可称可测试闭环
- 证据中必须含 Python 测试或启动 smoke 结果

### 5.2 迭代收口模板（必须）

每轮结束在约束文档或 PRD 中记录：

- 本轮影响模块、三轴目标、达成值
- 已完成文件与未完成文件
- 证据命令（build/test/启动）
- 现有阻塞（若有）
- 下一轮最小可启动项

---

## 六、并行与分工规则

> 单 Agent 会话（如 ZCode 单实例）**默认串行**执行 [09-closed-loop §一](09-closed-loop.md) 的 5 视角 + 6 门禁，
> 无需派发多角色子代理。多 Agent 并行需用户显式批准。

### 6.1 并行启动前置条件

仅在以下条件**全部**满足时允许并行（且需用户显式批准）：

- 有 BATCH 拆分
- 任务边界互不重叠
- 接口冻结
- 每个任务都有独立验收点
- 共享入口（`pyproject.toml`、`uv.lock`、`README.md`、`EmbedDebug.bat`、`python/embeddebug/app/`、`resources/themes/`）单 Agent 串行

### 6.2 强制单一负责人文件

- `pyproject.toml`、`uv.lock`
- `README.md`
- `EmbedDebug.bat`、启动脚本
- `python/embeddebug/app/`
- `python/embeddebug/serial_station/ui/`
- `resources/themes/` 与图标资源
- Python 公共接口与共享值对象
- 串口工站的入口与核心控制类

### 6.3 合流规则

并行任务结束后，必须进行一次主线合流审查：

- 接口与契约是否一致
- 三轴状态是否一致
- 文档引用是否更新
- 证据链是否闭合（测试/启动/打包）

## 十、提交节奏与推送触发（与 06-git-commit 联动）

- 以可通过门禁的提交为单位计数，累计两次后必须触发一次 push 节点（第 2、4、6 次提交后）。
- 每次 push 前必须更新 `README.md` 的“企业级精简版”片段（状态、可验证能力、启动路径、最近评分/目标）。
- push 前若未更新 README，视为收口缺陷，不得进入下轮提交加分。
- push 节拍按“可通过门禁提交计数”而非文件类型计数；文档-only commit 达标同样计入闭环。
- README 的精简版需优先满足：
  - 可读性（非堆砌）
  - 可执行性（包含启动链路和 3~5 秒 smoke 指南）
  - 可核验性（包含证据入口）

---

## 七、README 与对外说明

- README 是入口文档，必须与真实状态一致。
- 未达到 `E4 + U3 + D2` 的能力不得在宣传口径写“稳定可用”。
- 出现 `stub`、TODO、未接入主路径、不含入口的能力，统一标注“预览/实验/未完成”。
- 任何能力改动必须指向证据路径（源码/测试/日志/启动命令）。

---

## 八、文档线（本模块约束）

### 8.1 文档更新触发器

- 新增/重构模块结构：`03-architecture.md`、`07-directory-structure.md`
- UI 规则变更：`05-ui-standard.md`
- Git 提交流程与提交格式变更：`06-git-commit.md`
- 图标/主题规范变更：`08-icon-standard.md`
- 串口重构边界更新：`docs/serial_station_architecture.md`

### 8.2 文档收口要求

- 不得出现“已完成”无状态的口径
- 所有能力条目附 `E/U/D`
- 约束文档与代码状态必须至少一致一轮，不得倒挂

---

## 九、每轮最小检查清单

- [ ] PRD 与本轮目标一致，非空
- [ ] 约束加载顺序完整命中（最先读 [00-ssot](00-ssot.md)）
- [ ] [09-closed-loop §一](09-closed-loop.md) 5 视角自检全部打勾（架构/实现/测试/产品/用户）
- [ ] [09-closed-loop §一](09-closed-loop.md) 6 门禁全绿（或未跑项已写明原因）
- [ ] 三轴状态更新（`E/U/D`）
- [ ] Python 测试命令执行可复现
- [ ] Python/PyQt smoke 启动成功
- [ ] `EmbedDebug.bat` 验证（涉及启动链路时）
- [ ] `uv run check-constraints` 退出码 0
- [ ] 代码变更文件与约束文档已同步
- [ ] 下一轮入口清晰，无遗留"已完成"误标
- [ ] 评分更新到 `docs/tracking/SCORE_TRACKING.md` 首行（唯一处），本轮 `+0` 或 `+1` 并说明原因
- [ ] 闭环结果追加到 `docs/tracking/LOOP_STATE.md`

## 十一、工具链效率：uv 与 PyInstaller 评估门槛

- 当前项目默认采用 `uv + PyQt6 + PyInstaller`。
- 优先优化 Python 启动、依赖锁定、测试分层和 PyInstaller workpath。
- 如需更换打包器或引入新运行时，必须先提交单独 PRD，并证明不破坏：
  - `EmbedDebug.bat -> uv run start-embeddebug`
  - `uv run test-embeddebug-py`
  - `uv run package-embeddebug`
  - `uv run verify-package-embeddebug`
