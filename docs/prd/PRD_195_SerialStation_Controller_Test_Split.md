# PRD-195 Serial Station 测试行为域拆分

## 目标

把过大的 controller 单测与 UI smoke 测试按行为域拆分，避免回归测试继续膨胀成单文件总集，保持 Python/PyQt 主线测试可维护、可定位。

## 范围

- 新增 Profile / Session 行为域测试文件。
- 将 profile 保存/加载、service result、回放诊断和失败不变性测试迁移出主 controller 测试文件。
- 将 Serial Station MVP UI smoke 按基础路径、日志筛选统计、传输入口拆分。
- 保持测试行为不变，不改生产代码。
- 所有 Python 测试文件降到 300 行以下。

## 非目标

- 不改变 controller API。
- 不改变日志、Profile 或 replay 行为。
- 不新增脚本、打包链路或 native 入口。

## 架构边界

- 测试仍位于 `tests/python/unit/`。
- 运行时代码不因测试拆分而改变。
- 拆分后测试文件按行为域命名，便于后续并行维护。

## 验收标准

- 架构测试证明主 controller 测试文件低于 300 行。
- Profile / Session 行为测试在独立文件中存在。
- UI smoke 日志与传输行为测试在独立文件中存在。
- 被迁移测试继续通过。
- 完整门禁通过：`uv run test-embeddebug-py`、`uv run test-embeddebug-tools`、`uv run start-embeddebug --smoke`、`cmd /c EmbedDebug.bat --smoke`。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，测试结构和全量门禁可复现 |
| 用户状态 | `U3`，用户行为不变 |
| 设备状态 | `D2`，替身链路保持 |
