# PRD-142 Serial Station UI Injection Result

## 目标

把 Serial Station 的 Fake RX 注入入口纳入统一 `OperationResult` 反馈模型，避免非 Fake transport 下失败被 UI 成功提示覆盖。

## 范围

- `SerialWorkbenchController.inject_received_text()` 返回结构化结果。
- 非 Fake transport 注入返回 `fake_injection_requires_fake_transport` 和可读 message。
- UI 注入动作拆入 `ui/injection_actions.py`，`MainWindow` 只保留编排入口。
- 失败时状态栏显示 `Inject failed: ...`，成功路径保持原行为。

## 非目标

- 不改变协议解析、日志格式、TCP transport 和 Profile 格式。
- 不新增脚本入口，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不提升真实设备验证状态。

## 验收

- 控制器在非 Fake transport 下返回失败结果并触发错误回调。
- UI 在 TCP 连接后执行 Fake RX 注入时显示可诊断失败信息。
- Fake loopback 下 RX 注入成功路径继续通过 UI smoke。
- `main_window.py` 与 controller 运行时文件继续小于等于 300 行。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，RX 注入失败路径有 controller 与 pytest-qt 覆盖。
- 用户状态：`U3`，错误反馈继续从连接/发送扩展到注入入口。
- 设备状态：`D2`，本轮不新增真实硬件证据。
