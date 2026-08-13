# ADR 0003：M2a 网络传输边界

状态：已接受（2026-08-09）

## 背景

SerialForge 需要支持 Wi-Fi 调试场景，但 TCP 和 UDP 的数据语义不同。当前 UART
切片的 `TransportPort` 面向连续字节流，直接把 UDP 或未来的 TCP Server 塞进同一
个会话对象，会把 EOF、datagram 边界、peer 身份和多客户端生命周期混在一起，降低
扩展性并增加关闭竞态。

## 决策

本轮只交付 M2a：

- TCP Client：单个远端、手动连接/断开、有限连接/读写超时，不自动重连；
- UDP 单播：单个固定远端、显式本地绑定、每次收发保留一个 datagram 边界；
- 两者复用已有终端、暂停预览、原始 JSONL 记录、发送历史和快捷命令；
- 传输端口按语义分离：stream 端口返回 DATA/TIMEOUT/EOF，datagram 端口返回带
  `PeerAddress` 的报文；
- TCP/UDP 事件和原始记录都可以携带 peer 元数据，UART 的 peer 继续为空；
- 使用 Python 标准库 `socket`，不新增第三方依赖，不引入 J-Link/SEGGER 依赖。

明确不在 M2a：TCP Server、多个 client、隐式广播、TLS/DTLS、协议级序列号统计、
自动重连、BLE 和 J-Link RTT。

## 约束

- TCP `recv()` 返回空字节表示对端 EOF，不得当成读超时；UART 的有限超时仍表示
  TIMEOUT。EOF 必须进入可见的关闭/错误状态，并保留已有记录。
- TCP 收发只报告本地提交/收到的字节，不把一次 `recv()` 当作消息边界；
- UDP 默认最大 payload 为 4 KiB，硬上限为 65,507 字节；超限拒绝并产生可见错误，
  不静默截断；
- UDP 只接受配置的远端 peer。非法来源、发送失败、本地队列丢弃和超限都要可观测；
  没有应用层序列号时，不声称能够推断网络丢包或乱序；
- 不在应用启动时自动连接或监听；网络连接由用户显式发起；
- TCP Server 后续默认只监听 `127.0.0.1`，LAN 监听必须显式确认并配置 allowlist，
  由 M2b/M2c 独立实现，不提前污染 M2a 的单连接模型。

## 后续拆分

- M2b：TCP Server 单客户端、监听状态、回环默认值、LAN allowlist；
- M2c：有界多客户端、peer 状态、准入/拒绝和逐 peer 背压；
- 协议里程碑：只有引入应用层序列号后，才增加丢包/乱序统计。

## 验收证据

M2a 必须有实际 loopback socket 证据：TCP 分段/合并发送后原始字节完全一致，
TCP EOF 与 timeout 可区分；UDP 多个报文保持边界并携带来源 peer，超限/非法来源
不会产生部分记录。还需通过静态检查、Qt 启动关闭和 onedir/onefile 打包启动。
真实 LAN、防火墙和硬件设备验证在具备环境后单独记录。
