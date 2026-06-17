# PRD-144 Serial Station Clear Log Action Boundary

## 目标

将 Serial Station 清空日志入口归入 `ui/session_actions.py`，让 `MainWindow` 继续只承担用户事件转发和界面编排。

## 范围

- 新增 `session_actions.clear_log()`，集中处理 controller 清空、日志视图清空、统计刷新和状态栏反馈。
- `MainWindow._clear_log()` 只委托 session action。
- 新增 UI 架构测试，防止清空日志逻辑回流到主窗口。
- 既有按钮清空和 `Ctrl+L` 清空路径保持原行为。

## 非目标

- 不改变日志数据结构、导出格式、回放语义和快捷键定义。
- 不新增脚本入口，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不提升真实设备验证状态。

## 验收

- `MainWindow._clear_log()` 只调用 `session_actions.clear_log(self)`。
- 清空日志后可见日志为空，统计归零，状态栏显示 `Log cleared`。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，清空日志动作边界有架构测试与 pytest-qt 覆盖。
- 用户状态：`U3`，清空日志用户路径保持稳定。
- 设备状态：`D2`，本轮不新增真实硬件证据。
