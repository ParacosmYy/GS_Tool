# ADR 0004：M2b TCP Server 单客户端

状态：已接受（M2b 历史基线，2026-08-09；容量与 peer 语义由 ADR 0005 扩展）

## 决策

M2b 增加独立 TCP Server 监听能力，但不把 listener 塞进 M2a 的
`SessionManager` 或 TCP Client adapter：

- 新增 server 专用 session manager 和 acceptor/transport port；
- 一个 listener 最多维护一个活动 client；client EOF 后 listener 继续等待下一个；
- 第二个 client 在有活动 client 时立即关闭并发布 `busy` 诊断，不能进入终端、发送队列或原始记录；
- 未授权来源立即关闭并发布 peer/reason，不能影响 listener 或当前 client；
- listener、client 和取消唤醒使用同一个后台 worker 的有界生命周期；关闭必须能唤醒
  `accept()`，不依赖长 accept timeout；
- 终端、暂停预览、原始 JSONL、历史和快捷命令沿用 M2a；发送目标固定为当前 client，
  没有 client 时发送失败且不入队。

## 网络安全门

- M2b 先只支持 IPv4 literal bind 和 IPv4 literal/CIDR allowlist；hostname 与 IPv6
  暂不接受，避免 DNS 多地址和地址族混淆；
- 默认 bind 为 `127.0.0.1`，用户必须显式点击开始监听；
- 非 loopback bind（包括 `0.0.0.0`）必须同时满足 `lan_confirmed=true` 和非空
  allowlist；校验失败发生在 bind 前，不产生监听 socket；
- allowlist 只按来源 IP 匹配，不包含 ephemeral source port；plain IP 规范化为 `/32`；
- 最大 client 数固定为 1，不提供广播、逐 peer 队列、自动重连或 M2c peer 列表。

## 状态与事件

listener 使用现有 session 生命周期表示启动/停止；server client 使用独立的
connected/disconnected 事件表达第二层状态：

```text
已停止 -> 启动中 -> 监听中
                       ├─ client connected -> 已接入 peer
                       ├─ client EOF       -> 监听中
                       ├─ unauthorized     -> 监听中 + 可见拒绝原因
                       └─ busy             -> 监听中 + 可见 busy 原因
监听中 -> 停止中 -> 已停止
```

监听 bind 失败、参数失败和关闭失败可以结束 server session；单个 client 断开、
非法来源和 busy 不得结束 listener。

## 验收

- loopback listener 只绑定 `127.0.0.1`，首个 client 可以收发并在 JSONL RX/TX 中携带 peer；
- 第二个 loopback client 在有界时间内被关闭并产生 busy 诊断，首个 client 仍能收发；
- 首个 client 断开后状态回到监听中，新的允许 client 可以接入；
- 非 loopback 未确认、allowlist 为空或 allowlist 语法非法时 bind 前失败，系统没有 listener；
- 手动停止/窗口关闭能释放 listener、client、wake socket 和 worker，无 shutdown timeout；
- 不运行真实 LAN、防火墙、IPv6 或硬件验证时，必须在交付记录中明确列为未运行。
