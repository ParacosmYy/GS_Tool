# 09 - 闭环协议（唯一权威）

> 适用范围：所有涉及代码、测试、文档变更的迭代。
> **本文件是项目唯一的闭环定义**。02-workflow / 06-git-commit / serial_station_architecture 里旧的闭环编号（S-I-P-E-V-R-L 七环、A-I-R-C-L-M-P 七环、S-E-L-T-V-Q-R + P-U-L-G-S-B 双段十三环）**已全部废除**，统一引用本文件。

---

## 为什么需要唯一闭环

历史教训（2026-06-22 审计）：同一项目里有 3 套互不引用的闭环编号，AI 不知道跑哪一套，干脆都不跑；
另有"7 角色子代理回执"门禁要求单 Agent 无法自动派发，成为永远过不了的幽灵门禁。
本文件用**5 视角自检 + 6 机械门禁**取代所有旧编号，确保单 Agent 会话也能完整闭环。

---

## 一、单轮闭环（5 视角 + 6 门禁）

每个 commit 前必须完成，结果写进 commit message。

### 1.1 5 视角自检（取代旧 7 角色子代理）

单 Agent 会话默认串行，无需派发 7 个子代理。改为当前 Agent 逐视角自检打勾：

| 视角 | 检查问题 | 通过条件 |
|------|----------|----------|
| 架构 | 改动是否越层？core→protocols？ui→transport？ | 依赖方向满足 [03-architecture](03-architecture.md) 矩阵 |
| 实现 | 是否复用公共能力？有无重复造轮子？ | CRC/Hex/RingBuffer/Settings/日志未重复实现 |
| 测试 | 新增/改动逻辑有对应 pytest？错误路径有覆盖？ | 至少 1 正向 + 1 异常路径 |
| 产品 | 三轴 E/U/D 真实变化？无 stub/TODO 冒充完成？ | E/U/D 写进 commit msg 且有证据路径 |
| 用户 | 主流程入口可达？错误有可见反馈？ | U 轴提升需有用户路径证据 |

任一视角未通过 → 本轮不得 commit，转 §三 LOOP 处置。

### 1.2 6 门禁（必须全绿，机械可复现）

| # | 命令 | 通过条件 | 何时必跑 |
|---|------|----------|----------|
| 1 | `uv run test-embeddebug-py` | 退出码 0，无新增失败 | 每次 commit |
| 2 | `uv run start-embeddebug --smoke` | 退出码 0 | 每次 commit |
| 3 | `cmd /c EmbedDebug.bat --smoke` | 退出码 0 | 启动/入口/依赖/资源/路径改动时 |
| 4 | `uv run lint-embeddebug-py` | 无新增错误 | 每次 commit |
| 5 | `uv run check-constraints` | 退出码 0 | 每次 commit |
| 6 | （含在门禁 5 内）文件行数 ≤300（.py）/ ≤250（test） | 脚本检测 | 每次 commit |
| 7 | （含在门禁 5 内）测试组织：无新增孤儿文件 / unit 层不混 PyQt | 脚本检测 | 每次 commit |

**门禁 5（check-constraints）机械检测清单**（详见 [tools/check_constraints.py](../../tools/check_constraints.py)）：
- 评分一致性：CLAUDE.md / README.md / 01-overview / 06 出现的分数与 SCORE_TRACKING 一致
- SSOT 合规：核心文档无硬编码 `NNNN passed` / `NNN 个测试文件`
- 冻结目录：仓库根无 `src/`、`build2/`、`native-build-*`
- 文件行数：runtime ≤300 / test ≤250
- **测试孤儿**：新增 `test_*.py` 若 <3 个测试函数即阻断（既有孤儿只警告，列技术债）
- **unit 层纯度**：新增 unit 测试若 import PyQt/pyqtgraph 即阻断（应放 ui_smoke/ 或用 fake）

测试组织规则详见 [CLAUDE.md §测试文件组织规则](../../CLAUDE.md) 和 [04-coding-standard §六](04-coding-standard.md)。

不能运行某门禁时（如无 GUI 环境），必须在 commit message 写明"未跑 X，原因 Y"，不得静默跳过。

### 1.3 闭环产出

每次 commit 必须在 message 里包含：
- 5 视角打勾结果（架构/实现/测试/产品/用户）
- 6 门禁通过情况
- 三轴 E/U/D 当前值与本轮变化
- 评分变化（旧→新）

---

## 二、评分闭环

### 2.1 评分规则

- 起始分 1，目标 1000
- 每个通过 §一 全部门禁的 commit：**+1 分**
- 门禁任一失败：本轮 +0，不得进入加分闭环
- canonical 唯一落点：[docs/tracking/SCORE_TRACKING.md](../tracking/SCORE_TRACKING.md) 首行
- 其他文档**禁止复制分数**，只能引用（见 [00-ssot.md](00-ssot.md)）

### 2.2 Push 节奏

- 每 2 个通过门禁的 commit 为 1 个 push 周期（第 2/4/6 次 commit 后 push）
- push 前必须更新 `README.md` 状态表（评分、能力、证据入口）
- push 前若未更新 README，视为收口缺陷，不得进入下一轮加分

### 2.3 评分历史

每次 +1 后，在 `docs/tracking/SCORE_TRACKING.md` 追加一行（格式见该文件）。
**只在 canonical 处维护**，CLAUDE.md / 01-overview / 06 等文档一律引用，不复制。

---

## 三、门禁失败处置 → LOOP

门禁连续失败时，按 [docs/superpowers/LOOP_PROTOCOL.md](../superpowers/LOOP_PROTOCOL.md) 分层处置：

| 失败特征 | 路由 | 动作 |
|----------|------|------|
| 环境/启动/工具链失败 | **Doctor** | 只读体检（git status、uv、依赖），不写代码 |
| 可复现崩溃/编译错误/逻辑错误 | **Debug** | 最小修复 + 回归测试 |
| 重复实现/文件过大/职责混乱 | **Simplify** | 拆分或删重，不扩面 |

### 3.1 安全刹车

GO 循环触发以下任一条件，必须停止当前循环，进 LOOP：
- 20 轮未收口
- 30 分钟未收口

刹车后必须输出：最后失败命令、最后错误摘要、下一步建议（路由到 Doctor/Debug/Simplify）。

### 3.2 LOOP 证据归档

- Doctor 报告：控制台摘要即可
- Debug 报告：`docs/reviews/debug/*.md`
- Simplify 报告：`docs/reviews/simplify/*.md`

无证据的 LOOP 不得作为完成依据。

---

## 四、三轴状态（E/U/D）

### 4.1 级别定义

| 轴 | 级别 | 含义 | 提升所需证据 |
|----|------|------|--------------|
| E 工程 | E0~E5 | 代码/测试/启动/打包完整度 | 源码 + pytest + smoke 退出码 |
| U 用户 | U0~U4 | 主流程入口可达 + 反馈体验 | 主流程路径 + 错误反馈证据 |
| D 设备 | D0~D4 | 替身/虚拟/真机验证 | 替身测试 / 虚拟设备 / 真机记录 |

详细级别定义见 [CLAUDE.md §交付状态口径](../../CLAUDE.md)。

### 4.2 宣传与完成门槛

- 对外宣传"完成"最低门槛：**E4 + U3 + D2**
- 文案暗示真实硬件可用：必须 **D4**
- 涉及 UART/CAN/BLE/USB/SPI/I2C 等外设，未到 D4 必须明确写"真机未验证"
- 禁止 stub/TODO/空方法/无入口模块冒充 U3/D3

### 4.3 禁止表述

- 禁止只写"已实现""完成""骨架完成"而无三轴状态
- 禁止把无入口模块描述为用户已可使用
- 禁止把无硬件/虚拟/替身验证的外设能力描述为设备已验证

---

## 五、commit message 模板（唯一权威）

```text
<模块>: <简述改了什么>

门禁: test✓ smoke✓ lint✓ check_constraints✓ [bat✓]
视角: 架构✓ 实现✓ 测试✓ 产品✓ 用户✓
三轴: E<x> U<x> D<x>（本轮变化: <无 / E↑ / U↑ / D↑>）
评分: <旧分> + 1 = <新分>（见 docs/tracking/SCORE_TRACKING.md）
变更: <N> files, <+M> insertions, <-K> deletions
```

说明：
- `[bat✓]` 仅在跑了 `cmd /c EmbedDebug.bat --smoke` 时打勾
- `评分` 行的 `<旧分>` `<新分>` 必须与 SCORE_TRACKING.md 一致
- 若某门禁未跑，写 `X✗（原因：...）`，不得省略

示例：

```text
controllers: Batch 118 — X 状态助手单元测试（5 测试）

门禁: test✓ smoke✓ lint✓ check_constraints✓
视角: 架构✓ 实现✓ 测试✓ 产品✓ 用户✓
三轴: E4 U2 D1（本轮变化: 无，纯测试增量）
评分: 787 + 1 = 788（见 docs/tracking/SCORE_TRACKING.md）
变更: 3 files, +120 insertions, -8 deletions
```

---

## 六、并行与单 Agent 规则

### 6.1 单 Agent 会话（默认）

- 默认串行执行 §一 单轮闭环
- 不要求派发多角色子代理（旧 7 角色已废除）
- 5 视角自检由当前 Agent 一次完成

### 6.2 多 Agent 并行（需用户显式批准）

仅在以下条件全满足时允许并行：
- 有 BATCH 拆分，任务边界互不重叠
- 接口冻结
- 每个任务有独立验收点
- 共享入口（`pyproject.toml`、`uv.lock`、`README.md`、`EmbedDebug.bat`、`python/embeddebug/app/`、`resources/themes/`）单 Agent 串行

并行结束必须做一次主线合流审查：接口契约一致、三轴状态一致、文档引用更新、证据链闭合。

---

## 七、收口 checklist（每次任务完成前）

- [ ] 改动范围符合本次任务边界
- [ ] 必读约束文档已按 [AGENTS.md](../../AGENTS.md) 加载顺序读齐
- [ ] §一 5 视角自检全部打勾
- [ ] §一 6 门禁全绿（或未跑项已写明原因）
- [ ] 新增文件路径符合 [07-directory-structure](07-directory-structure.md) canonical 落点
- [ ] 涉及 UI 的控件有 `objectName`，用户可见文字用 `tr()`
- [ ] 涉及 Serial Station 的改动未越 ui/controller/core/protocols/services/workers 边界
- [ ] commit message 符合 §五 模板
- [ ] 评分已更新到 SCORE_TRACKING.md（唯一处），其他文档未复制
- [ ] 闭环结果已追加到 [docs/tracking/LOOP_STATE.md](../tracking/LOOP_STATE.md)
