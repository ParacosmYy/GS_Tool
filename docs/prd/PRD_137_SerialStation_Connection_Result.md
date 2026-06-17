# PRD-137 Serial Station Connection Result

## 目标

把 Serial Station 连接入口纳入 `OperationResult` 结构化失败语义，让 fake、serial、tcp 连接路径具备统一成功值和错误码。

## 范围

- 增加连接结果 helper，统一把 transport open 结果转换为 `OperationResult`。
- 为 `connect_fake`、`connect_serial`、`connect_tcp` 增加对应 `*_result()` API。
- 保留旧 `bool` API 兼容现有 PyQt UI。
- 拆出 profile snapshot 构建，保持 controller 文件体积门禁。

## 非目标

- 不改变 PyQt UI 文案和交互。
- 不重写 transport interface。
- 不提升真实设备验证状态。

## 验收

- fake open 失败返回 `transport_open_failed`。
- serial/tcp 连接成功返回实际 `SerialPortConfig`。
- 原有 `connect_*()` 仍返回 `bool`。
- `workbench_controller.py` 不超过 300 行。
- `uv run test-embeddebug-py` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，连接结果 API 有 controller 单测和文件体积门禁。
- 用户状态：`U3`，UI 路径保持兼容，后续可接统一错误展示。
- 设备状态：`D1`，本轮只涉及连接错误语义治理。
