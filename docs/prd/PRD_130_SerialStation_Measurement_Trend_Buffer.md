# PRD-130 Serial Station 测量趋势缓冲

## 目标

将当前 JustFloat 测量摘要从“只看最新统计”推进到“可观察最近测量帧”。本轮为后续 VOFA+/OmniProbe 式波形工作区建立稳定数据缓冲和 UI 可见入口。

## 用户问题

用户接入连续 float 数据时，只能看到每个通道的 latest/min/max，无法判断最近几帧是否在变化，也无法把测量帧快速复制为表格数据。

## 范围

- `SerialMeasurementService` 保存最近测量帧，默认容量受控。
- service 提供趋势文本行、CSV 文本和测量 CSV 文件导出服务。
- `SerialMeasurementPanel` 增加最近帧显示区。
- `SerialStationController` 在 measurement 事件到达后同步发出摘要与趋势。
- 工作台测试覆盖 JustFloat 数据进入趋势区和清空行为。

## 非目标

- 不实现完整实时波形画布。
- 不在本轮把测量导出按钮接入 UI 对话框。
- 不接入真实串口、虚拟串口或硬件回环验证。
- 不修改 JustFloat 协议帧格式。

## 分层与状态

- 改动层：`services`、`controller`、`ui`、`tests`。
- 工程目标：`E5`，服务、控制器和工作台测试通过。
- 用户目标：保持 `U3`，工作台可见最近帧但尚非完整波形工作流。
- 设备目标：保持 `D1`，纯自动化测试验证，真实设备未验证。

## 验收标准

- 连续追加 measurement 事件后，service 可返回最近帧趋势行。
- 超过容量时，仅保留最近 N 帧。
- CSV 输出包含 `frame,ch1,ch2...` 表头和对应值。
- CSV 文件导出使用 service 层原子写入，空路径、空数据和缺失目录有失败结果。
- 面板空状态、趋势显示、清空行为可测试。
- JustFloat 数据进入工作台后，最近帧区域可见。
- 清空日志同步清空测量摘要和趋势。
