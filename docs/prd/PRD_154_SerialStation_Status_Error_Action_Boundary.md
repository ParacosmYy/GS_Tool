# PRD-154 Serial Station Status Error Action Boundary

## 目标

将 Serial Station 控制器错误回调的状态栏展示动作归入 `ui/status_actions.py`，让 `MainWindow` 不直接拼接错误文案或操作状态标签，为后续统一错误码、告警规则和状态栏策略保留扩展点。

## 范围

- 新增 `status_actions.show_error()`，集中处理 controller `on_error` 回调的错误展示。
- `MainWindow._show_error()` 只保留委托。
- 新增 UI 架构测试，防止错误状态展示逻辑回流到主窗口。
- 保持现有连接、发送、注入、回放和 Profile 失败反馈路径不变。

## 非目标

- 不新增全局告警中心、错误码枚举、toast 或弹窗体系。
- 不改变现有 connection/session/injection action 的状态栏成功或失败文案。
- 不新增 bat/cmd/ps1/sh 脚本，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不恢复 legacy native 源码、原生构建清单或原生打包链路。
- 不提升真实设备验证状态。

## 验收

- `MainWindow._show_error()` 只调用 `status_actions.show_error(self, message)`。
- controller error callback 仍能将错误显示为 `Error: <message>`。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，错误状态展示边界有架构测试和全量 Python 测试覆盖。
- 用户状态：`U3`，用户仍能看到 controller 错误反馈。
- 设备状态：`D2`，本轮使用替身和 UI smoke 验证，不新增真实硬件证据。
