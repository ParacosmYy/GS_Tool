# PRD-161 Serial Station Status Message Helper

## 1. 目标

将 Serial Station UI action 中的 OperationResult 成功/失败状态文案收敛为公共 helper，减少连接、发送等动作里的重复消息拼接，为后续更多连接类型和动作入口保持一致反馈打基础。

## 2. 范围

- 新增 `ui/status_messages.py` 纯 UI 消息 helper。
- `connection_actions.py` 复用 helper 展示连接和发送结果。
- 新增单测覆盖成功消息、失败消息和翻译后格式化。
- 保持现有用户可见文案不变。

## 3. 非目标

- 不新增连接类型。
- 不改变 controller、service、driver 或协议层行为。
- 不声明真实硬件状态提升。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/status_messages.py` | UI 层状态消息格式化，不读取控件状态 |
| `ui/connection_actions.py` | 调用 controller 并展示 helper 返回的状态消息 |
| `controllers/` | 保持 OperationResult 语义，不参与 UI 文案拼接 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 现有连接/发送反馈保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_status_messages.py tests\python\ui_smoke\test_serial_station_mvp.py::test_pyqt_mvp_send_failure_shows_result_message tests\python\ui_smoke\test_serial_station_udp_ui.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
