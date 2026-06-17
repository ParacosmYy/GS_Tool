# PRD-145 Serial Station Protocol Action Boundary

## 目标

将 Serial Station 协议选择入口归入独立 `ui/protocol_actions.py`，让 `MainWindow` 继续只承担用户事件转发和界面编排。

## 范围

- 新增 `protocol_actions.select_protocol()`，集中处理协议切换和状态栏反馈。
- `MainWindow._set_protocol()` 只委托 protocol action。
- 新增 UI 架构测试，防止协议选择逻辑回流到主窗口。
- 既有协议切换、日志、回放和 UI smoke 行为保持不变。

## 非目标

- 不改变协议 registry、协议解析、命令构建和日志格式。
- 不新增脚本入口，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不提升真实设备验证状态。

## 验收

- `MainWindow._set_protocol()` 只调用 `protocol_actions.select_protocol(self, name)`。
- 协议选择后状态栏仍显示 `Protocol: <name>`。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，协议选择动作边界有架构测试与 pytest 覆盖。
- 用户状态：`U3`，协议选择用户路径保持稳定。
- 设备状态：`D2`，本轮不新增真实硬件证据。
