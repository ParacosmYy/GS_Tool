# PRD-164 Serial Station Status Label Helper

## 1. 目标

将 connection、session 和 injection action 中重复的 `_set_result_status` 私有包装上移到 `ui/status_messages.py`，形成统一的状态标签写入 helper，避免每个 action 模块重复实现 OperationResult 到状态栏的转换。

## 2. 范围

- 新增 `set_result_status()` 公共 UI helper。
- `connection_actions.py`、`session_actions.py`、`injection_actions.py` 直接复用公共 helper。
- 新增/更新单测与架构测试，约束 action 模块不再保留私有 `_set_result_status`。
- 保持连接、发送、导出、回放、Profile 和 RX 注入的用户可见文案不变。

## 3. 非目标

- 不改变 controller、service、driver、transport 或协议层行为。
- 不新增连接类型、协议或硬件验证能力。
- 不调整启动脚本、uv script 或 PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/status_messages.py` | 统一 OperationResult 状态消息和状态标签写入 |
| `ui/*_actions.py` | 只调用 helper 展示 controller 结果 |
| `controllers/` | 保持业务结果语义，不参与 UI 文案或控件写入 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 现有主流程反馈保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_status_messages.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_workflow.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
