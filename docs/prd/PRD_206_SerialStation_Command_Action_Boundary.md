# PRD-206 Serial Station Command Action Boundary

## 目标

将 Serial Station UI 中的命令发送和命令历史动作从连接动作中拆出，避免 `connection_actions` 同时承担连接生命周期与命令输入职责，让 UI action 层按业务域继续收敛。

## 范围

- 新增 `command_actions`，承接发送命令、刷新命令历史、选择历史命令。
- `main_window` 的 `_send_text`、`_refresh_command_history`、`_select_command_history` 统一委托到 command actions。
- `connection_actions` 回归连接、端口刷新和连接控件状态职责。
- 更新 UI 架构测试，防止命令职责回流到连接动作。
- 补齐 `SafePlotWidget.resizeEvent` offscreen 生命周期保护，修复全量 smoke 中 pyqtgraph 延迟 resize 事件触发的对象销毁崩溃。

## 非目标

- 不改变发送命令、命令历史或快捷键用户交互。
- 不改变 controller API。
- 不新增协议、transport 或设备能力。
- 不提升真实设备验证口径。

## 三轴状态

| 轴 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | UI 架构测试与 Serial Station smoke 覆盖命令动作边界 |
| 用户 | `U3` | 用户入口和发送命令主流程不变 |
| 设备 | `D2` | 替身与 loopback 验证维持，真实设备未验证 |

## 验收

- `uv run pytest tests\python\unit\test_serial_station_ui_architecture.py -q`
- `uv run pytest tests\python\unit\test_waveform_preview_lifecycle.py -q`
- `uv run pytest tests\python\ui_smoke\test_serial_station_mvp.py tests\python\ui_smoke\test_serial_station_workflow.py -q`
- `uv run pytest tests\python\ui_smoke\test_serial_station_waveform_preview.py -q`
- `uv run test-embeddebug-py`
- `uv run test-embeddebug-tools`
- `uv run start-embeddebug --smoke`
- `cmd /c EmbedDebug.bat --smoke`

## 架构检查

- 命令动作不再放在 connection actions。
- main window 仅做委托，不直接访问 controller 发送结果或命令历史控件细节。
- command actions 不依赖 transport、protocol 或 service 内部实现。
- waveform preview 的 offscreen resize 防护不改变正常平台渲染路径。
- UI、controller、core、protocols、services 依赖方向未改变。
