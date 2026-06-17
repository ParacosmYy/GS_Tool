# PRD-151 Serial Station Connection Control State Action Boundary

## 目标

将 Serial Station 连接控件启用状态和串口可用性判断归入 `ui/connection_actions.py`，让 `MainWindow` 继续只承担用户事件转发和界面编排。

## 范围

- 新增 `connection_actions.set_connected_controls()`，集中处理 Fake/Serial/TCP/Disconnect 按钮启用状态。
- 新增 `connection_actions.has_serial_ports()`，集中判断当前串口下拉框是否包含可连接端口。
- `MainWindow._set_connected_controls()` 与 `_has_serial_ports()` 只保留委托。
- 新增 UI 架构测试，防止连接控件状态逻辑回流到主窗口。

## 非目标

- 不改变 fake/serial/tcp transport 生命周期、连接结果错误码和 Profile 格式。
- 不新增 bat/cmd/ps1/sh 脚本，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不恢复 legacy native 源码、构建清单或打包链路。
- 不提升真实设备验证状态。

## 验收

- `MainWindow._set_connected_controls()` 只调用 `connection_actions.set_connected_controls(self, connected)`。
- `MainWindow._has_serial_ports()` 只调用 `connection_actions.has_serial_ports(self)`。
- Fake/Serial/TCP 连接、断开和刷新端口后按钮状态保持可用。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，连接控件状态动作边界有架构测试与 pytest-qt 覆盖。
- 用户状态：`U3`，连接、断开和端口刷新用户路径保持稳定。
- 设备状态：`D2`，本轮使用 Fake/TCP/串口枚举替身与 UI smoke 验证，不新增真实硬件证据。
