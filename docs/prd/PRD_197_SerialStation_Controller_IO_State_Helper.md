# PRD-197 Serial Station Controller I/O 状态 Helper

## 目标

把 `SerialWorkbenchController` 中发送文本、Fake RX 注入与错误结果映射拆到独立 helper，减少主 controller 对 transport I/O 细节的直接编排。

## 范围

- 新增 controller I/O helper。
- `send_text_result` 和 `inject_received_text` 委托 helper。
- 保持错误码、日志条目、命令历史和用户可见行为不变。

## 非目标

- 不改变协议构建规则。
- 不改变 Fake transport 或真实 transport。
- 不新增 UI 控件、脚本或打包链路。

## 架构边界

- helper 位于 `python/embeddebug/serial_station/controllers/`。
- helper 只处理 controller 层 I/O 编排，不 import UI，不做协议 parser 实现。
- 测试位于 `tests/python/unit/`。

## 验收标准

- 单测覆盖未连接发送失败、发送成功命令历史与日志入库、非 Fake 注入失败。
- controller 回归测试继续通过。
- 完整门禁通过：`uv run test-embeddebug-py`、`uv run test-embeddebug-tools`、`uv run start-embeddebug --smoke`、`cmd /c EmbedDebug.bat --smoke`。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，controller I/O 状态 helper 有自动化测试 |
| 用户状态 | `U3`，用户行为不变 |
| 设备状态 | `D2`，Fake 替身链路保持 |
