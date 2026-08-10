# ADR-056：Gateway 跨重启加密重试队列

## 状态

已接受（2026-08-10）。

## 问题

本地 Gateway 已能在进程存活期间重试中心 Usage Ingest，但中心短暂不可用时如果进程重启，
内存队列会丢失。直接把 token、prompt、provider response 或普通 JSON 写入 SQLite 会扩大
凭据和隐私风险，也会让 Reporter 与存储实现发生耦合。

## 决策

- Windows Gateway 默认使用项目 `data/gateway-usage-queue.sqlite3`，该路径由 Git 忽略规则保护；
  队列只保存有界 `UsageReport` DTO 的 Windows DPAPI 密文、尝试次数和下一次重试时间。
- DPAPI 绑定当前 Windows 用户和机器环境；启动时会校验 schema、解密已有 payload 并在无法恢复时
  fail closed，绝不退回明文存储。用户切换账户、迁移到不兼容机器或修改文件后需要重新配置，
  不静默丢弃或猜测数据。
- `gateway_reporting.py` 只负责同步投递、退避和 worker 调度；`gateway_queue.py` 负责 SQLite
  事务和 DPAPI；`gateway_contracts.py` 负责跨模块 DTO。Gateway 路由不直接导入 SQL 或加密细节。
- 队列容量、尝试次数、payload 大小和路径均有界；达到容量或最大尝试次数返回/记录明确的
  `report-failed` 语义，不无限增长。原幂等键在恢复后保持不变，中心端仍负责幂等去重。
- `--memory-only` 仅作为显式临时调试选项；生产/分享路径使用默认加密队列。非 Windows 平台
  没有 DPAPI 时拒绝持久化队列启动，避免把“加密”降级成伪加密。

## 不变量

- provider Key、中心 ingest token、prompt、完整 provider response 不进入队列 payload 或 SQLite。
- SQLite 原始文件中不出现 model、token 数、备注或幂等键明文；只有 DPAPI 密文和调度元数据。
- 进程重启后同一幂等键可以继续投递；成功或明确终态后删除对应密文行。
- 队列路径不从客户端请求读取，CLI 不接受任何 secret 参数，日志不打印 secret 或 payload。

## 验证边界

- 已完成：Windows DPAPI 密文写入、同用户跨实例恢复、原始 SQLite 不含 usage DTO 明文、队列
  容量/删除/重排和 Reporter 重启 smoke。
- 待完成：真实中心短暂故障与合法 provider Key 的一次非流式/流式联调；不使用演示 Key 冒充真实
  provider 证据。
