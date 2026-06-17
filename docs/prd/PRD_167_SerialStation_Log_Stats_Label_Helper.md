# PRD-167 Serial Station Log Stats Label Helper

## 1. 目标

将日志统计标签展示从 `log_actions.py` 中抽离到 `ui/status_messages.py`，让日志统计文案与状态栏、Profile 标签等展示入口保持一致。

## 2. 范围

- 新增 `set_log_stats_label()` 公共 UI helper。
- `log_actions.update_log_stats()` 继续只负责统计 total、TX、RX、visible 数量。
- 日志统计标签格式保持为 `Visible x / Total y | TX z | RX n`。
- 补充纯单测覆盖翻译与计数格式化。

## 3. 非目标

- 不改变日志存储、过滤、搜索、导出或回放行为。
- 不改变 controller、service、driver、transport 或协议层行为。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/log_actions.py` | 计算日志统计数字并委托展示 helper |
| `ui/status_messages.py` | 统一状态栏、Profile 标签和日志统计标签展示 |
| `controllers/` / `services/` | 保持日志数据和业务结果职责，不写 UI 文案 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 日志统计可见反馈保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_status_messages.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py::test_pyqt_mvp_log_stats_follow_entries_and_filters -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
