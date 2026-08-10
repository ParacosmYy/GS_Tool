# ADR-075：Gateway 中心服务只读自动发现

**作者：** AI Token Tracker Engineering Team  
**维护者：** ARCH-1 / ARCH-2 / DEV-4  
**状态：** Accepted  
**日期：** 2026-08-10

## 背景

根目录体验入口在默认端口被占用时，会把当前源码服务移动到 `5001–5020` 的空闲端口；旧的
Gateway 快捷入口却把 Usage Ingest 地址固定为 `127.0.0.1:5000`。当旧实例仍然健康时，Gateway
可能把新的调用记录投递到错误的数据库，形成难以察觉的跨实例数据分流。

## 决策

1. 新增 `deployment/resolve-center-url.ps1`，按 `5000–5020` 顺序只读请求公开的
   `/api/v1/ready`，只接受 HTTP 200 且 JSON `status=ready` 的实例。
2. `start-gateway.ps1` 在未显式传入 `-IngestUrl` 时调用该探测器；显式地址仍保留，便于 HTTPS
   中心、局域网预览和正式部署使用。
3. 探测器不读取 Cookie、Key、数据库和响应业务数据，不启动/停止进程，不修改防火墙或环境变量；
   HTTP 上报仍必须由 `-AllowHttp` 明确授权，HTTPS 地址不需要该开关。
4. 发现范围只覆盖个人体验入口的本机端口窗口，不把该机制当作公网服务发现或负载均衡；正式部署
   仍必须通过 Caddy/域名/HTTPS 显式配置。
5. 探测先用 150ms 上限的 TCP 异步连接过滤闭合端口，再执行 1 秒上限的 HTTP ready 请求，避免无
   服务时逐端口等待长超时拖慢用户启动。

## 后果与回滚

- 用户双击 Gateway 快捷入口时，不再需要手工同步根入口的动态端口。
- 若没有就绪实例，脚本 fail-closed 并给出端口范围，不会把 usage 发送到猜测的地址。
- 回滚只需恢复 `start-gateway.ps1` 的显式 `-IngestUrl` 默认值；不改变 Gateway API、Usage Ingest
  契约、队列加密或数据库 schema。

## 验证边界

- 在隔离 SQLite 的临时源码实例 `127.0.0.1:5001` 上只读探测成功，输出
  `http://127.0.0.1:5001/api/v1/ingest/usage`；旧的 `5000` 实例未提供 ready 时不会被误选。
- TCP 过滤后，无就绪服务的单端口窗口以非零退出码快速 fail-closed；不会创建重试任务或后台线程。
- 使用假 provider/ingest 字符串启动 loopback Gateway `127.0.0.1:8789`，日志确认自动发现上述中心，
  Gateway `/health` 返回 HTTP 200；未发起任何上游模型请求。
- 探测过程不写入数据库、不创建测试账户、不接触真实 provider Key。
- PowerShell 语法、启动入口、发布审计和文件行数门禁必须通过。
- 真实 provider 联调、正式 HTTPS 服务发现和跨主机部署仍属于外部发布门禁。
