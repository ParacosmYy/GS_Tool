# BATCH Plan - PRD-077 Serial Station Minimal UART Skeleton

## 总目标

建立 `src/apps/serial_station/` 的最小 UART 可构建骨架，并用 QTest 覆盖协议、注册表和会话状态。该计划供人工审查后再决定是否启用 3 个子 Agent 并行。

## 本轮不做

- 不接入 `MainWindow`。
- 不修改 `PanelManager`。
- 不删除旧小写骨架文件。
- 不实现完整 UI。
- 不实现 Modbus RTU/custom_md。
- 不接真实 COM 口。

## 共享约定

- 接口：`ISerialProtocol` 只暴露 `name()`、`buildCommand()`、`feed()`、`reset()`。
- 事件：`SerialProtocolEvent` 只用 Qt Core 类型，不包含 UI 文案。
- 命名：新增生产文件全部 PascalCase。
- CMake：由主 Agent 串行修改 `CMakeLists.txt` 和 `tests/CMakeLists.txt`。
- QSS token：本阶段不新增 UI，不改 QSS。
- 验证命令：主目标 build、Serial Station QTest、source-tree audit。

## 子任务

| # | 任务 | 可改文件 | 禁止文件 | 验收方式 | 并行组 |
|---|------|----------|----------|----------|--------|
| 1 | 定义协议事件和协议接口 | `src/apps/serial_station/protocols/SerialProtocolEvent.h`, `ISerialProtocol.h` | `CMakeLists.txt` 之外所有共享入口 | 头文件不 include QWidget | A |
| 2 | 实现 ASCII 文本协议 | `protocols/ascii_text/AsciiTextProtocol.h/.cpp` | `ui/`, `core/SerialManager.*` | `test_ascii_text_protocol` | B |
| 3 | 实现协议注册表 | `protocols/SerialProtocolRegistry.h/.cpp` | 具体协议目录之外文件 | `test_serial_protocol_registry` | B |
| 4 | 定义配置和模型 | `SerialStationConfig.h/.cpp`, `SerialStationModels.h`, `SerialStationConstants.h` | 旧 `config.h/constants.h/models.h` | 编译通过 | A |
| 5 | 实现会话状态 | `core/SerialSession.h/.cpp` | UI 和具体协议目录 | `test_serial_session` | B |
| 6 | 实现 SerialPort/SerialManager 最小骨架 | `core/SerialPort.h/.cpp`, `core/SerialManager.h/.cpp` | 具体协议目录、UI | 编译通过，不依赖真实 COM 测试 | C |
| 7 | 实现 App/Window/Controller 骨架 | `SerialStationApp.*`, `SerialStationWindow.*`, `SerialStationController.*` | `MainWindow`, `PanelManager` | 编译通过，Window 只装配 | C |
| 8 | CMake 和测试合流 | `CMakeLists.txt`, `tests/CMakeLists.txt`, `tests/serial_station/*` | 启动脚本 | build + ctest + audit | 主 Agent |

## 合流顺序

1. 先合流协议接口、事件、配置模型。
2. 再合流 ASCII 协议、注册表和会话测试。
3. 再合流 core/app/controller 骨架。
4. 最后由主 Agent 修改 CMake 并统一构建、测试、审计。

## 冲突处理

- 同文件冲突：停止并行，主 Agent 串行合并。
- 架构冲突：回到 `docs/serial_station_architecture.md` 和 Specs 修订。
- 构建失败：进入 LOOP Debug。
- 重复实现：进入 LOOP Simplify，不允许带重复代码合流。

## 并行度建议

未审查前并行度为 `1`。人工审查通过后可使用 `3` 个子 Agent：

- A：接口/配置模型。
- B：协议/注册表/测试。
- C：core/app/controller 骨架。

CMake、启动、主目标验证仍由主 Agent 串行执行。
