# AGENTS.md - Agent 强制入口

> 本文件是 AI Agent 进入仓库后的稳定入口。
> 只保留索引、加载顺序和不可绕过的边界规则；详细约束以被索引文档为准。

---

## 一、强制加载顺序

任何 Agent 在修改代码或文档前，必须按任务类型加载对应文档：

1. **每次最先读 [docs/constraints/00-ssot.md](docs/constraints/00-ssot.md)** — 单一真相源指针，确认真相（评分/测试数/分支/版本）落在哪，禁止凭记忆硬编码。
2. 每次任务先读 [CLAUDE.md](CLAUDE.md)。
3. 每次开发先读 [docs/constraints/01-project-overview.md](docs/constraints/01-project-overview.md)。
4. 涉及实现、重构、新类、跨模块调用时，必须读 [docs/constraints/03-architecture.md](docs/constraints/03-architecture.md)。
5. 涉及新建、移动、删除文件时，必须读 [docs/constraints/07-directory-structure.md](docs/constraints/07-directory-structure.md)。
6. 涉及串口上位机重构、串口协议新增、串口收发框架拆分时，必须读 [docs/serial_station_architecture.md](docs/serial_station_architecture.md)。
7. **每次 commit 前必须读 [docs/constraints/09-closed-loop.md](docs/constraints/09-closed-loop.md)** — 5 视角自检 + 6 门禁的唯一定义。

---

## 二、约束模块索引

| 场景 | 必读文档 |
|------|----------|
| **真相落点（评分/测试数/分支）** | [docs/constraints/00-ssot.md](docs/constraints/00-ssot.md) |
| 项目概况、构建环境、模块清单 | [docs/constraints/01-project-overview.md](docs/constraints/01-project-overview.md) |
| 工作流、PRD、技术债、文档线 | [docs/constraints/02-workflow.md](docs/constraints/02-workflow.md) |
| 架构分层、依赖方向、新增类 | [docs/constraints/03-architecture.md](docs/constraints/03-architecture.md) |
| Python/PyQt 编码、命名、线程、测试 | [docs/constraints/04-coding-standard.md](docs/constraints/04-coding-standard.md) |
| UI、QSS、objectName、tr() | [docs/constraints/05-ui-standard.md](docs/constraints/05-ui-standard.md) |
| Git、commit、提交前检查 | [docs/constraints/06-git-commit.md](docs/constraints/06-git-commit.md) |
| 目录结构、canonical 路径 | [docs/constraints/07-directory-structure.md](docs/constraints/07-directory-structure.md) |
| 图标、SVG、IconManager | [docs/constraints/08-icon-standard.md](docs/constraints/08-icon-standard.md) |
| **闭环协议（5 视角 + 6 门禁，唯一权威）** | [docs/constraints/09-closed-loop.md](docs/constraints/09-closed-loop.md) |
| Serial Station Python/PyQt 分层 | [docs/serial_station_architecture.md](docs/serial_station_architecture.md) |

---

## 三、不可绕过规则

1. 不经 PRD 不直接写新功能代码。
2. 不经架构检查不新增跨模块类。
3. `MainWindow` 和 `PanelManager` 只做装配、导航和面板编排，不写业务逻辑。
4. 公共能力优先复用既有组件，不重复造 CRC、Hex、RingBuffer、Settings、日志等基础能力。
5. 新增 Python 源码必须落在 canonical 包路径，并被 `uv run test-embeddebug-py` 或相邻测试覆盖。
6. 不提交 `build/`、`dist/`、`.venv/`、PyInstaller 临时产物、Qt 生成文件或二进制产物。
7. 用户已有改动不得擅自回退。
8. `EmbedDebug.bat` 通过 `uv run start-embeddebug` 启动是最低验收线；任何影响启动、入口、资源、依赖、路径的改动，收口前必须验证或说明无法验证的具体原因。
9. 仓库不恢复遗留原生工程线；禁止新增、引用或兼容平行 native 构建目录。

---

## 四、Serial Station 特别规则

后续 Python/PyQt 串口上位机重构统一落点为 `python/embeddebug/serial_station/`。

1. UI 只通过 PyQt signal/slot 或 controller intent 调用 `SerialStationController`。
2. `core/` 只处理 bytes 收发、线程/任务、会话、dispatcher、codec。
3. `protocols/` 只处理命令构建、帧定义、流式解析。
4. `services/` 只处理日志、导出、回放、设备档案。
5. `workers/` 不持有 QWidget，不直接更新 UI。
6. `core/` 不依赖具体协议目录，只依赖协议接口和 registry。
7. 协议目录不依赖 UI、QWidget、`SerialStationWindow`、`SerialStationController`。
8. 新增协议必须配套 pytest，测试放在 `tests/python/`。

---

## 五、收口检查

每次完成任务前，至少确认：

- [ ] 修改范围符合本次任务边界。
- [ ] 必读约束文档已经按 §一 加载顺序读齐（**最先读 00-ssot**，**commit 前读 09-closed-loop**）。
- [ ] 新增文件路径符合 canonical 目录。
- [ ] 新增 Python 源码落在 canonical 包路径，并有相邻测试或既有命令覆盖。
- [ ] 涉及 UI 的控件有 `objectName`，用户可见文字用 `tr()`。
- [ ] 涉及 Serial Station 的改动没有越过 ui/controller/core/protocols/services/workers 边界。
- [ ] [09-closed-loop §一](docs/constraints/09-closed-loop.md) 5 视角自检全部打勾（架构/实现/测试/产品/用户）。
- [ ] [09-closed-loop §一](docs/constraints/09-closed-loop.md) 6 门禁全绿（或未跑项已写明原因）。
- [ ] `uv run check-constraints` 退出码 0（评分/SSOT/行数漂移检测通过）。
- [ ] `EmbedDebug.bat` -> `uv run start-embeddebug` 启动路径仍然可用；不能验证时已写明原因。
- [ ] 评分已更新到 `docs/tracking/SCORE_TRACKING.md` 首行（唯一处），其他文档未复制硬编码分数。
- [ ] 闭环结果已追加到 `docs/tracking/LOOP_STATE.md`。
