# AI Token Tracker 架构说明

## 目标

这是一个模块化单体产品：个人可以双击根目录的 `start.bat` 立即体验；团队共享时，多个用户通过同一个 Windows 中心网站登录使用，管理员在服务端查看聚合和审计。数据默认留在中心 SQLite，不依赖云数据库；未来可以保留 `/api/v1` 迁移到 Go/Vue 或 PostgreSQL。

## 当前部署形态

当前是“前后端代码分离、分层模块化单进程部署”，不是两个独立服务：

```text
┌──────────────────────────────┐
│ Browser                      │
│ templates + static/app.js    │
└──────────────┬───────────────┘
               │ same-origin JSON API + CSRF
               ▼
┌──────────────────────────────┐
│ Flask / web.py + api_v1.py  │  路由、会话、版本 API、安全响应头
└──────────────┬───────────────┘
               ▼
┌──────────────────────────────┐
│ application modules          │  services/events/admin/mobile_auth
└──────────────┬───────────────┘
               ▼
┌──────────────────────────────┐
│ db.py + provider boundary    │  参数化 SQL、provider 隔离、CSV
└──────────────┬───────────────┘
               ▼
       data/token_tracker.sqlite3

providers.py ── adapter registry ── one request ── allowlisted provider /chat/completions or /models
```

### 为什么先不拆成两个服务器

当前用户量、页面数量和数据量都很小。模块化单体可以减少 CORS、跨域会话、反向代理和部署配置，个人体验更稳定；它不是把所有逻辑塞在一个文件，而是用分层模块和契约隔离。未来如果出现独立部署需求，可以只替换 Presentation/Infrastructure，不必重写业务用例。

## 目录职责

| 路径 | 责任 | 不负责什么 |
|---|---|---|
| `run.py` / `start.bat` | 本机一键体验；默认 5000 被占用时无副作用回退到可用端口 | 不承载业务逻辑、不终止已有进程 |
| `token_tracker/cli.py` | CLI 参数和终端输出 | 不实现 SQL 统计细节 |
| `token_tracker/web.py` | HTTP 路由、登录会话、CSRF、安全头 | 不直接拼接业务 SQL、上游 URL 或 provider HTTP |
| `token_tracker/api_v1.py` | Android/采集器版本 API、bearer auth、管理员路由编排 | 不保存 token 原文、不绕过 application/repository 边界 |
| `token_tracker/auth_service.py` | 账号注册、用户名/密码校验和密码哈希 | 不签发/保存 bearer token，不读取 HTTP request |
| `token_tracker/settings.py` | 环境变量、白名单、Cookie 和代理配置 | 不读取 request，不决定用户权限 |
| `token_tracker/providers.py` | 白名单 URL、adapter 选择、模型发现、OpenAI-compatible 请求和 usage 解析 | 不管理用户会话或写 SQLite |
| `token_tracker/provider_projection.py` | 将不可信 provider 响应投影为有界客户端字段 | 不保存原始 provider JSON，不决定授权或 token 统计 |
| `token_tracker/services.py` | 记录校验、日期范围转换 | 不渲染 HTML |
| `token_tracker/events.py` | work event、app log、audit event 校验和写入 | 不接受原始 prompt、Key 或任意 JSON SQL |
| `token_tracker/admin_service.py` | 管理员页面、读模型、导出和审计用例编排 | 不绕过管理员装饰器或返回敏感字段 |
| `token_tracker/admin_data.py` | 管理员聚合、成员明细、固定列有界游标导出 | 不返回密码哈希、令牌或未经脱敏的秘密；不使用无界导出缓冲 |
| `token_tracker/csv_export.py` | 个人/管理员固定列 CSV 的统一有界序列化 | 不拼接 SQL、不读取 request/session、不改变业务列投影 |
| `token_tracker/readiness.py` | liveness/readiness 之外的 SQLite 核心 schema 探针 | 不返回路径、表名、异常或业务数据 |
| `token_tracker/mobile_auth.py` | access/refresh token digest、轮换和撤销 | 不把 bearer secret 写入数据库 |
| `token_tracker/ingest_auth.py` | 外部用量采集 token 的签发、摘要解析、过期和撤销边界 | 不保存 provider Key、原始 prompt 或 token 原文 |
| `token_tracker/gateway.py` | 本地 OpenAI-compatible `/v1/models`、chat JSON/SSE 转发和 usage 上报 | 不写中心 SQLite，不接受客户端 base URL/API Key，不承担 Anthropic/Responses 私有协议 |
| `token_tracker/gateway_reporting.py` | Gateway Usage Ingest 同步投递、退避和 worker 调度 | 不保存 provider Key、prompt 或 SQL 细节，不改变中心入账契约 |
| `token_tracker/gateway_contracts.py` | Gateway 内部 UsageReport DTO 与序列化投影 | 不读取 HTTP、环境变量、SQLite 或 provider response |
| `token_tracker/gateway_queue.py` | Windows DPAPI 密文队列、SQLite 事务和跨重启恢复 | 不接收 provider Key/prompt，不承担 HTTP 投递或页面会话 |
| `token_tracker/schema.py` | SQLite DDL、索引和加法式兼容迁移 | 不读取 request/session，不组合业务查询 |
| `token_tracker/db.py` | SQLite 连接、事务、参数化查询和 CSV | 不处理 HTTP 请求 |
| `token_tracker/rate_limit.py` | 限流策略、内存/SQLite 状态适配和哈希 key | 不读取 Flask session，不保存原始 IP、用户 ID 或 provider Key |
| `token_tracker/access_logging.py` | 应用访问日志的字段白名单、耗时和单行/长度边界 | 不读取 query、Header、Cookie、body、IP 或 provider Key |
| `templates/` | 页面结构和语义 | 不保存状态或直接访问 DB |
| `static/app.js` | 仪表盘编排、表单状态和 API 组合 | 不保存 API Key、不直接绘图实现 |
| `static/modules/api-client.js` | CSRF、同源 fetch、统一错误解析 | 不管理页面业务状态 |
| `static/modules/charts.js` | Chart.js 趋势和模型占比渲染 | 不发起 API 请求、不写数据库 |
| `static/modules/motion.js` | motion state、reveal、count-up、pointer feedback | 不改变业务数据 |
| `static/style.css` | UI token、布局、响应式、动效 | 不包含业务判断 |
| `docs/` | 决策、契约、设计约束 | 不替代代码注释 |

## 数据模型

### `users`

- `id`: SQLite 主键。
- `username`: 唯一用户名。
- `password_hash`: Werkzeug scrypt 哈希，不保存明文密码。
- `created_at`: 本地墙上时间。
- `role`: `user` 或 `admin`；权限由服务端强制，不由模板决定。

### `usage_records`

- `id`: SQLite 主键。
- `user_id`: 所属用户；所有查询必须带用户条件。
- `model`: 模型名。
- `input_tokens` / `output_tokens`: 非负整数。
- `timestamp`: 本地时间文本 `YYYY-MM-DD HH:MM:SS`。
- `note`: 用户备注。
- `source`: `manual`、`proxy`、`android` 或 `ingest`，便于区分来源。
- `idempotency_key`: 可空的移动端重试键，只参与同用户去重，不进入公开投影。

### `auth_tokens`

只保存 access/refresh token 的 SHA-256 digest、类型、过期和撤销时间；原文只在 HTTPS token exchange 响应中出现。

### `usage_ingest_tokens`

只保存外部采集 token 的 SHA-256 digest、所属用户、标签、到期和撤销时间。原文由 CLI 创建时只显示
一次；该表只授权用量事实入口，不授权读取 provider Key、用户密码或其他用户数据。

### `rate_limit_buckets`

只保存哈希后的限流 key、窗口时间戳和清理时间；它是基础设施状态，不参与业务统计和导出。

### `work_events` / `app_logs` / `audit_events`

分别记录方向/效率/结果、脱敏排障日志和管理员安全动作。事件同步可携带 v1 command envelope 与幂等键；token 写入使用同用户唯一重试键，防止 Android 重放造成重复计数。

## 请求流

### 自动采集（主流程）

1. 页面用内存中的 API Key 请求 `POST /api/provider/models`，adapter 自动读取上游 `/models`，把可用模型填入选择列表。
2. 页面将 prompt 发到 `POST /api/proxy/chat/completions`；用户不填写输入/输出 token。
3. `providers.py` 读取响应 `usage`，以响应中的真实 `model` 优先归档，缺失时才回退到请求模型。
4. `services.add_usage` 校验数字并写入 `db.insert_record`。

### 客户端补录

Android 通过 bearer 认证的 `POST /api/v1/records` 写入手动 token 记录，服务端
固定 `source=android` 并在成功后由客户端重新读取汇总。网页旧接口
`POST /api/records` 继续作为会话兼容入口，用于 provider 没有返回 usage 或导入
历史账单的 fallback；两者都复用 `services.add_usage`，不复制校验规则。

### 自动代理记录

1. 浏览器把 `base_url`、页面会话内存中的 `api_key`、模型和消息发送给同源 Flask。
2. 服务端校验 HTTPS、白名单、请求大小、用户会话和限流。
3. `providers.py` 向上游发送一次非流式请求，禁止自动跟随重定向。
4. 只从上游 `usage.prompt_tokens` / `completion_tokens`（或兼容字段）取数字。
5. 成功解析后写入 `usage_records`；API Key 不入库、不写日志、不回传浏览器。

### 外部客户端用量采集

1. 管理员/账户持有人通过 CLI 为指定账户创建有期限的 Usage Ingest Token；服务端只写入摘要。
2. 外部 wrapper、SDK adapter 或中心 Gateway 从自己已经拿到的 provider `usage` 中提取模型、输入和输出
   token，通过 `POST /api/v1/ingest/usage` 上报；服务端不接收 provider Key 或原始 prompt。
3. `ingest_auth.py` 通过 token digest 解析用户归属，接口固定 `source=ingest`，并使用用户级幂等键避免重试重复计数。
4. token 支持到期与撤销，接口独立于浏览器 Cookie/CSRF；HTTPS、限流、字段预算和统一错误 envelope 仍由中心服务强制。
5. 该边界不会观察 Kimi Code 等其他进程的网络流量；现有 stock 客户端仍需后续 wrapper/Gateway 适配才能实现自动上报。

### 本地 Gateway 自动采集

1. Gateway 作为独立本地进程启动，默认只绑定 loopback；provider Key、中心 ingest token 和可选
   Gateway 访问令牌来自环境变量，不进入命令行参数或中心数据库。
2. Kimi Code/OpenAI-compatible 客户端把 Base URL 改为 Gateway 的 `/v1` 地址；Gateway 忽略
   客户端提供的 upstream 地址和 Key，使用启动配置的 allowlisted upstream 转发 `/models` 与
   `/chat/completions`。
3. 非流式响应直接返回有界的兼容 JSON；流式响应按 SSE 原样转发并在完成 chunk 解析 usage。
4. 有效 usage 通过 `POST /api/v1/ingest/usage` 上报，使用客户端幂等键或 Gateway 生成的请求键；
   上报失败不伪造 token，也不回滚已经完成的 provider 调用。
5. Reporter 默认把短暂失败放入 Windows DPAPI 密文队列，使用同一幂等键跨重启恢复；容量、
   尝试次数和 payload 均有界，`--memory-only` 只能显式用于临时调试。
6. Gateway 与中心服务是两个可独立替换的进程边界；Anthropic/Responses adapter 应作为独立
   切片实现，不在 `web.py` 增加厂商分支。

Android 复用同一 application/provider service，但入口是 bearer 版本的
`/api/v1/provider/models` 与 `/api/v1/proxy/chat/completions`。`data/remote`
只解析助手文本、usage 摘要和记录投影，Compose 不接触原始 provider JSON、SQLite
或 API Key 持久化；请求携带幂等键，服务端按用户边界自动归档。

## 时间规则

所有时间均使用运行机器本地时间。统计查询使用 `[start, end)` 半开区间：当天起点包含，下一天起点不包含，避免边界记录被重复统计。

## 演进边界

### 企业级版本协议

网页旧接口 `/api/*` 继续兼容；Android 和未来采集器使用 `/api/v1/*`。普通查询使用 REST 资源，离线事件才使用有限 `command` 信封。详细字段、状态码和权限见 `docs/api-contract.md` 与 ADR-007/008。

### 文件规模门禁

每个源文件最多 1000 行，接近上限就按职责拆分。拆分后的模块通过显式导出、类型/数据契约或 HTTP API 暴露能力，保持高内聚、低耦合。路由文件只做编排，统计/权限/日志查询分别进入独立服务或数据访问模块；前端页面编排、API client、图表、motion 和组件样式不互相偷渡职责。

- 新 provider：先增加 `ProviderAdapter` 实现并明确兼容契约，不在路由里堆厂商分支；OpenAI-compatible 的公共逻辑集中在 `providers.py`。
- 页面 Key 只能在内存中复用；持久化 Key 或外部 gateway 必须新增认证/密钥隔离 ADR。
- 外部客户端自动采集使用本地 Gateway + `POST /api/v1/ingest/usage` 的 per-user token 边界；stock 客户端不能被网页凭空观察，Gateway 演进必须遵守 ADR-004、ADR-053 与 ADR-054。
- 新统计：进入对应 application/read-model 模块；个人 token 查询留在 `db.py`，管理员聚合进入 `admin_data.py`，避免跨角色 SQL 混在一起。
- 前后端拆分：先保留本文档中的 API 契约，新增 CORS、跨域 CSRF 和独立会话方案后再拆服务。
- 生产共享：使用 HTTPS 反向代理、Waitress/WSGI 服务、持久化数据卷和环境变量；不要使用 `debug=True`。
- 易部署：个人用根目录 `start.bat` 或 `windows/packaging/build.ps1` 生成 EXE；团队用一个 HTTPS 中心网站，不能把独立 EXE 分发给同学后再期待数据自动汇总。
