# PRD-194 Serial Station Controller 连接日志拆分

## 目标

把 `SerialWorkbenchController` 中连接成功日志追加规则继续下沉到 controller 日志状态 helper，减少主 controller 的私有包装方法，让连接流程和日志状态职责更清晰。

## 范围

- `controller_log_state` 新增连接成功日志 helper。
- 成功连接且带 `SerialPortConfig` 时追加 `connected: <mode> <portName>` System entry。
- 连接失败或缺少配置时不追加日志。
- `SerialWorkbenchController` 对外连接 API、日志文案和回调行为保持不变。

## 非目标

- 不改变连接结果结构。
- 不改变错误日志策略。
- 不改变 UI 状态文案。
- 不新增脚本、打包链路或 native 入口。

## 架构边界

- helper 位于 `python/embeddebug/serial_station/controllers/`。
- helper 只依赖 controller 日志 entry、共享 OperationResult 和 driver 配置值对象。
- UI、services、drivers 不反向依赖 controller。

## 验收标准

- helper 成功路径追加 System 日志并通知 log callback。
- helper 失败路径不追加日志、不通知 callback。
- `workbench_controller.py` 继续低于 280 行。
- 完整门禁通过：`uv run test-embeddebug-py`、`uv run test-embeddebug-tools`、`uv run start-embeddebug --smoke`、`cmd /c EmbedDebug.bat --smoke`。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，纯单测与 controller 回归覆盖 |
| 用户状态 | `U3`，连接日志可见行为不变 |
| 设备状态 | `D2`，替身链路保持 |
