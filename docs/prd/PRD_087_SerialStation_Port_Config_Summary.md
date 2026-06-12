# PRD-087 Serial Station UART 配置校验与摘要

## 1. 背景

Serial Station 已具备 UART 配置面板、连接控制、发送闭环和接收闭环，但 `SerialPortConfig`
目前只有 `isValid()`，无法给出标准化错误原因、稳定摘要或连接日志上下文。后续自动重连、设备档案、
日志导出和错误诊断都需要复用同一套配置描述，不能把 UART 参数文案散落在 UI 或 Controller 中。

## 2. 目标

- 为 `SerialPortConfig` 增加可复用的标准化能力：端口名裁剪、配置有效性、错误原因、参数摘要。
- Controller 连接前使用配置错误原因，避免只把失败推给底层 `QSerialPort`。
- Controller 连接时输出带 UART 参数摘要的系统日志，方便用户确认实际打开的端口参数。
- 增加 QTest 覆盖常用 UART 参数组合、非法配置、连接日志和错误路径。

## 3. 非目标

- 本轮不新增真实串口扫描策略。
- 本轮不做配置持久化。
- 本轮不新增 Modbus/custom 协议。
- 本轮不改旧 `src/serial/` 模块。
- 本轮不修改 UI 布局和 QSS。

## 4. 范围

| 类型 | 路径 | 动作 |
|------|------|------|
| 主要 | `src/apps/serial_station/SerialStationConfig.h/.cpp` | 增加配置校验、标准化、摘要方法 |
| 主要 | `src/apps/serial_station/SerialStationController.cpp` | 使用配置错误原因和摘要日志 |
| 测试 | `tests/serial_station/` | 增加配置和控制器回归 |
| 构建 | `tests/CMakeLists.txt` | 如新增测试目标则注册 |

## 5. 验收标准

- `SerialPortConfig` 能输出稳定的 `COMx 115200 8N1 无流控 DTR=off RTS=off` 类摘要。
- 空端口、非正波特率、非法数据位、非法停止位、未知校验或流控能给出明确错误原因。
- `connectSerialPort()` 在非法配置时不依赖底层串口打开失败文案，直接上报配置错误。
- `connectSerialPort()` 在合法配置尝试打开前输出一次包含配置摘要的系统日志。
- QTest 覆盖配置摘要、错误原因、Controller 合法尝试日志、非法配置错误。
- `EmbedDebug` 构建通过，相关 QTest 通过，`tools/doctor.ps1` 通过，`EmbedDebug.bat` 可启动。

## 6. 失败条件

- UI 面板直接调用 `SerialManager` 或协议对象。
- `core/` include 具体协议目录。
- UART 配置文案在多个类中重复拼接。
- 新增源码未加入 CMake。
- 为了凑行数加入无业务意义的代码。
