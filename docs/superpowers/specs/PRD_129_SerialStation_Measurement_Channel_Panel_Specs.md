# PRD-129 Specs - Serial Station 测量通道面板

## 执行边界

- 主线：PRD 线。
- 触碰层：`services`、`controller`、`ui`、app 装配。
- 允许修改：CMake、README、评分追踪、Serial Station 测试。
- 不触碰：`core/SerialManager`、`workers/`、具体协议 parser、Qt Charts。

## 数据模型

`SerialMeasurementService` 维护一个会话内快照：

- `protocolName`
- `frameCount`
- `channels`
  - `index`
  - `name`，默认 `ch1`、`ch2`
  - `latest`
  - `minimum`
  - `maximum`
  - `sampleCount`

处理规则：

- 只接受 `event.type == "measurement"` 且 `payload["values"]` 非空的事件。
- 每个 measurement 事件算一个 frame。
- 第 N 个 values 更新第 N 个通道。
- 新通道按出现顺序创建。
- `reset()` 清空所有统计。
- `displayLines()` 返回适合 UI 展示的摘要，每行格式包含通道名、最新值、样本数、最小值和最大值。

## Controller 行为

- measurement 事件仍写 RX 日志并计数。
- measurement 事件额外更新 `SerialMeasurementService`。
- 更新成功后 emit `serialMeasurementUpdated(QStringList lines)`。
- `clearLogRecords()` 同步 reset measurement service，并 emit 空列表。

## UI 行为

- 新增 `SerialMeasurementPanel`。
- objectName：
  - `serialMeasurementPanel`
  - `serialMeasurementTitle`
  - `serialMeasurementSummaryLabel`
  - `serialMeasurementView`
- 初始显示“暂无测量数据”。
- 收到摘要时逐行显示。
- 收到空摘要或 clear 时恢复空态。

## 工作台装配

- `SerialMeasurementPanel` 放在左侧 `SerialProtocolPanel` 下方。
- Window 只连接 controller 信号到 panel，不解析 payload。
- 日志清空时通过 controller 清 service，panel 从空摘要信号恢复空态。

## 验证命令

```powershell
cmake --build build --target test_serial_measurement_service test_serial_measurement_panel test_serial_station_controller test_serial_station_workbench EmbedDebug --parallel 4
.\build\tests\test_serial_measurement_service.exe
.\build\tests\test_serial_measurement_panel.exe
.\build\tests\test_serial_station_controller.exe
.\build\tests\test_serial_station_workbench.exe
.\EmbedDebug.bat --station serial
git diff --check
```

## 失败条件

- UI include 具体协议目录。
- service 操作 QWidget。
- controller 把 measurement 解析逻辑写成协议字节解析。
- README 把摘要面板描述成完整波形引擎或真实硬件验证能力。
