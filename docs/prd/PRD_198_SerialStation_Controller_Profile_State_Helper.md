# PRD-198 Serial Station Controller Profile 状态 Helper

## 目标

把 `SerialWorkbenchController` 中 Profile 保存快照、Profile 加载后的命令历史恢复和 System 日志追加拆到独立 helper，减少主 controller 对档案状态细节的直接处理。

## 范围

- 新增 controller Profile 状态 helper。
- `save_profile_result` 和 `load_profile_result` 委托 helper。
- 保持 Profile JSON 字段、命令历史恢复规则、日志文案和错误结果不变。

## 非目标

- 不改变 Profile service 文件格式。
- 不新增 UI 控件或启动参数。
- 不改变日志导出/回放行为。

## 架构边界

- helper 位于 `python/embeddebug/serial_station/controllers/`。
- helper 只处理 controller 层 Profile 状态映射，不 import UI，不直接操作 QWidget。
- 测试位于 `tests/python/unit/`。

## 验收标准

- 单测覆盖保存快照字段、加载成功恢复命令历史并追加 System 日志、加载失败不改变状态。
- controller Profile 回归测试继续通过。
- 完整门禁通过：`uv run test-embeddebug-py`、`uv run test-embeddebug-tools`、`uv run start-embeddebug --smoke`、`cmd /c EmbedDebug.bat --smoke`。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，Profile 状态 helper 有自动化测试 |
| 用户状态 | `U3`，用户行为不变 |
| 设备状态 | `D2`，替身链路保持 |
