# PRD-141 Serial Station UI Connection Result

## 目标

把 Serial Station UI 的连接和发送入口切换到 controller 的 `OperationResult` API，让用户看到可诊断的失败原因，而不是泛化的失败提示。

## 范围

- Fake 连接调用 `connect_fake_result()`，失败时展示 result message。
- Serial 连接调用 `connect_serial_result()`，失败时展示 result message。
- TCP 连接调用 `connect_tcp_result()`，失败时展示 result message。
- 发送调用 `send_text_result()`，未连接、写入不完整等失败显示具体原因。
- 将连接和发送动作拆入 `ui/connection_actions.py`，继续压低 `main_window.py` 职责。

## 非目标

- 不改变 transport、协议和日志格式。
- 不新增脚本入口，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不提升真实设备验证状态。
- 不删除 controller 旧 bool 兼容 API。

## 验收

- 未连接发送显示 `Send failed: Open a transport before sending`。
- Fake 连接失败显示 controller result message。
- TCP 连接失败显示 `Failed to open tcp transport`。
- 既有连接、发送、历史、TCP profile 成功路径继续通过 UI smoke。
- `main_window.py` 小于等于 300 行。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，连接和发送 UI result 路径有 pytest-qt 覆盖。
- 用户状态：`U3`，主流程失败原因可见，可用于排障。
- 设备状态：`D1`，本轮只涉及 UI 错误语义治理。
