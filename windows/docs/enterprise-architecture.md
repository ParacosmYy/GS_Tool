# 企业级架构基线

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**状态：** Accepted  
**版本：** 1.0  
**日期：** 2026-08-10

## 1. 目标与范围

本项目采用“模块化单体优先、可拆分演进”的企业架构。当前业务规模不需要为了形式引入微服务、消息队列和 Kubernetes；但所有模块必须通过清晰接口隔离，使未来拆成 Go API、Vue 3 Web 和独立采集器时不重写业务规则。

这里使用的是公开工程原则，不声称复制任何公司的内部规范。参考公开的 CloudWeGo 企业云原生中间件方向和 ByteDance 开源项目所体现的模块化/可扩展思路，项目自身约束以本文件、根目录 `AGENTS.md`、API 契约和 ADR 为准。

## 2. 逻辑分层

```text
Browser / Android / CLI
          │
          ▼
Presentation: web.py, api_v1.py, cli.py
          │  stable DTO / error envelope / auth decorators
          ▼
Application: services.py, events.py, admin_data.py, mobile_auth.py
          │  use cases, validation, idempotency, RBAC decisions
          ▼
Domain ports: api_contract.py, provider adapter interfaces
          │
          ├──────────────► Provider boundary: providers.py
          ▼
Infrastructure: db.py / SQLite / local files
```

Android 具体实现保持同一方向：`feature/*` 只渲染 ViewModel 状态，`TrackerRepository` 负责会话与重试，`data/remote` 通过 `TrackerRemoteDataSource` 适配 `/api/v1`，`data/secure` 负责 Keystore 加密会话。这样 Android 可以替换 HTTP 库而不让网络细节进入 Compose，也不让客户端直接触碰 Windows SQLite。

依赖只能从上到下。`db.py` 不导入 Flask；`providers.py` 不读取 session；模板和 Android 不猜测数据库字段；路由不复制 SQL 或业务校验。

## 3. 模块边界

| 模块 | 负责 | 禁止 |
|---|---|---|
| Presentation | 路由、解析请求、状态码、模板和响应编排 | 直接拼接 SQL、决定跨用户权限、持久化 Key |
| Application | 用例校验、角色判定、事件幂等、token 生命周期、限流策略、管理员审计编排 | 依赖 DOM、读取 Flask 模板状态 |
| Domain ports | 稳定 DTO、错误码、provider 能力接口 | 包含具体 HTTP 请求或页面逻辑 |
| Provider boundary | URL 白名单、HTTPS、超时、usage 解析、上游错误归一化 | 写入密码、Key 或未经筛选的上游响应 |
| Infrastructure | 参数化 SQL、schema 迁移、索引、CSV 数据投影 | 依赖 request/session 或向 UI 返回敏感列 |

## 4. 数据边界

- `users` 只保存账户、密码哈希、角色和创建时间。
- `usage_records` 只保存 token 事实；不承载效率判断、原始 prompt 或管理员审计。
- `work_events` 保存方向、结果、时长、可选效率评分和正确/错误码；效率评分不是绩效结论。
- `app_logs` 保存脱敏排障信息；消息和 metadata 有长度上限。
- `audit_events` 保存管理员和安全动作；查看、导出、角色变更必须写入。
- `auth_tokens` 只保存 token digest；access/refresh 原文只在 HTTPS 响应中出现一次。
- `rate_limit_buckets` 只保存哈希后的限流 key、窗口时间戳和清理时间；它不进入业务汇总、CSV 导出或管理员活动读模型。

## 5. 跨端契约

- 网页旧 `/api/*` 继续兼容，新的 Android/采集器使用 `/api/v1/*`。
- 所有 v1 响应使用稳定错误 envelope 和 `request_id`。
- 离线事件用 `protocol_version=1`、有限 `command`、`request_id`、`idempotency_key`；不允许客户端传函数名或 SQL。
- 破坏性变更创建新主版本；新增字段默认可忽略；旧客户端有迁移窗口。

## 6. 可观测性和质量闸门

- 每个 HTTP 请求生成 correlation/request id；provider 错误不泄露 traceback、请求头或 Key。
- provider 输入还必须通过字段级预算：Key 4096、Base URL 2048、模型名 200、消息 100 项。
- API 404/405/413/500 统一走 JSON error envelope；`request_ids.py` 只负责关联 ID，不承担身份或授权。
- 共享/生产模式缺少安全 session secret 时必须 fail-closed；本机模式的临时随机 key 不得被当作生产配置。
- `deployment_checks.py` 只读验证 shared/production/lan 的 Secret 和数据库路径，并在 production 额外验证 Secure Cookie 与 provider HTTPS allowlist；`serve --production` 和 `serve --lan-preview` 都在应用创建和端口监听前强制调用它，前者不允许绕过 HTTPS 门禁，后者明确限定为可信 HTTP 预览。
- `schema.py` 集中 SQLite DDL 和加法式迁移；`db.py` 集中连接、事务和查询，不再混合 schema 生命周期。
- 限流策略由 `rate_limit.py` 提供单一接口；local 使用内存滑动窗口，shared/production/lan 使用 SQLite 共享窗口并哈希存储 key，后续高并发迁移 PostgreSQL/Redis 时只替换该基础设施适配器。
- 应用访问日志由 `access_logging.py` 执行字段白名单；Caddy 边缘日志使用显式 filter 删除凭据、Cookie、查询参数和精确客户端地址，正式部署仍需现场验证。
- `events.py` 在持久化前脱敏结构化日志并保持合法有界 JSON；认证、Provider 和个人数据响应统一禁止缓存。
- 结构化应用日志、业务工作事件和安全审计事件分开保留。
- 管理员 CSV 使用固定列和游标逐行读取，单次最多 100,000 行/16 MiB；超限返回 `413 EXPORT_TOO_LARGE`，不返回部分文件。
- `admin.export` 只记录成功导出；因行数/字节边界拒绝时记录 `admin.export.rejected`，避免安全失败不可追溯。
- 每个文件最多 1000 行；接近 800 行必须拆分成高内聚接口。
- 每个源文件有统一元信息，公共函数/路由/DTO 有输入、输出、权限、失败语义和不变量说明。
- UI 变更必须有真实浏览器截图、DOM/ARIA、控制台、网络和四档响应式证据。
- Android 没有 JDK/Gradle/SDK 时只报告静态配置，不能把骨架当成 APK 构建成功。
- Windows 运行时以 `requirements.lock` 作为可复现安装边界；可选构建工具不混入运行时锁，升级必须独立审查。

## 7. Go/Vue 迁移原则

当前 Python/Flask 模块化单体先作为可运行基线。未来迁移时：

1. 先冻结 `/api/v1` 和错误码，不先重写数据库。
2. Vue 3 只替换 Presentation 层，复用 JSON 契约和权限语义。
3. Go 先承接 API gateway/高并发 provider proxy，再逐步迁移应用用例。
4. SQLite 迁移到 PostgreSQL 前先完成 schema、备份、并发和回滚演练；不因为“企业级”四个字直接切库。
