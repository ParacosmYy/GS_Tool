# PRD-165 Serial Station Plain Status Text Helper

## 1. 目标

将 UI action 层中散落的普通状态文本写入收敛到 `ui/status_messages.py`，让状态栏写入入口同时覆盖 OperationResult 状态与普通提示文本。

## 2. 范围

- 新增 `set_status_text()` 公共 UI helper。
- `connection_actions.py`、`session_actions.py`、`injection_actions.py`、`protocol_actions.py`、`status_actions.py` 复用公共 helper。
- 补充纯单测覆盖翻译与格式化参数。
- 保持当前用户可见文案与连接、发送、Profile、协议切换、错误展示行为不变。

## 3. 非目标

- 不改变 controller、service、driver、transport 或协议层行为。
- 不新增连接类型、协议解析或真实硬件验证能力。
- 不调整 `EmbedDebug.bat`、uv scripts 或 PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/status_messages.py` | 统一普通状态文本、OperationResult 文案和状态标签写入 |
| `ui/*_actions.py` | 只调用公共 helper 展示用户反馈 |
| `controllers/` | 保持业务结果语义，不参与 UI 文案格式化或控件写入 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 状态栏提示保持一致，反馈入口更可维护 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_status_messages.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py tests\python\ui_smoke\test_serial_station_workflow.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
