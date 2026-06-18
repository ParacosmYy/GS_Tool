# PRD-207 Serial Station Endpoint Connection Action Boundary

## 目标

将 Serial Station UI 中的 TCP/UDP endpoint 连接动作从通用连接动作中拆出，让 `connection_actions` 聚焦 fake/serial/断开/端口刷新/连接控件状态，endpoint 连接独立维护。

## 范围

- 新增 `endpoint_connection_actions`，承接 TCP 与 UDP 连接、endpoint 校验和连接状态反馈。
- `main_window` 的 `_connect_tcp`、`_connect_udp` 统一委托到 endpoint connection actions。
- `connection_actions` 删除 TCP/UDP endpoint 连接职责。
- 更新 UI 架构测试，防止 endpoint 连接职责回流。

## 非目标

- 不改变 TCP/UDP 用户交互。
- 不改变 controller API 或 transport registry。
- 不新增设备能力。
- 不提升真实设备验证口径。

## 三轴状态

| 轴 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | UI 架构测试与 TCP/UDP smoke 覆盖 endpoint action 边界 |
| 用户 | `U3` | TCP/UDP 入口和状态反馈不变 |
| 设备 | `D2` | TCP/UDP 替身与 loopback 验证维持，真实设备未验证 |

## 验收

- `uv run pytest tests\python\unit\test_serial_station_ui_architecture.py -q`
- `uv run pytest tests\python\ui_smoke\test_serial_station_mvp_endpoints.py tests\python\ui_smoke\test_serial_station_udp_ui.py -q`
- `uv run test-embeddebug-py`
- `uv run test-embeddebug-tools`
- `uv run start-embeddebug --smoke`
- `cmd /c EmbedDebug.bat --smoke`

## 架构检查

- endpoint connection actions 不依赖 transport 具体实现。
- connection actions 不再直接处理 TCP/UDP endpoint 字段。
- main window 仅做委托，不直接访问 controller endpoint API。
- UI、controller、drivers、core、protocols、services 依赖方向未改变。
