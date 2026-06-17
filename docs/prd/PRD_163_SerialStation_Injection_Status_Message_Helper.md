# PRD-163 Serial Station Injection Status Message Helper

## 1. 目标

将 Fake RX 注入动作中的 OperationResult 成功/失败状态文案收敛到 `ui/status_messages.py`，让连接、发送、会话文件动作和注入动作使用一致的 UI 状态消息格式。

## 2. 范围

- `injection_actions.py` 复用 `translated_result_message`。
- 新增 UI 架构测试约束 injection action 不再局部拼接 `Inject failed: {message}`。
- 保持 RX 注入成功和失败的用户可见文案不变。

## 3. 非目标

- 不改变 fake transport、controller 注入语义或协议解析。
- 不新增真实串口、TCP 或 UDP 注入能力。
- 不调整启动脚本、uv script 或打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/status_messages.py` | 统一 OperationResult 状态文案格式 |
| `ui/injection_actions.py` | 调用 controller 注入接口并展示统一状态文案 |
| `controllers/` | 保持注入结果语义，不参与 UI 文案拼接 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | RX 注入成功/失败反馈保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_serial_station_ui_architecture.py::test_injection_result_messages_reuse_status_message_helper tests\python\ui_smoke\test_serial_station_mvp.py::test_pyqt_mvp_inject_failure_shows_result_message -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
