# PRD-088 Serial Station 发送编码器

## 1. 背景

Serial Station 当前发送闭环已经能处理 ASCII 与协议命令，但发送模式校验和字节构建仍集中在
`SerialStationController`。命令面板中已经给出 `01 03 00 00 00 02` 这类 HEX 输入提示，
但 Controller 会拒绝 `hex` 模式，导致用户可见提示和实际能力不一致。

`docs/serial_station_architecture.md` 已要求 `core/SerialCodec.h/.cpp` 作为核心层能力落地。
本轮将补齐该核心组件，让 Controller 只负责协调，不继续堆编码细节。

## 2. 目标

- 新增 `core/SerialCodec`，统一处理发送模式归一化、ASCII 编码、HEX 编码和协议委托。
- HEX 模式支持空格、逗号、换行、`0x` 前缀输入，解析失败时给出明确错误。
- Controller 使用 `SerialCodec` 构建发送帧，不在自身内部维护具体模式分支。
- 命令面板暴露 HEX 模式选项，使 UI 提示和实际发送能力一致。
- 增加 QTest 覆盖 ASCII、HEX、协议、未知模式、空输入和错误路径。

## 3. 非目标

- 本轮不新增真实串口硬件依赖。
- 本轮不新增 Modbus/custom 协议。
- 本轮不改旧 `src/serial/`。
- 本轮不改 QSS 主题和布局结构。
- 本轮不改全局 `HexConverter` 行为，只在 Serial Station 内部做发送输入诊断。

## 4. 范围

| 类型 | 路径 | 动作 |
|------|------|------|
| 主要 | `src/apps/serial_station/core/SerialCodec.h/.cpp` | 新增 |
| 主要 | `src/apps/serial_station/SerialStationController.*` | 修改 |
| UI | `src/apps/serial_station/ui/SerialCommandPanel.cpp` | 增加 HEX 模式选项 |
| 测试 | `tests/serial_station/test_serial_codec.cpp` | 新增 |
| 测试 | `tests/serial_station/test_serial_station_controller.cpp` | 修改 |
| 测试 | `tests/serial_station/test_serial_station_workbench.cpp` | 必要回归 |
| 构建 | `CMakeLists.txt`, `tests/CMakeLists.txt` | 注册新增源码和测试 |

## 5. 验收标准

- `SerialCodec` 能把 ASCII 文本转成原始 UTF-8 bytes。
- `SerialCodec` 能把 `01 03 00 00 00 02`、`0x01,0x03`、换行分隔 HEX 转成 bytes。
- HEX 输入存在奇数字符、非法字符或空输入时，返回明确错误。
- 协议模式仍委托当前默认协议构建命令。
- Controller 支持 `hex` 模式发送准备信号，不再将其判定为不支持。
- 命令面板模式下拉包含 `ASCII`、`HEX`、`协议命令`。
- 相关 QTest、`EmbedDebug` 构建、doctor 和 `EmbedDebug.bat` 启动验证通过。

## 6. 失败条件

- UI 直接拼接 QByteArray 或直接调用 `SerialManager`。
- `core/` include 具体协议目录。
- 新增 HEX 解析重复实现全局公共组件已有职责。
- 新增 `.h/.cpp` 未注册 CMake。
- 为了通过提交行数加入无业务意义代码。
