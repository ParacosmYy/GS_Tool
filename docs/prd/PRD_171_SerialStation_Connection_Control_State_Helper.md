# PRD-171 Serial Station Connection Control State Helper

## 1. 目标

将连接、串口连接、TCP/UDP 连接与断开按钮的启用状态从 `connection_actions.py` 中抽离到独立 UI helper，避免动作层继续直接维护按钮矩阵。

## 2. 范围

- 新增 `ui/connection_control_state.py`。
- 提供 `set_connection_control_state()`。
- `connection_actions.set_connected_controls()` 复用 helper。
- 补充纯单测覆盖未连接、已连接、无串口三条路径。

## 3. 非目标

- 不改变连接、断开、发送、端口刷新或 controller 行为。
- 不新增真实硬件依赖。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/connection_control_state.py` | 统一连接相关按钮的启用状态同步 |
| `ui/connection_actions.py` | 编排连接状态变化并委托 UI 状态 helper |
| `controllers/` | 提供连接状态，不直接操作 UI 控件 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 连接按钮可见行为保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_connection_control_state.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
