# PRD-089 Serial Station Modbus RTU 协议

## 1. 背景

Serial Station 当前已经具备串口配置、发送编码、接收 dispatcher 和 ASCII 文本协议，但协议目录仍只有
`ascii_text`。`docs/serial_station_architecture.md` 明确串口工站第一阶段协议应包含 `ascii_text`、
`modbus_rtu` 和 `custom_md`。如果继续只优化构建系统，串口上位机本体无法形成面向工业调试的协议能力。

本轮补齐 `modbus_rtu` 协议层，让 registry 能创建真实 Modbus RTU 协议，支持基础主站请求构建和接收帧解析。

## 2. 目标

- 新增 `protocols/modbus_rtu` 协议实现，并通过 `SerialProtocolRegistry` 注册为 `modbus_rtu`。
- 支持常用主站请求构建：读线圈、读离散输入、读保持寄存器、读输入寄存器、写单线圈、写单寄存器。
- 使用 CRC16-Modbus 校验，帧尾按 RTU 低字节在前、高字节在后编码。
- 支持接收侧流式解析：半包暂存、粘包连续输出、异常响应识别、CRC 错误上报。
- 增加 QTest 覆盖请求构建、参数校验、响应解析、半包粘包、异常响应和 registry 注册。

## 3. 非目标

- 本轮不新增 Modbus UI 参数面板。
- 本轮不直接打开真实串口硬件。
- 本轮不实现 Modbus 从站寄存器表。
- 本轮不新增 ASCII/custom 协议能力。
- 本轮不修改旧 `src/serial/`。

## 4. 范围

| 类型 | 路径 | 动作 |
|------|------|------|
| 主要 | `src/apps/serial_station/protocols/modbus_rtu/ModbusRtuProtocol.h/.cpp` | 新增 |
| 主要 | `src/apps/serial_station/protocols/SerialProtocolRegistry.cpp` | 注册协议 |
| 主要 | `src/apps/serial_station/SerialStationConstants.h` | 增加协议事件类型常量 |
| 测试 | `tests/serial_station/test_modbus_rtu_protocol.cpp` | 新增 |
| 测试 | `tests/serial_station/test_serial_protocol_registry.cpp` | 补注册断言 |
| 构建 | `CMakeLists.txt`, `tests/CMakeLists.txt` | 注册新增源码和测试 |

## 5. 验收标准

- Registry `contains("modbus_rtu")` 为真，且能创建协议实例。
- `read_holding_registers` 参数 `slaveId=1,startAddress=0,quantity=2` 构建出合法 FC03 RTU 请求。
- `write_single_register` 参数 `slaveId=1,address=1,value=0x1234` 构建出合法 FC06 RTU 请求。
- 无效从站地址、地址、数量、值和未知命令返回明确错误，不构建空成功帧。
- 半包输入不产生事件；补齐后产生一条 Modbus 帧事件。
- 粘包输入能连续产生多条事件。
- CRC 错误产生错误事件，不把坏帧当成有效业务帧。
- 相关 QTest、`EmbedDebug` 构建、doctor 和 `EmbedDebug.bat` 启动验证通过。

## 6. 失败条件

- `protocols/modbus_rtu` include UI、QWidget、Controller 或 Window。
- `core/` include 具体 Modbus 目录。
- 重复新造与项目已有 CRC 公共组件冲突的公共轮子。
- 新增 `.h/.cpp` 未注册 CMake。
- 为了满足提交行数加入无意义代码。
