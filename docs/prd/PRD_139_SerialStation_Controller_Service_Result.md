# PRD-139 Serial Station Controller Service Result

## 目标

把日志导出、回放读取和 Profile 读写的结构化结果继续上浮到 controller 层，让 PyQt UI 后续只消费统一 `OperationResult`，不直接依赖文件异常或 service 细节。

## 范围

- `SerialWorkbenchController.export_log_result()` 返回日志导出结果。
- `SerialWorkbenchController.replay_log_result()` 返回回放读取结果，失败时不清空当前日志。
- `SerialWorkbenchController.save_profile_result()` 与 `load_profile_result()` 返回 Profile 读写结果。
- 日志条目模型与协议事件转换拆出独立 helper，降低 controller 文件体积和职责密度。
- 旧 `export_log()`、`replay_log()`、`save_profile()`、`load_profile()` 保持兼容。

## 非目标

- 不改 PyQt 可见文案和布局。
- 不改日志、回放和 Profile 文件格式。
- 不提升真实设备验证状态。
- 不新增脚本入口；用户入口继续保持 `EmbedDebug.bat -> uv run start-embeddebug`。

## 验收

- 成功路径返回 `OperationResult.ok` 并保留既有文件读写能力。
- 日志导出失败返回 `log_export_failed`。
- 回放读取失败返回 `replay_load_failed`，且不破坏当前日志状态。
- Profile 保存失败返回 `profile_save_failed`。
- Profile 加载失败返回 `profile_load_failed`，且不破坏命令历史。
- controller 运行时文件继续小于等于 300 行。
- `uv run test-embeddebug-py` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，controller/service result 链路有单测覆盖。
- 用户状态：`U3`，现有用户路径兼容，后续 UI 可接统一错误展示。
- 设备状态：`D1`，本轮只涉及文件服务错误语义和 controller 编排治理。
