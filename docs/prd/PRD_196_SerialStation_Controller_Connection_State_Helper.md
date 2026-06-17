# PRD-196 Serial Station Controller 连接状态 Helper

## 目标

把 `SerialWorkbenchController` 中 transport 生命周期相关的本地端口读取、transport 替换绑定和断开日志追加拆到独立 helper，降低 controller 主编排文件复杂度。

## 范围

- 新增 controller 连接状态 helper。
- `active_local_port`、`_replace_transport`、`disconnect` 复用 helper。
- 保持用户行为、日志文案、连接 API 和 Profile 行为不变。
- `workbench_controller.py` 继续向 300 行以下安全区间收敛。

## 非目标

- 不改变 transport registry。
- 不新增连接类型。
- 不改 UI 控件、脚本、打包或 native 相关入口。

## 架构边界

- helper 位于 `python/embeddebug/serial_station/controllers/`，只服务 controller 层状态编排。
- helper 不 import UI，不解析协议，不做文件读写。
- 测试位于 `tests/python/unit/`。

## 验收标准

- helper 单测覆盖本地端口读取、替换 transport 关闭旧连接并绑定回调、断开成功时追加 System 日志。
- controller 回归测试继续通过。
- 完整门禁通过：`uv run test-embeddebug-py`、`uv run test-embeddebug-tools`、`uv run start-embeddebug --smoke`、`cmd /c EmbedDebug.bat --smoke`。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，controller 生命周期 helper 有自动化测试 |
| 用户状态 | `U3`，用户行为不变 |
| 设备状态 | `D2`，替身链路保持 |
