# PRD-193 Serial Station Controller 日志状态拆分

## 目标

把 `SerialWorkbenchController` 内部日志 entry 追加、System/Error entry 构造和回调通知拆到独立 helper，降低 controller 主文件行数压力，让 controller 更专注于流程编排。

## 范围

- 新增 controller 层日志状态 helper。
- 保持普通日志 entry 追加后通知 `on_log_entry` 回调。
- 保持 System 日志构造规则：`direction=system`、`raw=text.encode("utf-8")`。
- 保持 Error 日志构造规则：先写入 Error entry，再通知 `on_error` 回调。
- `SerialWorkbenchController` 对外 API、日志导出/回放、筛选统计行为不变。

## 非目标

- 不改变日志格式。
- 不改变 JSONL 导出格式。
- 不改变 UI 文案。
- 不新增脚本、打包链路或 native 入口。

## 架构边界

- helper 位于 `python/embeddebug/serial_station/controllers/`，只服务 controller 层状态编排。
- helper 不依赖 UI、services、drivers 或具体协议。
- controller 继续负责决定何时记录日志，不负责展开日志状态细节。

## 验收标准

- helper 能追加普通 entry 并按快照通知 log callback。
- helper 能构造 System entry 并通知 log callback。
- helper 能构造 Error entry，写入日志后通知 error callback。
- `workbench_controller.py` 低于 280 行。
- 完整门禁通过：`uv run test-embeddebug-py`、`uv run test-embeddebug-tools`、`uv run start-embeddebug --smoke`、`cmd /c EmbedDebug.bat --smoke`。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，纯单测与 controller 回归覆盖 |
| 用户状态 | `U3`，日志可见行为不变 |
| 设备状态 | `D2`，替身链路保持 |
