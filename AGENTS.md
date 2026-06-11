# AGENTS.md - Agent 强制入口

> 本文件是 AI Agent 进入仓库后的稳定入口。
> 只保留索引、加载顺序和不可绕过的边界规则；详细约束以被索引文档为准。

---

## 一、强制加载顺序

任何 Agent 在修改代码或文档前，必须按任务类型加载对应文档：

1. 每次任务先读 [CLAUDE.md](CLAUDE.md)。
2. 每次开发先读 [docs/constraints/01-project-overview.md](docs/constraints/01-project-overview.md)。
3. 涉及实现、重构、新类、跨模块调用时，必须读 [docs/constraints/03-architecture.md](docs/constraints/03-architecture.md)。
4. 涉及新建、移动、删除文件时，必须读 [docs/constraints/07-directory-structure.md](docs/constraints/07-directory-structure.md)。
5. 涉及串口上位机重构、串口协议新增、串口收发框架拆分时，必须读 [docs/serial_station_architecture.md](docs/serial_station_architecture.md)。

---

## 二、约束模块索引

| 场景 | 必读文档 |
|------|----------|
| 项目概况、构建环境、模块清单 | [docs/constraints/01-project-overview.md](docs/constraints/01-project-overview.md) |
| 工作流、PRD、技术债、文档线 | [docs/constraints/02-workflow.md](docs/constraints/02-workflow.md) |
| 架构分层、依赖方向、新增类 | [docs/constraints/03-architecture.md](docs/constraints/03-architecture.md) |
| C++ 编码、命名、线程、测试 | [docs/constraints/04-coding-standard.md](docs/constraints/04-coding-standard.md) |
| UI、QSS、objectName、tr() | [docs/constraints/05-ui-standard.md](docs/constraints/05-ui-standard.md) |
| Git、commit、提交前检查 | [docs/constraints/06-git-commit.md](docs/constraints/06-git-commit.md) |
| 目录结构、canonical 路径 | [docs/constraints/07-directory-structure.md](docs/constraints/07-directory-structure.md) |
| 图标、SVG、IconManager | [docs/constraints/08-icon-standard.md](docs/constraints/08-icon-standard.md) |
| Serial Station C++ 分层 | [docs/serial_station_architecture.md](docs/serial_station_architecture.md) |

---

## 三、不可绕过规则

1. 不经 PRD 不直接写新功能代码。
2. 不经架构检查不新增跨模块类。
3. `MainWindow` 和 `PanelManager` 只做装配、导航和面板编排，不写业务逻辑。
4. 公共能力优先复用既有组件，不重复造 CRC、Hex、RingBuffer、Settings、日志等基础能力。
5. 新增 `.h/.cpp` 必须加入 `CMakeLists.txt`，不得提交构建系统未引用的死代码。
6. 不提交 build 产物、Qt 生成文件、二进制产物或 CMake 缓存。
7. 用户已有改动不得擅自回退。
8. `EmbedDebug.bat` 双击能启动是最低验收线；任何影响构建、启动、入口、资源、依赖、路径的改动，收口前必须验证或说明无法验证的具体原因。
9. 构建目录永远只允许 `build/` 一个；禁止创建、引用或兼容 `build2/`、`build-debug/`、`build-release/` 等平行构建目录。

---

## 四、Serial Station 特别规则

后续 C++ 串口上位机重构统一落点为 `src/apps/serial_station/`。

1. UI 只通过 Qt signal/slot 调用 `SerialStationController`。
2. `core/` 只处理 `QByteArray` 收发、线程、会话、dispatcher、codec。
3. `protocols/` 只处理命令构建、帧定义、流式解析。
4. `services/` 只处理日志、导出、回放、设备档案。
5. `workers/` 不持有 QWidget，不直接更新 UI。
6. `core/` 不 include 具体协议目录，只依赖 `ISerialProtocol` 和 registry。
7. 协议目录不 include UI、QWidget、`SerialStationWindow`、`SerialStationController`。
8. 新增协议必须配套 QTest，测试放在 `tests/serial_station/`。

---

## 五、收口检查

每次完成任务前，至少确认：

- [ ] 修改范围符合本次任务边界。
- [ ] 必读约束文档已经加载。
- [ ] 新增文件路径符合 canonical 目录。
- [ ] 新增源码已加入 CMake。
- [ ] 涉及 UI 的控件有 `objectName`，用户可见文字用 `tr()`。
- [ ] 涉及 Serial Station 的改动没有越过 ui/controller/core/protocols/services/workers 边界。
- [ ] `EmbedDebug.bat` 双击启动路径仍然可用；不能验证时已写明原因。
- [ ] 能运行的测试或构建命令已经运行；不能运行时说明原因。
