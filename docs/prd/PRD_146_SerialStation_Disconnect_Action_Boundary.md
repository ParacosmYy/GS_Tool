# PRD-146 Serial Station Disconnect Action Boundary

## 目标

将 Serial Station 断开连接入口归入 `ui/connection_actions.py`，让 `MainWindow` 继续只承担用户事件转发和界面编排。

## 范围

- 新增 `connection_actions.disconnect()`，集中处理 controller 断开、状态栏反馈和连接控件状态刷新。
- `MainWindow._disconnect()` 只委托 connection action。
- 新增 UI 架构测试，防止断开连接逻辑回流到主窗口。
- 既有 Fake/Serial/TCP 连接、断开按钮状态和 UI smoke 行为保持不变。

## 非目标

- 不改变 transport 生命周期、连接错误码、Profile 格式和协议行为。
- 不新增脚本入口，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不提升真实设备验证状态。

## 验收

- `MainWindow._disconnect()` 只调用 `connection_actions.disconnect(self)`。
- 断开后状态栏仍显示 `Disconnected`，连接按钮恢复可用。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，断开连接动作边界有架构测试与 pytest-qt 覆盖。
- 用户状态：`U3`，连接/断开用户路径保持稳定。
- 设备状态：`D2`，本轮不新增真实硬件证据。
