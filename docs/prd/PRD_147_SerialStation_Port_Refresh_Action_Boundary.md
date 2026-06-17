# PRD-147 Serial Station Port Refresh Action Boundary

## 目标

将 Serial Station 串口端口刷新入口归入 `ui/connection_actions.py`，让 `MainWindow` 继续只承担用户事件转发和界面编排。

## 范围

- 新增 `connection_actions.populate_serial_port_combo()`，集中处理端口枚举与端口下拉框填充。
- 新增 `connection_actions.refresh_serial_ports()`，集中处理刷新反馈和连接控件状态同步。
- `MainWindow._refresh_serial_ports()` 只委托 connection action。
- `sections.py` 初始化端口列表时复用 connection action，不再依赖主窗口私有填充逻辑。
- 新增 UI 架构测试，防止端口刷新逻辑回流到主窗口。

## 非目标

- 不改变 UART transport、端口枚举来源、连接错误码和 Profile 格式。
- 不新增 bat/cmd/ps1/sh 脚本，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不恢复 legacy native 源码、构建清单或打包链路。
- 不提升真实设备验证状态。

## 验收

- `MainWindow._refresh_serial_ports()` 只调用 `connection_actions.refresh_serial_ports(self)`。
- 首次打开窗口仍能填充当前串口列表；无串口时显示 `No serial ports`。
- 点击刷新按钮或使用 `Ctrl+R` 后状态栏仍显示 `Serial ports refreshed`。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，端口刷新动作边界有架构测试与 pytest-qt 覆盖。
- 用户状态：`U3`，串口端口刷新用户路径保持稳定。
- 设备状态：`D2`，本轮使用替身枚举与 UI smoke 验证，不新增真实硬件证据。
