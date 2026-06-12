# PRD-080 - Serial Station UART Config Panel

## 1. 背景

Serial Station 已有 `SerialPortConfig`、`SerialManager` 和 `SerialPort`，可以描述并应用 UART 参数，但 `src/apps/serial_station/ui/SerialPortPanel.*` 尚未落地，导致用户看不到独立串口工站里的端口、波特率、校验、停止位和流控配置入口。

## 2. 目标

1. 新增 `SerialPortPanel`，显示并收集最小 UART 参数。
2. UI 只通过 signal/slot 调用 `SerialStationController`，不直接调用 `SerialManager`。
3. `SerialStationController` 负责把 UI 意图转成 core 调用。
4. `SerialStationWindow` 只做装配和连接，不写业务逻辑。
5. 增加 QTest 覆盖默认配置和连接/断开信号。

## 3. 非目标

1. 不接入真实设备自动打开测试。
2. 不新增协议 UI、日志 UI 或命令发送 UI。
3. 不迁移旧 `src/serial/config/` 面板。
4. 不改变旧主线串口配置行为。

## 4. 改动范围

| 范围 | 路径 | 动作 |
|------|------|------|
| UI | `src/apps/serial_station/ui/SerialPortPanel.h/.cpp` | 新增 |
| 装配 | `src/apps/serial_station/SerialStationWindow.*` | 修改 |
| 控制 | `src/apps/serial_station/SerialStationController.*` | 修改 |
| 构建 | `CMakeLists.txt`, `tests/CMakeLists.txt` | 注册新增文件和测试 |
| 测试 | `tests/serial_station/test_serial_port_panel.cpp` | 新增 |

## 5. 验收标准

1. 面板包含端口、波特率、数据位、校验、停止位、流控、DTR、RTS、刷新、连接和断开入口。
2. 面板所有 QWidget 有稳定 `objectName`。
3. 用户可见文字使用 `tr()`。
4. UI 不 include `core/SerialManager.h` 或具体协议头。
5. 新增 `.h/.cpp` 已加入 CMake。
6. `test_serial_port_panel` 通过。
7. 既有 Serial Station QTest 继续通过。
8. `EmbedDebug.bat` 可启动。

## 6. 失败条件

- UI 直接调用 `SerialManager` 或 `QSerialPort::open()`。
- 新增源码未加入 CMake。
- 测试依赖真实 COM 口。
- 构建或启动验证失败。
