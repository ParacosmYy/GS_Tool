# PRD-149 Serial Station Profile Control Action Boundary

## 目标

将 Serial Station Profile 加载后的控件回填逻辑归入 `ui/session_actions.py`，让 `MainWindow` 继续只承担用户事件转发和界面编排。

## 范围

- 新增 `session_actions.apply_profile_controls()`，集中处理协议、端口、TCP endpoint、UART 参数和连接控件状态回填。
- 新增 `session_actions._select_combo_value()`，复用下拉框选中与补项逻辑。
- `session_actions.load_profile()` 在 Profile 加载成功后直接调用 action 回填控件。
- `MainWindow._apply_profile_controls()` 只保留委托，不再直接操作 Profile 控件细节。
- 新增 UI 架构测试，防止 Profile 控件回填逻辑回流到主窗口。

## 非目标

- 不改变 Profile 文件格式、controller 读写语义、TCP endpoint 保存规则和命令历史恢复语义。
- 不新增 bat/cmd/ps1/sh 脚本，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不恢复 legacy native 源码、构建清单或打包链路。
- 不提升真实设备验证状态。

## 验收

- `MainWindow._apply_profile_controls()` 只调用 `session_actions.apply_profile_controls(self, profile)`。
- Profile 加载后协议、串口端口、TCP host/port、UART 参数和命令历史仍可恢复。
- Profile 加载失败时当前界面状态保持不被覆盖。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，Profile 控件回填动作边界有架构测试与 pytest-qt 覆盖。
- 用户状态：`U3`，Profile 加载和恢复用户路径保持稳定。
- 设备状态：`D2`，本轮使用 Fake/TCP 替身与 UI smoke 验证，不新增真实硬件证据。
