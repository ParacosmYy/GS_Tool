# PRD-148 Serial Station Command History Action Boundary

## 目标

将 Serial Station 命令历史刷新与选择入口归入 `ui/connection_actions.py`，让 `MainWindow` 继续只承担用户事件转发和界面编排。

## 范围

- 新增 `connection_actions.refresh_command_history()`，集中处理命令历史下拉框刷新、选中和启用状态。
- 新增 `connection_actions.select_command_history()`，集中处理历史命令回填输入框。
- `connection_actions.send_text()` 在发送成功后直接刷新命令历史，不再依赖主窗口实现细节。
- `MainWindow._refresh_command_history()` 与 `_select_command_history()` 只保留委托。
- 新增 UI 架构测试，防止命令历史动作回流到主窗口。

## 非目标

- 不改变 controller 的命令历史存储、去重、Profile 格式和发送语义。
- 不新增 bat/cmd/ps1/sh 脚本，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不恢复 legacy native 源码、构建清单或打包链路。
- 不提升真实设备验证状态。

## 验收

- `MainWindow._refresh_command_history()` 只调用 `connection_actions.refresh_command_history(self)`。
- `MainWindow._select_command_history()` 只调用 `connection_actions.select_command_history(self, text)`。
- 发送成功后历史下拉框仍可刷新；选择历史命令仍可回填输入框。
- Profile 加载后的命令历史刷新路径保持可用。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，命令历史动作边界有架构测试与 pytest-qt 覆盖。
- 用户状态：`U3`，发送、历史回填和 Profile 恢复路径保持稳定。
- 设备状态：`D2`，本轮使用 Fake transport 与 UI smoke 验证，不新增真实硬件证据。
