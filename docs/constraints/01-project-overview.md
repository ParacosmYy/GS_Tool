# 01 - 项目概况、构建环境与约束入口

> 适用范围：所有开发任务。
> 版本：2026-06-14 重构版
> 更新目标：把项目基线、启动链路、模块结构、三轴状态与约束加载顺序统一化，并引入可量化评分闭环。

---

## 一、项目基础信息

| 项 | 值 |
|----|-----|
| 项目名称 | EmbedDebug |
| 仓库路径 | `D:\Workplace\Embedded_workplace\User_workplace\GS_Tool` |
| 当前版本 | `0.1.0` |
| 远端仓库 | `https://github.com/ParacosmYy/GS_Tool.git` |
| 默认分支 | `feat/embed-debug` |
| 目标 | `D1`（当前工程） → `E5 + U4 + D4`（最终） |
| 当前得分 | `551 / 1000` |
| 目标得分 | `1000 / 1000` |

---

## 二、强制加载顺序（每次任务）

1. `CLAUDE.md`（最顶层铁律，修改约束前必须核对）
2. `docs/constraints/01-project-overview.md`（本文件）
3. 涉及架构新增类/跨层修改：`docs/constraints/03-architecture.md`
4. 涉及文件新建/移动/删除：`docs/constraints/07-directory-structure.md`
5. 涉及串口工具：`docs/serial_station_architecture.md`
6. 关联构建行为：`docs/constraints/06-git-commit.md`

> 任何偏离这条加载链路的任务，视为流程未开始，不执行变更。

## 二-a、AI 可执行评分目标（全项目统一）

### 2-a.1 评分基础

- 当前目标分：`500`。
- 目标分：`1000`。
- 每次通过门禁的 commit：`+1` 分。
- Push 节拍：每 2 次门禁通过 commit 形成一个 Push 周期（第 2、4、6 次...）。
- Push 周期发起前，先更新 README 的企业级精简快照。
- 未通过门禁的 commit：不得提交。
- 任何单轮关闭循环后必须记录得分变化并同步到对应约束条目。

### 2-a.2 评分闭环触发条件

- PRD/Specs 与约束映射都已更新。
- 至少一类验收证据可复现（构建、测试、启动、文档证据）。
- 本轮变更没有越界目录、层级、Python 主线落点等硬边界。
- 每轮评分闭环前必须完成 Python/PyQt 启动 smoke 证据：`uv run start-embeddebug --smoke` 与 `cmd /c EmbedDebug.bat --smoke`。
- 三轴 `E/U/D` 至少一项有真实改变量。
- `EmbedDebug.bat` 相关链路变化需补充启动复测结论。

### 2-a.3 量化里程碑

| 分数档 | 目标 | 说明 |
|---|---|---|
| 500~599 | 约束对齐期 | 将已有规则统一到可执行清单 |
| 600~699 | 结构与流程稳定期 | 形成闭环执行模板并落地 |
| 700~799 | 串口核心能力增强期 | 约束与功能一体化推进 |
| 800~899 | 体验一致性期 | UI、日志、回放、脚本、错误可恢复能力齐备 |
| 900~999 | 交付前硬化期 | 完整闭环证据、回归率与风险缓释 |
| 1000 | 目标达成 | 可持续执行的主线与长期开启状态 |

### 2-a.4 可量化最小验收（每 20 分）

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

每次任务结束追加一条评分日志，格式如下：

- 时间：YYYY-MM-DD HH:mm
- 本轮增量：`+N`（默认 0~1，若门禁未过为 0）
- 当前得分：`xx/1000`
- 本轮影响：`文件列表`、`约束文档列表`、`E/U/D`变更
- 证据摘要：`build/test/startup/doc` 三类至少 1 项
- smoke 证据：`uv run start-embeddebug --smoke` 与 `cmd /c EmbedDebug.bat --smoke` 的退出码
- 阶段：`500->1000` 中的当前区间
- 子代理闭环：A-1 / A-2 / D-1 / D-2 / D-3 / P-1 / U-1 的结论状态
