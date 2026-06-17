# PRD-155 Serial Station Window Lifecycle Action Boundary

## 目标

将 Serial Station 主窗口关闭时的资源释放动作归入 `ui/lifecycle_actions.py`，让 `MainWindow.closeEvent()` 不直接操作具体子控件资源，为后续连接断开、后台任务停止和临时会话保存提供统一生命周期出口。

## 范围

- 新增 `lifecycle_actions.close_window()`，集中执行主窗口关闭前的 UI 资源释放。
- `MainWindow.closeEvent()` 只调用生命周期 action，然后继续调用父类默认关闭处理。
- 新增 UI 架构测试，防止 `_waveform_preview.shutdown()` 逻辑回流到主窗口。
- 保持现有启动 smoke、UI smoke 和波形预览关闭行为不变。

## 非目标

- 不新增会话自动保存、断线重连或后台 worker 管理。
- 不改变 `SerialWaveformPreview.closeEvent()` 自身的资源释放兜底。
- 不新增 bat/cmd/ps1/sh 脚本，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不恢复 legacy native 源码、原生构建清单或原生打包链路。
- 不提升真实设备验证状态。

## 验收

- `MainWindow.closeEvent()` 只调用 `lifecycle_actions.close_window(self)` 并保留父类默认关闭处理。
- `lifecycle_actions.close_window()` 负责关闭波形预览资源。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，窗口生命周期边界有架构测试和全量 Python 测试覆盖。
- 用户状态：`U3`，用户关闭窗口路径保持可用。
- 设备状态：`D2`，本轮使用 UI smoke 验证，不新增真实硬件证据。
