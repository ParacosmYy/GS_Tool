# PRD-133 Serial Station TCP Client Transport

## 目标

为 Serial Station 增加 TCP client 字节通道，使工作台从单一 UART 走向 UART + TCP 的多源接入结构。

## 范围

- 在 `drivers/` 增加 PyQt6 TCP client transport。
- 通过 transport registry 暴露稳定 `tcp` mode。
- Controller 只负责编排 `host:port` 配置与 profile 状态，不处理 socket 细节。
- 单测覆盖 endpoint 校验、loopback 收发和 profile 持久化。

## 非目标

- 不新增 UI 面板。
- 不实现 UDP。
- 不改变协议解析和日志服务。
- 不宣称真实设备验证完成。

## 验收

- registry 默认包含 `fake`、`serial`、`tcp`。
- TCP client 可通过本机 loopback 收发字节。
- Controller 能保存 `tcp` mode 与 endpoint。
- `uv run test-embeddebug-py` 和 smoke 启动通过。

## 三轴状态

- 工程状态：`E4`，新增 driver 与 controller 关键路径有单测。
- 用户状态：`U2`，controller 能力已具备，UI 入口待后续接入。
- 设备状态：`D2`，本轮通过本机 loopback 替身链路验证。
