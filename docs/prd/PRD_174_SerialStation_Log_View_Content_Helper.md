# PRD-174 Serial Station Log View Content Helper

## 1. 目标

将日志视图清空规则从 `log_actions.py` 与 `session_actions.py` 中抽离到独立 UI helper，避免多个 action 模块直接操作日志控件内容。

## 2. 范围

- 新增 `ui/log_view_content.py`。
- 提供 `clear_log_view()`。
- `log_actions.render_log_entries()`、`session_actions.clear_log()`、`session_actions.replay_log()` 复用 helper。
- 补充纯单测覆盖日志视图清空行为。

## 3. 非目标

- 不改变日志服务、导出、回放、过滤或统计逻辑。
- 不改变 controller、transport、protocol 或 service 行为。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/log_view_content.py` | 统一日志视图内容清空规则 |
| `ui/log_actions.py` | 渲染日志条目并委托视图内容 helper |
| `ui/session_actions.py` | 响应清空和回放动作并委托视图内容 helper |
| `controllers/` / `services/` | 管理日志数据与回放结果，不直接操作 QWidget |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 清空日志、重绘日志与回放日志可见行为保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_log_view_content.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
