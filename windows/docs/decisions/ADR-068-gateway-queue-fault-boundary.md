# ADR-068：Gateway 持久队列运行时故障边界

**作者：** AI Token Tracker Engineering Team  
**状态：** accepted  
**日期：** 2026-08-10  
**范围：** Windows Local Gateway / UsageReporter / DPAPI SQLite Queue

## 问题

Gateway 启动时会校验 DPAPI 队列，但进程运行期间仍可能出现 SQLite 锁、文件损坏或
DPAPI 解密失败。若这些异常从 daemon worker 逃逸，线程会静默退出；若从同步请求逃逸，
会把 Usage Ingest 故障扩大为未封装的 500，并且调用方无法知道本地记录是否安全保留。

## 决策

- `UsageReporter._enqueue` 捕获 `QueueProtectionError` 与 `QueueStorageError`，返回既有
  `report-failed`，不把异常透传到 Gateway 路由。
- daemon worker 对同一类故障设置进程内 `queue_faulted` 熔断状态并停止继续处理；当前
  队列行不删除，交给下一次进程启动的 fail-closed 校验处理。
- 故障只写入不含 payload、Key、token 或请求正文的结构化 logger 记录；后续报告仍可直接
  成功投递，只有需要写入已故障持久队列时才明确返回 `report-failed`。
- 内存队列路径不改变既有有界重试语义；该边界只保护持久化队列的运行时存储故障。

## 验证与边界

- 已完成 Python 编译、发布审计和 Gateway 源码边界复核；队列数据不因 worker 异常被主动删除。
- 本地无外部服务的内存 reporter smoke 返回 `queued`，关闭后仍保留 1 条有界待投递项，既有
  内存重试路径保持可用；该 smoke 不代表真实中心或 Provider 联调。
- 真实 Provider Key、真实中心短暂故障和正式 Windows DPAPI 现场恢复仍待部署环境联调，不能
  用本地无密钥 smoke 冒充完成证据。
