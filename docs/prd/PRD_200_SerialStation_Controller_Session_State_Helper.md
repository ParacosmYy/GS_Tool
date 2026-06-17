# PRD-200 Serial Station Controller Session 状态 Helper

## 目标

把 `SerialWorkbenchController` 中日志清空、日志导出和回放恢复状态拆到独立 helper，减少主 controller 对 session 服务结果和 entries 变更细节的直接处理。

## 范围

- 新增 controller Session 状态 helper。
- `clear_log`、`export_log_result`、`replay_log_result` 委托 helper。
- 保持日志导出格式、回放 entries 顺序、回调通知和失败不变性不变。

## 非目标

- 不改变 service 层日志格式。
- 不改变 Profile 行为。
- 不新增 UI 控件、启动参数或打包入口。

## 架构边界

- helper 位于 `python/embeddebug/serial_station/controllers/`。
- helper 只处理 controller 层 session 状态映射，不 import UI，不直接触碰 QWidget。
- 测试位于 `tests/python/unit/`。

## 验收标准

- 单测覆盖清空日志、导出日志、回放成功更新 entries 并通知回调、回放失败不改变 entries。
- controller Profile/Session 回归测试继续通过。
- 完整门禁通过：`uv run test-embeddebug-py`、`uv run test-embeddebug-tools`、`uv run start-embeddebug --smoke`、`cmd /c EmbedDebug.bat --smoke`。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，Session 状态 helper 有自动化测试 |
| 用户状态 | `U3`，用户行为不变 |
| 设备状态 | `D2`，替身链路保持 |
