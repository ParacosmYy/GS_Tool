# PRD-170 Serial Station Command History Options Helper

## 1. 目标

将命令历史 Combo 的信号阻断、选项刷新、最后一条选中和启用状态控制从 `connection_actions.py` 中抽离到独立 UI helper，避免连接动作层继续维护控件刷新细节。

## 2. 范围

- 新增 `ui/command_history_options.py`。
- 提供 `populate_command_history_options()`。
- `connection_actions.refresh_command_history()` 复用 helper。
- 补充纯单测覆盖有历史与空历史两条路径。

## 3. 非目标

- 不改变命令历史的来源、保存策略或 controller 行为。
- 不改变发送、连接、协议、driver 或 transport 行为。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/command_history_options.py` | 统一命令历史 Combo 的刷新、选中和启用状态 |
| `ui/connection_actions.py` | 编排发送成功后的历史刷新动作 |
| `controllers/` | 提供命令历史数据，不操作 UI 控件 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 命令历史可见行为保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_command_history_options.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
