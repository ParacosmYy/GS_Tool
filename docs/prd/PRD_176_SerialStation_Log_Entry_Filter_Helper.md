# PRD-176 Serial Station Log Entry Filter Helper

## 1. 目标

将日志方向过滤与搜索匹配规则从 `log_actions.py` 中抽离到独立 UI helper，避免日志 action 同时承担控件读取和过滤算法。

## 2. 范围

- 新增 `ui/log_entry_filter.py`。
- 提供 `log_entry_matches_filter()`。
- `log_actions.log_entry_visible()` 读取 UI 状态后委托 helper。
- 补充纯单测覆盖全部方向、TX/RX 方向过滤和搜索匹配路径。

## 3. 非目标

- 不改变日志数据结构、导出、回放、统计或视图渲染行为。
- 不改变 controller、transport、protocol 或 service 行为。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/log_entry_filter.py` | 统一日志方向与搜索过滤规则 |
| `ui/log_actions.py` | 读取日志过滤控件状态并委托 helper |
| `controllers/` / `services/` | 提供日志数据，不直接操作 QWidget |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 日志方向过滤和搜索行为保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_log_entry_filter.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
