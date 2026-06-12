# PRD-090 Serial Station Custom MD 协议

## 1. 背景

Serial Station 已经具备 ASCII 文本协议、Modbus RTU 协议、发送编码器和接收 dispatcher。
`docs/serial_station_architecture.md` 明确第一批协议只保留 `ascii_text`、`modbus_rtu`、`custom_md`。
当前缺口是 `custom_md` 尚未落地，导致上位机还不能覆盖常见“自定义 MCU 调试帧”的场景。

本轮补齐 `custom_md` 协议层，只做协议本体和测试，不做 UI 参数面板。

## 2. 目标

- 新增 `protocols/custom_md` 协议实现，并通过 `SerialProtocolRegistry` 注册为 `custom_md`。
- 定义默认自定义帧格式：`A5 5A | len | cmd | payload | checksum | 0D 0A`。
- `len` 表示 `cmd + payload` 的字节数，默认最大 payload 为 240 字节。
- `checksum` 默认使用低 8 位加和，覆盖 `len/cmd/payload`，便于 MCU 端快速实现。
- `buildCommand()` 支持通过 `command` 构建命令码，并从 `params["payload"]` 读取 `QByteArray` 或 HEX 字符串。
- `feed()` 支持半包缓存、粘包连续解析、坏 checksum 上报、坏 header/footer 重同步。
- 增加 QTest 覆盖构帧、解析、半包、粘包、checksum 错误、参数校验和 registry 注册。

## 3. 非目标

- 本轮不新增 UI 协议配置面板。
- 本轮不实现 JSON/DSL 协议编辑器。
- 本轮不修改旧 `src/serial/` 或旧 `src/protocol/`。
- 本轮不修改 `core/SerialDispatcher` 和 `core/SerialCodec` 的职责。
- 本轮不引入真实串口硬件依赖。

## 4. 范围

| 类型 | 路径 | 动作 |
|------|------|------|
| 主要 | `src/apps/serial_station/protocols/custom_md/CustomMdProtocol.h/.cpp` | 新增 |
| 主要 | `src/apps/serial_station/protocols/SerialProtocolRegistry.cpp` | 注册协议 |
| 主要 | `src/apps/serial_station/SerialStationConstants.h` | 增加事件类型常量 |
| 测试 | `tests/serial_station/test_custom_md_protocol.cpp` | 新增 |
| 测试 | `tests/serial_station/test_serial_protocol_registry.cpp` | 补注册断言 |
| 构建 | `CMakeLists.txt`, `tests/CMakeLists.txt` | 注册新增源码和测试 |

## 5. 验收标准

- Registry `contains("custom_md")` 为真，且能创建协议实例。
- 默认参数下，命令 `0x10` 和 payload `01 02` 能构建出 `A5 5A 03 10 01 02 16 0D 0A`。
- `payload` 支持 `QByteArray` 和 HEX 字符串输入，非法 HEX 返回明确错误。
- 半包输入不产生事件；补齐后产生一条 `custom_md_frame` 事件。
- 粘包输入能连续产生多条 `custom_md_frame` 事件。
- checksum 错误产生 `custom_md_error`，不能当成正常帧。
- 噪声前缀不会卡死解析缓存，协议能重同步到下一条合法帧。
- 相关 QTest、`EmbedDebug` 构建、doctor 和 `EmbedDebug.bat` 启动验证通过。

## 6. 失败条件

- `protocols/custom_md` include UI、QWidget、Controller 或 Window。
- `core/` include `protocols/custom_md`。
- 新协议绕过 `ISerialProtocol` 或 registry。
- 新增 `.h/.cpp` 未注册 CMake。
- 为了满足提交行数加入无意义代码。
