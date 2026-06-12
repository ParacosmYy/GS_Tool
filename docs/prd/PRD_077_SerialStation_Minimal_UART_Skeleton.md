# PRD-077 - Serial Station Minimal UART Skeleton

## 背景

PRD-076 的只读审计确认：

- 旧主线 UART 配置链路存在，并已进入 CMake。
- 新 `src/apps/serial_station/` 只有 6 个未接入 CMake 的小写骨架文件。
- 现有 `window.h` include 了不存在的 `controller.h`，说明该骨架不能作为可构建模块使用。

后续串口上位机重构必须进入 `src/apps/serial_station/`，但不能继续堆在旧 `src/serial/`、`src/protocol/` 或 `MainWindow` 中。

## 目标

建立 Serial Station 的最小 UART 可构建骨架，为后续 UART 配置 UI、串口打开/关闭、ASCII 文本协议和日志服务提供稳定边界。

本 PRD 的第一阶段目标是“规范化并可编译”，不是替代旧串口页面。

## 非目标

1. 不删除旧 `src/serial/` 串口配置。
2. 不把 Serial Station 接入主导航。
3. 不实现完整 Modbus RTU。
4. 不依赖真实 COM 口测试。
5. 不在 UI 中直接操作 `QSerialPort`。
6. 不让多个子 Agent 同时修改 CMake 或共享接口。

## 范围

### 第一阶段：最小可构建骨架

新增或规范化以下文件：

- `src/apps/serial_station/SerialStationApp.h/.cpp`
- `src/apps/serial_station/SerialStationWindow.h/.cpp`
- `src/apps/serial_station/SerialStationController.h/.cpp`
- `src/apps/serial_station/SerialStationConfig.h/.cpp`
- `src/apps/serial_station/SerialStationModels.h`
- `src/apps/serial_station/SerialStationConstants.h`
- `src/apps/serial_station/protocols/SerialProtocolEvent.h`
- `src/apps/serial_station/protocols/ISerialProtocol.h`
- `src/apps/serial_station/protocols/SerialProtocolRegistry.h/.cpp`
- `src/apps/serial_station/protocols/ascii_text/AsciiTextProtocol.h/.cpp`
- `src/apps/serial_station/core/SerialSession.h/.cpp`
- `src/apps/serial_station/core/SerialPort.h/.cpp`
- `src/apps/serial_station/core/SerialManager.h/.cpp`

对应测试：

- `tests/serial_station/test_ascii_text_protocol.cpp`
- `tests/serial_station/test_serial_protocol_registry.cpp`
- `tests/serial_station/test_serial_session.cpp`

### 第二阶段：UART 配置 UI

后续新增：

- `src/apps/serial_station/ui/SerialPortPanel.h/.cpp`
- controller 与 UI signal/slot 连接
- 三套主题 QSS objectName 样式

第二阶段需要单独执行，不能和第一阶段混在同一轮里扩大风险。

## 架构边界

| 层 | 本 PRD 规则 |
|----|-------------|
| App/Window | 只做装配，不写串口业务 |
| Controller | 只协调 UI 意图、core、protocol registry |
| core | 只处理端口配置、会话状态、bytes 发送接收，不 include 具体协议目录 |
| protocols | 只构建命令、流式解析，不能 include QWidget |
| ui | 第一阶段不实现 |
| services | 第一阶段不实现 |
| workers | 第一阶段不实现 |

## 最小行为

1. `AsciiTextProtocol::buildCommand()` 接收 `text` 参数，按 UTF-8 输出，可选追加换行。
2. `AsciiTextProtocol::feed()` 把收到的 bytes 聚合成以 `\n` 分隔的 frame 事件。
3. `SerialProtocolRegistry` 支持注册、查询、默认协议和重复名称拒绝。
4. `SerialSession` 保存当前端口配置、打开状态和错误文本。
5. `SerialManager` 提供不依赖真实串口的状态转换接口；真实 `QSerialPort` 打开/关闭在后续阶段补强。

## CMake 规则

1. 所有新增 `.h/.cpp` 必须加入 `CMakeLists.txt` 或 `tests/CMakeLists.txt`。
2. 第一阶段可以只把 core/protocol/controller/app 骨架接入主目标，不接入主 UI。
3. 测试目标只链接需要的 Serial Station 源文件，避免拉入整个 GUI 主目标。

## 验收标准

1. 主目标 CMake 配置不报错。
2. `cmake --build build` 通过。
3. `cmake -S . -B build -DBUILD_TESTS=ON` 后，Serial Station 新增 QTest 可构建并通过。
4. `tools/source-tree-audit.ps1` 报告中 `src/apps/serial_station/` 的关键文件存在且进入 CMake。
5. 不新增第二构建目录。
6. 不回退用户已有改动。

## 风险

1. CMake 已经极大，新增文件必须集中且少量。
2. 旧小写骨架文件与新 PascalCase 文件可能共存，第一阶段不删除，先让规范骨架可构建。
3. 后续如果要删除旧小写文件，必须单独做技术债清理。
