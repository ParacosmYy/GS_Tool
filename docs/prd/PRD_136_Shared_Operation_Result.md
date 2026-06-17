# PRD-136 Shared Operation Result

## 目标

为 Python/PyQt 主线增加统一操作结果值对象，减少 controller、transport、service 之间只返回 `bool` 或散落字符串错误的情况。

## 范围

- 在 `python/embeddebug/shared/` 增加轻量 `OperationResult` 与 `OperationError`。
- 结果对象必须表达成功状态、可选值、错误码和用户可读消息。
- Serial Station controller 先接入一条发送路径，保留现有 `bool` API 兼容 UI。
- 单测覆盖结果对象约束和 controller 失败语义。

## 非目标

- 不一次性重写所有 controller、driver 和 service 接口。
- 不改变 PyQt UI 文案和交互。
- 不提升真实设备验证状态。

## 验收

- 成功结果不能携带错误对象。
- 失败结果必须有非空错误码。
- `send_text_result()` 在未连接时返回结构化失败，并保持 `send_text()` 兼容返回 `False`。
- `uv run test-embeddebug-py` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，共享结果对象与 controller 接入路径有单测。
- 用户状态：`U3`，用户入口不变，但后续错误展示和恢复路径具备统一底座。
- 设备状态：`D1`，本轮只涉及软件错误语义治理。
