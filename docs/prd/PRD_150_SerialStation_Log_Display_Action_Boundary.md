# PRD-150 Serial Station Log Display Action Boundary

## 目标

将 Serial Station 日志展示、过滤和统计刷新逻辑归入 `ui/log_actions.py`，让 `MainWindow` 继续只承担用户事件转发和界面编排。

## 范围

- 新增 `log_actions.append_log_entry()`，集中处理新增日志条目的可见性判断、追加和统计刷新。
- 新增 `log_actions.render_log_entries()`，集中处理当前过滤条件下的日志重渲染。
- 新增 `log_actions.log_entry_visible()` 与 `update_log_stats()`，集中处理方向过滤、搜索过滤和统计文本。
- `MainWindow` 中日志相关方法只保留委托。
- 新增 UI 架构测试，防止日志展示逻辑回流到主窗口。

## 非目标

- 不改变日志服务、导出格式、回放文件格式和 controller 日志语义。
- 不新增 bat/cmd/ps1/sh 脚本，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不恢复 legacy native 源码、构建清单或打包链路。
- 不提升真实设备验证状态。

## 验收

- `MainWindow._append_log_entry()` 只调用 `log_actions.append_log_entry(self, entry)`。
- `MainWindow._render_log_entries()` 只调用 `log_actions.render_log_entries(self)`。
- `MainWindow._update_log_stats()` 只调用 `log_actions.update_log_stats(self)`。
- TX/RX 方向过滤、搜索过滤、日志统计和回放失败保留可见日志路径保持可用。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，日志展示动作边界有架构测试与 pytest-qt 覆盖。
- 用户状态：`U3`，日志过滤、搜索、统计和回放可见性路径保持稳定。
- 设备状态：`D2`，本轮使用 Fake transport 与 UI smoke 验证，不新增真实硬件证据。
