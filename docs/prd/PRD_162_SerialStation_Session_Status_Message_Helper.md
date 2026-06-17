# PRD-162 Serial Station Session Status Message Helper

## 1. 目标

将日志导出、日志回放、Profile 保存和 Profile 加载动作中的 OperationResult 状态文案收敛到 `ui/status_messages.py`，让 session 类 UI action 与 connection 类 UI action 使用同一套成功/失败反馈格式。

## 2. 范围

- `session_actions.py` 复用 `translated_result_message`。
- 新增 UI 架构测试约束 session action 不再局部拼接 `failed: {message}`。
- 保持日志导出、回放、Profile 保存和加载的用户可见文案不变。

## 3. 非目标

- 不改变 `controllers/`、`services/`、`drivers/` 或协议层行为。
- 不新增文件选择、导出格式、Profile 字段或真实设备能力。
- 不调整 `EmbedDebug.bat` 或 uv script。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/status_messages.py` | 统一 OperationResult 状态文案格式 |
| `ui/session_actions.py` | 调用 controller/service 结果并展示统一状态文案 |
| `services/` | 保持文件读写、导出、回放和 Profile 持久化职责 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 架构测试、workflow smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 导出/回放/Profile 状态反馈保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_serial_station_ui_architecture.py::test_session_result_messages_reuse_status_message_helper tests\python\ui_smoke\test_serial_station_workflow.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
