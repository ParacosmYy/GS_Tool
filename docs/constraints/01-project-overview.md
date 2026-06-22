# 01 - 项目概况、构建环境与约束入口

> 适用范围：所有开发任务。
> 版本：2026-06-14 重构版
> 更新目标：把项目基线、启动链路、模块结构、三轴状态与约束加载顺序统一化，并引入可量化评分闭环。

---

## 一、项目基础信息

> ⚠️ 评分/版本/分支等动态值**不在本表硬编码**，统一引用 [00-ssot.md](00-ssot.md) 的 canonical 落点。
> `uv run check-constraints` 会机械检测漂移。

| 项 | 值 |
|----|-----|
| 项目名称 | EmbedDebug |
| 仓库路径 | `E:\Embedded\Tool\Serial_tool\User_Serial`（运行 `git rev-parse --show-toplevel` 复核） |
| 远端仓库 | `https://github.com/ParacosmYy/GS_Tool.git` |
| 默认分支 | `feat/embed-debug`（运行 `git branch --show-current` 复核） |
| 当前版本 | 见 `pyproject.toml` `[project].version` |
| **当前评分** | **见 [docs/tracking/SCORE_TRACKING.md](../tracking/SCORE_TRACKING.md) 首行**（canonical 唯一） |
| 目标评分 | 1000（起始分 1，每通过门禁 commit +1，规则见 [09-closed-loop §二](09-closed-loop.md)） |
| 最终三轴目标 | `E5 + U4 + D4` |

---

## 二、强制加载顺序（每次任务）

1. **最先读 [00-ssot.md](00-ssot.md)** — 单一真相源指针，确认真相落点，禁止硬编码动态值
2. `CLAUDE.md`（最顶层铁律，修改约束前必须核对）
3. `docs/constraints/01-project-overview.md`（本文件）
4. 涉及架构新增类/跨层修改：`docs/constraints/03-architecture.md`
5. 涉及文件新建/移动/删除：`docs/constraints/07-directory-structure.md`
6. 涉及串口工具：`docs/serial_station_architecture.md`
7. 关联构建行为：`docs/constraints/06-git-commit.md`
8. **每次 commit 前：`docs/constraints/09-closed-loop.md`** — 5 视角 + 6 门禁

> 任何偏离这条加载链路的任务，视为流程未开始，不执行变更。

## 二-a、AI 可执行评分目标（全项目统一）

> 评分规则、加分门禁、push 节奏、三轴提升证据已统一收口到
> **[09-closed-loop.md §二 评分闭环](09-closed-loop.md)** 和 **§四 三轴状态**。
>
> 本节不再重复定义，避免历史多源矛盾（旧文本曾同时出现 500/618/787 三个"当前分"）。

要点速览（详见 09-closed-loop）：

- 起始分 1，目标 1000，每通过门禁 commit `+1`
- canonical 唯一落点：`docs/tracking/SCORE_TRACKING.md` 首行
- 其他文档禁止硬编码分数，引用方式见 [00-ssot.md](00-ssot.md)
- 每 2 个通过门禁 commit 为 1 个 push 周期，push 前必须更新 README
- 评分闭环触发条件：PRD/Specs 已更新 + 至少一类验收证据可复现 + 无越界 + 三轴至少一项有真实改变 + smoke 退出码 0

### 量化里程碑（参考）

| 分数档 | 目标 | 说明 |
|---|---|---|
| 1~199 | 起步期 | 基础设施、协议层、OTA 体系逐步成型 |
| 200~599 | 约束对齐期 | 将已有规则统一到可执行清单 |
| 600~799 | 结构与流程稳定期 | 形成闭环执行模板并落地 |
| 800~899 | 体验一致性期 | UI、日志、回放、脚本、错误可恢复能力齐备 |
| 900~999 | 交付前硬化期 | 完整闭环证据、回归率与风险缓释 |
| 1000 | 目标达成 | 可持续执行的主线与长期开启状态 |

### 可量化最小验收（每 20 分）

每完成 20 分至少满足一条：

1. 一个主要闭环从触发到验收闭环可追踪。
2. 至少 2 个约束文档条目更新且与代码状态一致。
3. 一个关键场景有三轴证据（至少 E 或 U 或 D）。
4. 进行一次最小可复现 smoke：`uv run start-embeddebug --smoke` 与 `cmd /c EmbedDebug.bat --smoke`。


---

## 三、Python/PyQt 启动、测试与打包环境

### 3.1 环境

- uv：PATH 中可执行 `uv`
- Python：由 `uv` 根据 `pyproject.toml` 与 `uv.lock` 管理
- GUI：PyQt6
- 打包：PyInstaller（通过 `uv run package-embeddebug`）
- J-Link：`E:/Embedded/Tool/SEGGER_IOT/JLink_V932`

### 3.2 Python/PyQt 工具链

```powershell
uv run start-embeddebug
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run package-embeddebug --version smoke --clean
uv run verify-package-embeddebug --package-dir dist\EmbedDebugPy-smoke-windows-x64
```

### 3.3 启动最低验收线

- `EmbedDebug.bat` 是用户侧最低验收入口，必须双击可启动。
- 自 PRD-136/B22 起，`EmbedDebug.bat` 默认且唯一活跃启动链路为 Python/PyQt：`uv run start-embeddebug`。
- 遗留 native 打包链路、源码树和测试入口已移除，不再作为活跃工程入口、fallback 或验收口径。
- 建议每轮考核固定执行：
  - `uv run start-embeddebug --smoke`
  - `cmd /c EmbedDebug.bat --smoke`
- 任何涉及 Python 依赖、路径、资源、部署、启动脚本的修改，收口前必须验证 Python/PyQt 默认入口；不能验证必须写明缺失组件与修复命令。
- 严禁平行构建目录：`build2/`、`build-debug/`、`build-release/`、`native-build-*`。

---

## 四、模块与目录快照（当前）

### 4.1 已确认主模块

| 模块 | 目录 | 职责 | 当前状态 |
|------|------|------|----------|
| app | `python/embeddebug/app/` | Python/PyQt 应用入口与 smoke | 主线 |
| serial_station | `python/embeddebug/serial_station/` | 串口工作台 | 主线 |
| devtools | `python/embeddebug/devtools/` | 测试、打包、验证入口 | 主线 |
| tests | `tests/python/` | Python 单测、集成测试、UI smoke | 主线 |

### 4.2 新增优先落地点（必须使用）

- `python/embeddebug/`：产品运行时代码
- `python/embeddebug/serial_station/`：串口工作台主线
- `tests/python/`：Python 测试
- `tests/fixtures/`：协议样本、日志和回放 fixtures

---

## 五、项目质量口径（必须同步到所有说明）

### 5.1 三轴状态

- `E` 工程：`E0/E1/E2/E3/E4/E5`
- `U` 用户：`U0/U1/U2/U3/U4`
- `D` 设备：`D0/D1/D2/D3/D4`

### 5.2 状态提升条件（最小集合）

- `E` 仅能在源码、关键路径测试、启动或打包证据齐备时提升。
- `U` 仅有主入口 + 完整闭环+错误反馈时可提升。
- `D` 外设能力未有 `D2+` 不能提升为“硬件可用”。

### 5.3 约束入口联动

> 任何功能/文档升级不能跳过约束链路。
> 约束映射：

- 工程风格与边界：`03-architecture.md`
- 工作流与 PRD/Specs：`02-workflow.md`
- UI 改造：`05-ui-standard.md`
- 文件变更与目录：`07-directory-structure.md`
- 图标与主题：`08-icon-standard.md`
- 代码提交与验收：`06-git-commit.md`
- 串口专项：`docs/serial_station_architecture.md`

---

## 六、Serial Station 当前目标（不写在这里就不能开始该子域）

- Python/PyQt 新增落点为 `python/embeddebug/serial_station/`，明确 `ui`、`controllers`、`core`、`protocols`、`services`、`workers`、`drivers` 分层。
- 遗留串口目录已移除，不再新增默认交付能力。
- 首先实现用户闭环（连接-发送-接收-日志-导出-回放），再扩展高级功能。
- 任何新协议必须有 `tests/python/` 对应 parser/build 测试。
- 真实设备验证与模拟验证必须记录，不允许只在 parser 测试下宣称“工程可调试”。

---

## 七、即时行动清单（适用于本轮）

1. 统一补齐约束文档并在 PRD/Specs/迭代说明中引用本文件版本。
2. 串口工站与主界面所有入口需能在版本文档中明确 `E/U/D` 和验收链接。
3. 任何宣传文案需与文档证据一致，避免“完成”误导。

## 八、项目评分执行日志要求（AI 永久执行）

> 本节已并入 [09-closed-loop.md §二 评分闭环](09-closed-loop.md) 和
> [docs/tracking/LOOP_STATE.md](../tracking/LOOP_STATE.md)。
>
> 历史格式（保留参考）：

每次任务结束追加一条评分日志到 `docs/tracking/LOOP_STATE.md`（闭环状态表）和 `docs/tracking/SCORE_TRACKING.md`（评分历史），格式如下：

- 时间：YYYY-MM-DD HH:mm
- 本轮增量：`+1`（门禁全绿时；未过为 `+0`）
- 当前得分：见 `docs/tracking/SCORE_TRACKING.md` 首行（canonical）
- 本轮影响：`文件列表`、`约束文档列表`、`E/U/D`变更
- 证据摘要：`build/test/startup/doc` 三类至少 1 项
- smoke 证据：`uv run start-embeddebug --smoke` 与 `cmd /c EmbedDebug.bat --smoke` 的退出码
- 阶段：当前所处的量化里程碑区间（见 §二-a）
- 5 视角 + 6 门禁结果：见 [09-closed-loop §一](09-closed-loop.md)
