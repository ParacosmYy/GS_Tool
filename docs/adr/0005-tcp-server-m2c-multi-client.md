# ADR 0005：M2c TCP Server 有界多客户端

状态：已接受（2026-08-09）

## 背景

M2b 已经建立了独立的 TCP Server session/transport 边界。M2c 需要在不污染 TCP
Client、UDP 和通用终端组件的前提下，把一个 listener 扩展为多个可独立观察和发送的
accepted connection。网络监听、慢 peer、断开重连和窗口关闭都必须保持有界行为。

## 决策

- `TcpServerSessionManager` 与 `TcpServerTransportPort` 继续独立于 M2a manager；
- `max_clients` 取 1–16，默认 4，并放在配置 dataclass 的旧字段末尾以保持 M2b 位置参数兼容；
- 每次 `accept` 生成新的 UUID `PeerId`。`PeerAddress` 只用于显示、allowlist 和审计，不能作为
  socket、队列、发送目标或重连身份；同一地址再次连接也必须得到新的 `PeerId`；
- 每次发送使用 `TcpServerSend(PeerId, StreamWrite)`，没有 client 或目标已断开时失败；不做广播、
  自动切换、自动重连、可靠重放或把旧目标转投到新连接；
- application 层保留每 peer FIFO 队列，transport 层最多保留一个 in-flight partial write；总出站字节
  上限为 16 MiB，单 peer 的有效上限不超过 `min(per_peer_limit, total_limit // max_clients)`，
  队列满时显式返回背压错误；断开时该 peer 的未发送数据直接丢弃；
- server worker 独占 listener、accepted client、pending write 和 wake socket 的 I/O 与最终关闭权。
  外部关闭只设置停止标志并调用 `wake()`，worker 醒来后退出并在 finally 中关闭句柄；
- accepted socket 始终 non-blocking。worker 的 selector 同时观察读就绪和有 pending write 的写就绪，
  partial write 只推进 offset；完整写入后才发布 `StreamDataSentEvent` 和 TX 原始记录，失败只生成
  对应 peer 的错误/断开事件；读写轮询采用有界 round-robin；
- Windows listener 不设置 `SO_REUSEADDR`，避免不同进程共享或抢占同一监听端口；快速重启遇到地址占用时
  展示可行动错误。

## 保持不变的边界

- 默认只绑定 `127.0.0.1`；非回环绑定仍必须同时确认 LAN 并提供非空 IPv4/CIDR allowlist；
- 当前只支持 IPv4 literal/CIDR，不做 hostname、IPv6、TLS、认证和 NAT 穿透；
- reject/busy 不进入终端、发送队列或原始记录；一个 peer 的 EOF/写失败不结束 listener 或其他 peer；
- UI 只消费 peer snapshot 和 immutable event，不接触 socket；target combo 是发送目标的唯一 UI 入口。

## 验收

- 两个 localhost client 具有不同 `PeerId`，可分别 RX/TX；第三个 client 得到 busy；
- 断开后旧 `PeerId` 发送失败，重新连接不能复用旧 ID；其他 peer 继续收发；
- 多 peer 时 UI 不隐式选择目标；已显式选择的 peer 断开后不自动切换；
- non-blocking partial write、worker wake/shutdown、窗口关闭均在有界时间内完成；
- `ruff`、format、compile、loopback、Qt offscreen 和重建后的 onedir/onefile 包均有交付记录。

## 后续明确不做

高吞吐 IOCP/asyncio、广播/组播、TLS/认证、IPv6、自动踢除策略、优雅 drain、应用层序列号、
丢包/乱序统计和持久化出站队列留给后续独立设计。
