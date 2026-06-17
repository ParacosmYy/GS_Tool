# PRD-191 Serial Station 命令历史状态拆分

## 目标

把 `SerialWorkbenchController` 内部命令历史去重与 Profile 恢复逻辑拆到独立 helper，降低 controller 行数压力，为后续 PyQt 上位机功能迭代留出可维护空间。

## 范围

- 新增 controller 层命令历史状态 helper。
- 保持成功发送后“去重并移动到末尾”的行为不变。
- 保持 Profile 加载时只恢复非空字符串的行为不变。
- `SerialWorkbenchController` 对外 API、UI 行为和 Profile 文件格式不变。

## 非目标

- 不新增 UI 控件。
- 不改变命令历史容量策略。
- 不改变 Profile JSON 字段。
- 不新增脚本、打包或 native 入口。

## 架构边界

- helper 位于 `python/embeddebug/serial_station/controllers/`，只服务 controller 层状态编排。
- UI 仍只读取 controller 暴露的 `command_history`。
- services 仍只负责 Profile 文件读写。

## 验收标准

- helper 能把重复命令移动到末尾。
- helper 能从 Profile 值恢复非空字符串并忽略无效值。
- controller 现有命令历史相关测试保持通过。
- `workbench_controller.py` 明显低于 300 行。
- 完整门禁通过：`uv run test-embeddebug-py`、`uv run test-embeddebug-tools`、`uv run start-embeddebug --smoke`、`cmd /c EmbedDebug.bat --smoke`。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，纯单测与 controller 回归覆盖 |
| 用户状态 | `U3`，用户行为不变 |
| 设备状态 | `D2`，替身链路保持 |
