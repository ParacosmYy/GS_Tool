# PRD-128 Serial Station JustFloat 协议基础

## 背景

用户提供 EK-OmniProbe 作为参考，并要求结合 VOFA+ 精进串口工具。当前 Serial Station 已支持 `ascii_text`、`modbus_rtu` 和 `custom_md`，但缺少面向波形数据的低摩擦浮点流协议。VOFA+ 的 JustFloat 适合作为 Serial Station 后续图表工作区的数据入口基础。

## 目标

1. 在 `src/apps/serial_station/protocols/just_float/` 新增 JustFloat 协议实现。
2. 支持 VOFA+ JustFloat 帧格式：小端 IEEE754 float 数组 + 帧尾 `00 00 80 7F`。
3. 支持半包、粘包、多通道浮点解析和异常 payload 长度诊断。
4. 将 `just_float` 注册到 `SerialProtocolRegistry`，让工作台协议选择列表可见。
5. 补充 QTest、CMake、README 和评分追踪记录。

## 非目标

- 本轮不新增波形图 UI、FFT 面板或字段侧栏。
- 本轮不声明真实串口设备验证。
- 本轮不实现 VOFA+ FireWater 或 RawData。
- 本轮不修改 Serial Station 的 core 串口收发逻辑。

## 架构影响

| 项 | 结论 |
|----|------|
| 主模块 | `src/apps/serial_station/protocols/just_float/` |
| 对外接口 | 仅实现既有 `ISerialProtocol` |
| 依赖方向 | 具体协议只依赖 Qt Core 和 `protocols/ISerialProtocol` |
| 装配点 | `SerialProtocolRegistry::registerBuiltInProtocols()` |
| UI 影响 | 仅通过已有协议列表显示 `just_float`，UI 不 include 具体协议 |
| 设备验证 | D1 纯单测，不提升到 D2/D3/D4 |

## 三轴目标

| 轴 | 本轮目标 |
|----|----------|
| 工程状态 | E5，可维护收口：协议、registry、CMake、QTest、README 均闭环 |
| 用户状态 | U3，用户可在工作台选择协议并走既有发送/接收路径 |
| 设备验证 | D1，纯自动化测试；真实设备未验证 |

## 验收标准

1. `test_just_float_protocol` 覆盖命令构建、单通道、多通道、半包、粘包、异常 payload 和 reset。
2. `test_serial_protocol_registry` 验证内置协议包含 `just_float` 且可创建独立实例。
3. `test_serial_station_workbench` 验证协议组合框包含 `just_float`，选择后 controller 状态与日志同步。
4. 新增 `.h/.cpp` 已加入 `cmake/EmbedDebugSources.cmake` 和测试 CMake。
5. README 只按 D1 口径描述 JustFloat 基础解析，不夸大为完整波形工作区。
6. `EmbedDebug.bat --station serial` 启动探针通过或说明不能验证原因。
