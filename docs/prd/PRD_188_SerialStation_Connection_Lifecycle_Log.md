# PRD-188 Serial Station 连接生命周期日志

## 目标

把 Serial Station 的连接成功与断开动作写入统一日志链路，方向为 `system`，使用户可以在同一日志区完成筛选、统计、导出与回放。

## 范围

- Fake、Serial、TCP、UDP 连接成功后追加一条 `system` 日志。
- 已连接状态下执行断开后追加一条 `system` 日志。
- 失败连接不追加成功日志，错误仍走现有 Error 日志链路。
- UI 日志统计把生命周期日志纳入 System 计数。

## 非目标

- 不新增传输协议。
- 不改变底层 transport 接口。
- 不引入新的脚本入口或打包入口。

## 架构边界

- `controller` 负责把连接结果转为日志事实。
- `ui` 只展示 controller 推送的日志条目。
- `services` 继续复用现有导出与回放能力。
- `core`、`protocols` 不感知生命周期展示语义。

## 验收标准

- 连接 Fake 后日志末尾存在 `system:connected: fake FAKE_LOOPBACK`。
- 断开 Fake 后日志末尾存在 `system:disconnected: fake`。
- 导出与回放保持 `system` 方向。
- UI 统计显示 System 数量。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools`、`uv run start-embeddebug --smoke`、`cmd /c EmbedDebug.bat --smoke` 通过。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，controller 与 UI smoke 覆盖 |
| 用户状态 | `U3`，连接生命周期在主日志区可见 |
| 设备状态 | `D2`，Fake/endpoint 替身链路验证 |
