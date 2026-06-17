# PRD-153 Serial Station Keypress Shortcut Action Boundary

## 目标

将 Serial Station 主窗口的按键兜底分发逻辑归入 `ui/shortcuts.py`，让 `MainWindow.keyPressEvent()` 只负责委托和默认事件上抛，为后续统一快捷键体系保留清晰扩展点。

## 范围

- 新增 `shortcuts.handle_key_press()`，集中处理 Ctrl+Enter、Ctrl+L、Ctrl+R。
- `MainWindow.keyPressEvent()` 只调用快捷键分发函数，未命中时调用父类默认处理。
- 新增 UI 架构测试，防止按键分发逻辑回流到主窗口。
- 保持现有 UI smoke 中发送、清空日志和刷新端口快捷键路径不变。

## 非目标

- 不新增全局命令面板、可编辑快捷键配置或持久化快捷键映射。
- 不改变 `install_shortcuts()` 的 `QShortcut` 安装行为。
- 不新增 bat/cmd/ps1/sh 脚本，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不恢复 legacy native 源码、原生构建清单或原生打包链路。
- 不提升真实设备验证状态。

## 验收

- `MainWindow.keyPressEvent()` 只调用 `shortcuts.handle_key_press(self, event)` 并保留未处理事件的默认上抛。
- Ctrl+Enter 仍触发发送，Ctrl+L 仍清空日志，Ctrl+R 仍刷新串口列表。
- `tests/python/ui_smoke/test_serial_station_mvp.py::test_pyqt_mvp_shortcuts_send_clear_and_refresh` 保持通过。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，快捷键兜底分发边界有架构测试与 UI smoke 覆盖。
- 用户状态：`U3`，用户原有快捷键路径保持可用。
- 设备状态：`D2`，本轮使用 Fake/TCP/串口枚举替身与 UI smoke 验证，不新增真实硬件证据。
