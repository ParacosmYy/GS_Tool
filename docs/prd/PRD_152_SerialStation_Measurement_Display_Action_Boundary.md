# PRD-152 Serial Station Measurement Display Action Boundary

## 目标

将 Serial Station 测量数据展示动作归入 `ui/measurement_actions.py`，让 `MainWindow` 不直接操作波形预览控件，为后续多图表、仪表盘和统计视图扩展保留清晰边界。

## 范围

- 新增 `measurement_actions.append_measurement_batch()`，集中接收 controller 输出的 `ChannelBatch` 并更新展示控件。
- `MainWindow._append_measurement_batch()` 只保留委托。
- 新增 UI 架构测试，防止 `_waveform_preview.update_batch()` 逻辑回流到主窗口。
- 保持现有 FireWater/JustFloat 测量波形 smoke 路径不变。

## 非目标

- 不改变 `ChannelBatch`、ring buffer、协议解析和 controller 测量回调语义。
- 不新增波形功能、仪表盘功能或导出格式。
- 不新增 bat/cmd/ps1/sh 脚本，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不恢复 legacy native 源码、原生构建清单或原生打包链路。
- 不提升真实设备验证状态。

## 验收

- `MainWindow._append_measurement_batch()` 只调用 `measurement_actions.append_measurement_batch(self, batch)`。
- `measurement_actions.append_measurement_batch()` 是波形预览控件更新的唯一 UI action 入口。
- `tests/python/ui_smoke/test_serial_station_waveform_preview.py` 保持通过。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，测量展示动作边界有架构测试与 pytest-qt smoke 覆盖。
- 用户状态：`U3`，用户注入测量帧后仍可在波形预览看到通道更新。
- 设备状态：`D2`，本轮使用 Fake RX 和 UI smoke 验证，不新增真实硬件证据。
