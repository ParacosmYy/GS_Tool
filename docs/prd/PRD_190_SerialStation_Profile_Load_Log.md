# PRD-190 Serial Station Profile 加载日志

## 目标

把 Serial Station 的 Profile 加载动作写入统一日志链路，方向为 `system`，让导出、回放和现场复盘能明确当前工位配置来自哪个 Profile。

## 范围

- `load_profile_result()` 成功后追加一条 `system` 日志。
- 日志文本格式为 `profile loaded: <name>`。
- 失败加载不改变现有日志和 Profile 状态。
- UI 继续复用现有日志区、System 筛选、统计、导出和回放能力。

## 非目标

- 不新增 Profile 文件格式。
- 不改变 Profile 控件回填规则。
- 不新增脚本、打包或 native 入口。

## 架构边界

- `controller` 把 Profile 加载结果转换为日志事实。
- `ui` 只调用 controller 并渲染日志条目。
- `services` 继续只负责 Profile 文件读写。

## 验收标准

- controller 成功加载名为 `factory-profile` 的 Profile 后 entries 末尾包含 `system:profile loaded: factory-profile`。
- UI 加载 Profile 后日志区显示 `System profile loaded: serial-profile`。
- 加载失败不追加 Profile 加载日志。
- 完整门禁通过：`uv run test-embeddebug-py`、`uv run test-embeddebug-tools`、`uv run start-embeddebug --smoke`、`cmd /c EmbedDebug.bat --smoke`。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，controller 与 UI smoke 覆盖 |
| 用户状态 | `U3`，Profile 上下文在日志主路径可见 |
| 设备状态 | `D2`，文件与替身链路验证 |
