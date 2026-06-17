# PRD-140 Serial Station UI Service Result

## 目标

把 Serial Station UI 的日志导出、回放和 Profile 读写入口切换到 controller 的 `OperationResult` API，避免用户操作失败时出现静默成功、异常冒泡或状态被破坏。

## 范围

- UI 日志导出调用 `export_log_result()`，失败时显示可见错误。
- UI 回放调用 `replay_log_result()`，失败时不清空当前可见日志。
- UI Profile 保存调用 `save_profile_result()`，失败时不误报成功。
- UI Profile 加载调用 `load_profile_result()`，失败时不破坏当前 Profile 输入和标签。
- 将文件会话操作拆入 `ui/session_actions.py`，降低 `main_window.py` 职责密度。

## 非目标

- 不改变日志、回放和 Profile 文件格式。
- 不新增脚本入口，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不提升真实设备验证状态。
- 不引入弹窗体系，本轮只把状态标签接到结构化失败。

## 验收

- 回放缺失文件时状态显示 `Replay failed`，且已有日志仍可见。
- Profile 缺失文件加载时状态显示 `Load profile failed`，且现有 Profile 名称和标签保持不变。
- 既有导出、回放、保存、加载成功路径继续通过 UI smoke。
- `main_window.py` 小于等于 300 行。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，UI result 失败路径和成功路径均有 pytest-qt 覆盖。
- 用户状态：`U3`，用户在主流程中能看到文件操作失败反馈，且失败不破坏当前工作台状态。
- 设备状态：`D1`，本轮仅涉及 UI/service 错误语义治理。
