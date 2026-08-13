# M6 J-Link RTT Telnet review（最后能力）

本轮按项目工作流启动产品、架构、UI、开发、验证、打包/流程六个只读角色；父级是唯一
源码写入者，六个角色完成后均已关闭。J-Link RTT 按用户要求最后接入；当前机器仍未安装
J-Link 驱动、SEGGER 工具或目标硬件。

## 六角色结论

| 角色 | 结论 | 约束/证据 |
|---|---|---|
| 产品 | 有条件放行 attach-only | 默认 `127.0.0.1:19021`；不把 RTT 伪装成完整 J-Link 调试器；复用终端、发送、raw 记录 |
| 架构 | 放行单写入实现 | RTT 只新增 typed config/adapter，复用 `SessionManager` 与 stream 生命周期；不新增 SDK/DLL、进程 supervisor 或专用 manager |
| UI | 放行 | 增加 `J-Link RTT（外部桥接）`、host/port、channel 0/1；不暴露 probe、memory、halt、flash、command file |
| 开发 | 放行 | `SocketStreamTransport` 复用 TCP/RTT socket 行为；`RttTransport` 只在连接短窗口发送 `RTTCh` Config String |
| 验证 | 非硬件门通过 | 本地 TCP 回环覆盖 channel 1 Config String、Up/Down bytes、SessionManager 路由、关闭和边界拒绝 |
| 打包/流程 | 内部验证包有条件放行 | 默认 onedir/onefile、BLE-enabled onefile 启动通过；默认包不收集 Bleak/WinRT、SEGGER/J-Link vendor 文件；正式发行仍需签名与许可审查 |

## 代码边界

- `RttTransportConfig`：schema v1、`PeerAddress`、有限 timeout/read chunk、channel 0/1；默认
  RTT Telnet 端口 19021；endpoint identity 包含 host/port/channel。
- `RttTransport`：标准库 TCP attach-only；发送
  `$$SEGGER_TELNET_ConfigStr=RTTCh;<channel>$$` 后仅传输原始 bytes。
- RTT Down 命令使用 `RttDownWrite(channel, StreamWrite)`，SessionManager 会拒绝与当前
  endpoint channel 不一致的写入。
- `SocketStreamTransport`：TCP Client 与 RTT 共用的有界 socket adapter；不把 RTT 设备
  句柄、SEGGER DLL 或 Qt 对象跨层传递。
- `RoutingTransportFactory`：注册 RTT typed config；普通 `SessionManager` 继续负责 worker、
  有界 outbound queue、取消、raw recorder、StreamData 事件和关闭。
- RTT 不进入 M4a/M4b protocol/component parser；首版只走 raw terminal/raw JSONL record，
  channel 作为会话 endpoint metadata 和配置身份保留。

## 非破坏性验证

已执行并通过：

- `uv run --locked ruff format --no-cache src`
- `uv run --locked ruff check --fix src`
- `uv run --locked ruff check src`
- `uv run --locked python -m compileall -q src`
- `uv lock --check`、`.\scripts\check.ps1`
- inline vector：本地 TCP server 收到 channel 1 Config String，返回 Up bytes，接收 Down bytes；
- inline vector：`RoutingTransportFactory` + `SessionManager` 收发/事件/关闭；
- inline vector：默认 RTT 端口、channel 0/1 边界和 SessionManager channel mismatch 拒绝；
- Qt offscreen：RTT combo/channel defaults、创建窗口、关闭和 backend worker shutdown；
- `.\scripts\package.ps1 -Mode onedir`，实际 EXE 启动并清理；
- `.\scripts\package.ps1 -Mode onefile`，实际 parent/child 启动并清理；
- `.\scripts\package.ps1 -Mode onefile -Ble`，BLE-enabled 实际启动并清理；随后恢复默认 onefile。

最终默认产物：

- `dist/SerialForge/SerialForge.exe`：2,912,335 bytes，SHA-256
  `969D90EE14F1BCFD837A7874C7CA2B0D8133145D1CA00CBE4751445FEF143CE9`；
- `dist/SerialForge.exe`：47,407,609 bytes，SHA-256
  `1FE0B79D7FAC73B14E480D491B24BF53FCA5090AC17071223D6618CA2B36DF85`；
- 默认 onedir 文件名扫描未发现 Bleak/WinRT/Bluetooth/SEGGER/J-Link/probe-rs vendor 文件；
  当前环境 `importlib.util.find_spec('bleak') is None`；
- 最终验证后没有残留 `SerialForge` 进程。

## 未运行项目与风险

- 未连接 J-Link、目标板或真实 RTT Telnet 服务；未验证目标 RTT 控制块、channel、活动 debug
  session、Down buffer 消费/丢失、吞吐、断线和多客户端行为；
- 未安装或启动 `JLink.exe`、`JLinkRTTClient.exe`、J-Link Commander/GDB Server；本机常见
  SEGGER 路径也不存在。真实 RTT 只能在用户安装驱动/工具并提供授权硬件后验收；
- 未验证正式发行签名、干净 Windows、Qt/SEGGER 商业许可和驱动安装流程；
- RTT Telnet 的连接错误只按 socket/open/timeout/EOF/close 事实呈现，不推断具体探针占用或
  目标未初始化；
- 由于没有 MCU/SoC、固件、BSP/HAL/CMSIS、RTOS 或 C/C++ 目标，本轮无适用厂商要求；不作
  MISRA、ISO 26262、ASPICE 或任何认证合规声明。

公共一手资料边界：

- [SEGGER J-Link RTT Client](https://kb.segger.com/J-Link_RTT_Client)：RTT Client 需要活动
  debug connection 且自身只支持 Up/Down channel 0；
- [SEGGER J-Link RTT Telnet Channel](https://kb.segger.com/J-Link_RTT_TELNET_Channel)：
  RTT Telnet 默认端口、原始 TCP 连接和 `RTTCh;<ChannelNo>` 配置串；
- [SEGGER J-Link Commander](https://kb.segger.com/J-Link_Commander)：RTT Telnet 端口配置
  由外部工具负责。上述资料只约束主机桥接行为，不是 MCU 厂商契约，也不构成授权结论。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
