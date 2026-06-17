# PRD-189 Serial Station 协议切换日志

## 目标

把 Serial Station 的协议切换动作写入统一日志链路，方向为 `system`，让后续导出、回放、筛选和问题复盘能看到“当前数据按哪个协议解释”。

## 范围

- 用户或 Profile 触发 `set_protocol()` 后追加一条 `system` 日志。
- 日志文本格式为 `protocol: <name>`。
- UI 已有 System 筛选、统计、导出和回放能力继续复用，不新增独立展示控件。

## 非目标

- 不新增协议实现。
- 不改变协议 parser、dispatcher 的帧语义。
- 不新增脚本、打包或 native 入口。

## 架构边界

- `controller` 负责把协议选择转换为日志事实。
- `ui` 仍只调用 controller 并展示日志条目。
- `protocols` 不依赖 UI 或日志服务。

## 验收标准

- controller 切换到 `fire_water` 后 entries 末尾为 `system:protocol: fire_water`。
- UI 切换协议后日志区可显示 `System protocol: fire_water`。
- System 筛选下协议日志可见，TX/RX 日志被隐藏。
- 完整门禁通过：`uv run test-embeddebug-py`、`uv run test-embeddebug-tools`、`uv run start-embeddebug --smoke`、`cmd /c EmbedDebug.bat --smoke`。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，controller 与 UI smoke 覆盖 |
| 用户状态 | `U3`，协议上下文在日志主路径可见 |
| 设备状态 | `D2`，替身链路验证 |
