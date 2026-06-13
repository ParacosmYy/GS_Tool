# PRD-129 Serial Station 测量通道面板

## 背景

PRD-128 已把 VOFA+ JustFloat 基础解析接入 Serial Station，但测量值目前只进入 RX 文本日志。对嵌入式调试工站来说，日志适合追溯，通道状态适合现场观察。参考 VOFA+ 和 EK-OmniProbe 的数据观察路径，本轮补一个轻量通道摘要面板，先让用户能看到最新值、样本数、最小值和最大值，为后续波形工作区打基础。

## 目标

1. 新增 `SerialMeasurementService`，把 `measurement` 事件汇总为通道快照。
2. 新增 `SerialMeasurementPanel`，只显示 controller 提供的摘要文本，不解析协议 bytes。
3. `SerialStationController` 收到 measurement 事件时更新 service，并向 UI 发出通道摘要。
4. `SerialStationWindow` 在左侧协议面板下方展示测量通道面板。
5. 清空日志时同步清空测量摘要，避免旧数据误导当前会话。

## 非目标

- 本轮不接入 Qt Charts，不画波形。
- 本轮不做通道重命名、颜色配置、单位配置。
- 本轮不做真实串口设备验证。
- 本轮不让 UI include 具体协议实现。

## 架构影响

| 项 | 结论 |
|----|------|
| 主模块 | `src/apps/serial_station/services/`、`ui/`、controller 装配 |
| 新增类 | `SerialMeasurementService`、`SerialMeasurementPanel` |
| 数据来源 | `SerialProtocolEvent` 的 `measurement` payload |
| UI 边界 | 面板只接收 `QStringList` 摘要，不解析 bytes |
| 复用 | 复用现有 `ISerialProtocol` 事件、controller 信号、QTest、Qt Widgets |
| 设备验证 | D1，纯自动化测试 |

## 三轴目标

| 轴 | 本轮目标 |
|----|----------|
| 工程状态 | E5，可维护收口：service、controller、UI、CMake、QTest、README 闭环 |
| 用户状态 | U3，不提升：用户可在工站看到测量摘要，但未完成波形/硬件验证 |
| 设备验证 | D1，不提升：仅自动化测试，真实设备未验证 |

## 验收标准

1. `test_serial_measurement_service` 覆盖空状态、单帧、多帧、多通道、min/max、reset 和非法事件忽略。
2. `test_serial_measurement_panel` 覆盖空态、摘要显示、清空。
3. `test_serial_station_controller` 验证 JustFloat measurement 会发出通道摘要。
4. `test_serial_station_workbench` 验证工作台存在测量面板，选择 `just_float` 后喂入样本能显示通道值。
5. 清空日志后测量面板回到空态。
6. README 使用 E5/U3/D1 口径说明测量通道摘要，不宣传完整波形能力。
