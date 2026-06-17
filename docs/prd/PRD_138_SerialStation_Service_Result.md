# PRD-138 Serial Station Service Result

## 目标

把 Serial Station service 层常见文件操作接入 `OperationResult`，让日志/测量导出、回放读取和 Profile 读写失败具备统一错误码。

## 范围

- `SerialMeasurementExportService.export_csv_result()` 返回结构化导出结果。
- `SerialReplayService.load_events_result()` 返回结构化回放读取结果。
- `SerialProfileService.save_result()` 与 `load_result()` 返回结构化 Profile 读写结果。
- 旧 API 保持不变，继续兼容现有 controller 与 UI。

## 非目标

- 不改变 PyQt UI 文案和交互。
- 不改变日志、回放、Profile 文件格式。
- 不提升真实设备验证状态。

## 验收

- 写入失败返回 `measurement_export_failed` 或 `profile_save_failed`。
- 读取失败返回 `replay_load_failed` 或 `profile_load_failed`。
- 旧 service 方法仍可按既有测试完成读写。
- `uv run test-embeddebug-py` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，service result API 有单测覆盖。
- 用户状态：`U3`，用户路径保持兼容，后续 UI 可接统一错误展示。
- 设备状态：`D1`，本轮只涉及文件服务错误语义治理。
