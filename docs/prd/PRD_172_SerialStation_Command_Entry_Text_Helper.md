# PRD-172 Serial Station Command Entry Text Helper

## 1. 目标

将命令历史选择后写入发送输入框的规则从 `connection_actions.py` 中抽离到独立 UI helper，避免动作层直接维护输入框写入细节。

## 2. 范围

- 新增 `ui/command_entry_text.py`。
- 提供 `apply_command_history_selection()`。
- `connection_actions.select_command_history()` 复用 helper。
- 补充纯单测覆盖非空历史写入与空历史不覆盖当前输入两条路径。

## 3. 非目标

- 不改变命令历史来源、保存策略或发送流程。
- 不改变 controller、transport、protocol 或 service 行为。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/command_entry_text.py` | 统一命令输入框文本写入规则 |
| `ui/connection_actions.py` | 响应命令历史选择事件并委托 helper |
| `controllers/` | 提供命令历史数据，不直接操作输入控件 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 命令历史选择写入发送框行为保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_command_entry_text.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
