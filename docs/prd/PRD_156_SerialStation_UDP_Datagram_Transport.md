# PRD-156 Serial Station UDP Datagram Transport

## 目标

在 Python/PyQt Serial Station 驱动层新增 UDP datagram transport，并通过 `TransportRegistry` 默认暴露 `udp` 模式，推进 UART + TCP + UDP 多源接入目标。

## 范围

- 新增 `drivers/udp_datagram.py`，使用 PyQt `QUdpSocket` 实现 `SerialTransport` 字节收发契约。
- 支持 `host:port` endpoint，打开时绑定本地 UDP 端口并将写入数据发送到远端 endpoint。
- `TransportRegistry.with_defaults()` 默认注册 `udp`。
- 新增 UDP loopback 单测和 registry 默认模式测试。

## 非目标

- 不新增 UDP UI 控件、不修改 `MainWindow`、不新增 Profile 字段。
- 不实现 UDP broadcast/multicast、重传、分包、连接保活或并发多 endpoint。
- 不新增 bat/cmd/ps1/sh 脚本，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不恢复 legacy native 源码、原生构建清单或原生打包链路。
- 不提升真实设备验证状态。

## 验收

- `UdpDatagramTransport` 能拒绝非法 endpoint 并给出稳定错误码。
- `UdpDatagramTransport` 能在本机 loopback 下收发 datagram。
- `TransportRegistry.with_defaults().modes` 包含 `udp`，并能创建 UDP transport。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，UDP 驱动层和 registry 入口有单测覆盖。
- 用户状态：`U2`，UDP 已进入工程 registry，但 UI 入口尚未接入。
- 设备状态：`D2`，本轮使用本机 UDP loopback 替身验证，不新增真实硬件证据。
