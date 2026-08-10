# AI Token Tracker 企业级交付计划

> 当前主线已升级为中心化多用户平台：`windows/` 是服务与数据库宿主，`android/` 是同账号移动客户端。下方历史阶段仍保留作为已交付基线；新增工作以本节和根目录 `AGENTS.md` 为准。

## Current Platform Expansion

### Long-term outcome

- 所有同学通过同一个 Windows 中心服务登录，数据集中存储在服务主机并按用户隔离。
- 管理员拥有服务端 `admin` 角色，可查看团队汇总、授权明细、结构化事件和审计导出。
- 网页与 Android APK 共用 `/api/v1` 契约和账号，不共享 SQLite 文件。
- token、AI 使用方向、工作效率、正确/错误结果和 log 各自建模，不能混成一张记录表。
- UI 继续以五个模块逐项交付，世界级动效目标必须同时通过可读性、性能和 reduced-motion 门禁。

### Seven role slots

| 槽位 | 责任 |
|---|---|
| UI-1 | 唯一 UI 视觉与动效总监 |
| DEV-1 | Shell / Auth |
| DEV-2 | Observatory / Analysis |
| DEV-3 | Connect / Frontend State |
| DEV-4 | History / Runtime |
| ARCH-1 | 系统架构、API、数据与安全边界 |
| ARCH-2 | 交付集成、性能、可访问性与证据闸门 |

### Delivery slices

1. **Slice A — 双端目录与环境**：已完成。`windows/`、`android/`、根目录启动器、Android Studio/Gradle/JDK 说明和文件规模/注释硬门禁已落盘。
2. **Slice B — 中心化数据与 RBAC**：已完成 Windows 首切片。用户角色、管理员概览、用户明细、审计事件和安全导出已接入。
3. **Slice C — 结构化工作事件**：已完成 Windows 首切片。AI 使用方向、任务效率、正确/错误码、请求 ID 和客户端日志模型已接入。
4. **Slice D — `/api/v1` 同账号**：Windows 与 Android 已完成登录、会话续期、分页、错误 envelope、离线安全策略和 token 记录写入源代码链路；APK 编译待工具链门禁。
5. **Slice E — Android 首个可用闭环**：登录 → 个人仪表盘 → token/事件/日志写入 → 同步 → 管理员只读观测台已完成源码闭环。
6. **Slice F — UI 动效五模块**：Shell、Auth、Observatory、Connect、History 逐个浏览器/设备验收。

每个 slice 都必须留下：变更文件、接口契约、角色评审、行数扫描、非破坏性运行证据和未完成风险。

## Completed Slice — Android admin read-only linkage

### Scope

Extend the existing `/api/v1` Android vertical slice so an account with
`user.role == "admin"` can read the same team overview, member list, and
selected member activity already protected by Windows RBAC. Ordinary users
must remain on the personal dashboard; the Android client must not receive or
cache password hashes, bearer tokens outside the encrypted session store, or
unbounded member activity.

### Ordered tasks

1. **Contract mapping** — document the existing response shapes and create
   Android domain/remote types without changing the server contract.
2. **Remote boundary** — add bounded, bearer-authenticated admin reads to the
   repository adapter with structured error handling.
3. **ViewModel state** — load admin data only after the authenticated profile
   proves the `admin` role; preserve personal-user behavior.
4. **Compose presentation** — add a focused admin screen with team metrics,
   paged member list, selected-member activity, loading, empty, and error
   states. Keep each file below the 1000-line project gate.
5. **Evidence gate** — run Windows compile/help/health checks, inspect API
   responses with a temporary local smoke account, scan source sizes, and
   record the Android toolchain blocker without downloading it.

### Acceptance criteria

- [x] An admin login can reach a read-only team overview through `/api/v1`.
- [x] A member can be selected and its bounded usage/events/logs are shown.
- [x] A normal user never sees or can request admin data from the Android UI.
- [x] No new dependency or external toolchain is downloaded without approval.
- [x] API documentation, ADRs, comments, and verification evidence are updated.

源码与 Windows 服务证据已满足本切片验收；Android APK 编译、安装和设备联调仍属于下一步工具链门禁，不在未批准时伪造完成。

## Current Active Slice — Android token record write linkage

### Scope

Expose the existing usage validation service through the stable `/api/v1/records`
bearer contract and let ordinary Android users enter model/input/output tokens.
The server remains the source of truth for account ownership, local timestamp
normalization, validation, and aggregate refresh.

### Acceptance criteria

- [x] `POST /api/v1/records` is authenticated and writes under `g.user`.
- [x] `GET /api/v1/records` provides bounded, paged personal records with local-time ranges.
- [x] The Android remote, repository, ViewModel, and dashboard form share one draft model.
- [x] Android refresh reads the bounded personal records page and renders it in the dashboard history card.
- [x] The client cannot override the server-assigned `source` value.
- [x] Android token writes carry a user-scoped idempotency key and replay the original record safely.
- [x] Successful write triggers a fresh `/me/summary` read instead of client-side aggregation.
- [x] ADR, API contract, README, comments, and no-download verification remain synchronized.
- [ ] APK compilation and device linkage are still pending approved Android tooling.

## Current Sharing Readiness — local backup gate

- [x] 提供 `python -m token_tracker backup`，使用 SQLite online backup API 生成时间戳副本。
- [x] 备份发布前执行 `PRAGMA integrity_check`，使用临时文件和原子替换避免暴露半成品。
- [x] 默认备份目录固定在数据库旁的 `backups/`，不进入网站静态目录，不上传云端。
- [x] 当前 schema 必要表包含 `usage_ingest_tokens`，同一秒重复备份使用唯一后缀避免覆盖（ADR-060）。
- [ ] 正式部署仍需管理员按策略执行恢复演练、HTTPS、限流和访问日志脱敏。

## Next Iteration — Application boundary alignment

本阶段把文档声明的 `Presentation → Application → Infrastructure` 依赖落实到一个
可运行的后端垂直切片，保持现有网页和 Android `/api/v1` 响应兼容。

### Task A1: 建立持续交付规格与架构决策

- [x] 写入 `docs/enterprise-delivery-spec.md`，定义目标、结构、命令、边界和长期验收。
- [x] 写入 ADR-016，记录用量/日志读取从控制器收敛到应用用例的原因和替代方案。
- [x] 更新任务清单，明确当前切片的验收证据和后续依赖。

### Task A2: 收敛个人用量读取用例

- [x] `services.py` 提供汇总、记录分页和 CSV 导出应用函数。
- [x] `web.py` 与 `api_v1.py` 通过应用函数读取，不复制时间范围和分页规则。
- [x] 旧 `/api/*` 与正式 `/api/v1/*` 响应字段保持兼容。

### Task A3: 收敛诊断日志读取用例

- [x] `events.py` 提供带用户边界和上限的日志分页函数。
- [x] `api_v1.py` 不再内嵌日志 SQL。
- [x] 日志 JSON 投影、脱敏边界和分页 metadata 保持契约不变。

### Checkpoint A

- [x] Python 模块编译通过，所有前端脚本语法通过。
- [x] 健康、登录页、静态资源和应用函数的运行 smoke check 通过；认证错误分支仍由既有门禁覆盖。
- [x] 静态检索确认 Presentation 层没有本切片范围内的 SQL。
- [x] 受影响文件均小于 1000 行，ADR、API 契约和 todo 同步。

### Task A4: 集中跨控制器限流状态

- [x] `rate_limit.py` 提供统一接口：local 内存滑动窗口，shared/production/lan SQLite 共享滑动窗口。
- [x] Web 和 `/api/v1` 使用同一 limiter，不再复制锁和 bucket。
- [x] ADR-017、架构说明和高并发迁移替换边界已记录。

### Task A5: 收敛 v1 凭据认证用例

- [x] `mobile_auth.py` 负责用户名边界、哈希校验和统一失败语义。
- [x] `api_v1.py` 只负责限流、调用认证用例和令牌响应，不直接依赖数据库/密码哈希。
- [x] ADR-018 和 `/api/v1/auth/login` 契约已同步。

### Task A6: 分离 SQLite schema 生命周期

- [x] 新增 `schema.py`，集中 DDL、索引和加法式兼容迁移。
- [x] `db.init_db` 通过 schema 接口初始化，既有查询调用方保持不变。
- [x] ADR-019、架构模块表和文件规模门禁已同步。

### Task A7: 统一 v1 错误和请求关联边界

- [x] 新增 `request_ids.py`，校验/生成安全的 `X-Request-ID`。
- [x] API 404/405 与既有 400/413/500 使用统一 JSON error envelope。
- [x] API 契约、架构说明和 ADR-020 已同步，未改变业务响应字段。

### Task A8: 收敛管理员读模型与审计编排

- [x] 新增 `admin_service.py`，统一管理员页面、聚合、明细和导出用例。
- [x] Web 管理员页面和 v1 管理员 API 都经过 Application 审计边界。
- [x] `admin_data.py` 保留脱敏查询/投影职责，ADR-021 和架构模块表已同步。

### Task A9: 统一个人与管理员有界 CSV 出口

- [x] 新增 `csv_export.py`，以游标逐行消费固定列，统一 100,000 行/16 MiB 边界。
- [x] 个人 Web/API/CLI 与管理员 Web/v1 导出复用同一序列化器，不返回部分文件。
- [x] `EXPORT_TOO_LARGE`、拒绝审计、README、API 契约和 ADR-039/040 已同步。

### Task B1: 共享部署 Session Secret 安全门禁

- [x] `settings.py` 区分 local/shared runtime mode，并对共享模式 fail-closed。
- [x] `serve --production` 在创建应用前强制进入 production 模式并执行 HTTPS 预检；可信 HTTP 分享使用独立的 `--lan-preview`/lan 模式。
- [x] `.env.example`、部署说明、架构约束和 ADR-022 已同步，真实密钥未输出或提交。

### Task B2: Provider 输入字段级资源预算

- [x] `providers.py` 对 API Key、Base URL、模型名和消息条数执行统一上限。
- [x] API 契约和错误语义明确超限行为，不记录或返回 Key。
- [x] ADR-023 与 provider 安全边界已同步。

### Task B3: 统一 Web/v1 凭据应用边界

- [x] 新增 `auth_service.py`，统一注册、用户名/密码校验和 scrypt 哈希。
- [x] Web 与 `/api/v1` 复用凭据用例，`mobile_auth.py` 只保留 bearer token 生命周期。
- [x] ADR-018 已标记 superseded，ADR-024 和架构模块表已同步。

### Task B4: 备份只读验证门禁

- [x] `backup.verify_backup` 执行 integrity、foreign key 和必要表检查。
- [x] 新增 `verify-backup` CLI，不修改当前数据库、不覆盖备份、不输出敏感数据。
- [x] ADR-025/060、README 和部署说明已同步；staging 恢复演练保留为后续部署门禁。

### Task B5: 共享/生产部署只读预检

- [x] 新增 `deployment_checks.py` 和 `preflight` CLI。
- [x] production 预检强制 Secret、Secure Cookie、HTTPS provider allowlist 和数据库路径门禁。
- [x] preflight 路径解析与初始化目录创建分离，预检保持无副作用。
- [x] ADR-026、README 和部署说明已同步，不执行联网、部署或防火墙变更。

### Task B6: Android Release 传输安全门禁

- [x] Debug 保留模拟器 HTTP 默认地址，Release Gradle 配置阶段强制 `https://`。
- [x] Release Manifest 继续关闭明文流量，构建命令、工具链门禁和 ADR-041 已同步。
- [x] Release endpoint store、repository 和 HTTP adapter 在请求前共享 HTTPS 门禁，避免用户输入 HTTP 后才由平台失败。
- [>] APK 编译、安装和设备联调仍等待用户批准 JDK/Gradle/SDK 工具链。

### Task B7: 中心服务 liveness/readiness 探针

- [x] 保留 `/api/v1/health` 进程存活语义，新增 `/api/v1/ready` 核心 schema 就绪语义。
- [x] 失败返回 `503 SERVICE_NOT_READY`，不泄露数据库路径、表名、异常或业务数据。
- [x] API 契约、部署检查说明、脱敏访问日志关联和 ADR-042 已同步。

### Task B8: Android 加密会话持久性门禁

- [x] 登录/刷新后的 access/refresh token 加密 blob 使用 worker 线程同步提交，避免进程快速终止造成半持久状态。
- [x] 退出、服务地址切换和 refresh 失效清除同样检查本地提交结果；失败不伪装为成功。
- [x] Android README、架构角色复核和 ADR-043 已同步；APK 构建和设备验证仍等待工具链批准。

### Task B9: Caddy 边缘日志保留策略

- [x] Caddy 示例显式配置每日/100 MiB 滚动、14 文件/14 天保留和本地文件名时间。
- [x] `serve --production` 的命令行/环境绑定地址与预检保持一致，并强制 loopback（ADR-061）。
- [x] Caddy filter、header、reverse proxy 边界与部署说明同步；配置语法依据官方文档记录。
- [>] 正式主机上的 `caddy validate`、ACL、权限和真实轮转样本仍需部署负责人执行。

### Task B10: Web/Android 场景资产一致性

- [x] Web 切换到 `embedded-rust-engineer-bg-v10.png`，保留 v9/v8 及之前版本作为回滚资产。
- [x] Android Compose 切换到同一像素资产的 `embedded_rust_engineer_bg_v10.png`，端侧遮罩和漂移实现不互相耦合。
- [x] 两端资源 SHA-256 一致，资源引用、注释和视觉规范同步。
- [>] Web v8 四档浏览器证据和 Android 编译/设备视觉验收仍分别等待 CDP 与 Android 工具链门禁。

### Task B11: Kimi provider 快速连接预设

- [x] Web 自动采集增加 Kimi Code、Kimi 开放平台、OpenAI 和自定义 OpenAI-compatible 地址预设。
- [x] 预设只填充 URL，不接触 Key；服务端 allowlist、HTTPS、超时、响应大小和 usage 记录边界不变。
- [x] 官方 Kimi Base URL、Key 隔离、协议和错误语义写入 README 与 ADR-048。
- [>] 真实 `/models`/chat usage smoke 仍需用户本人合法 Key，不得在无授权时伪造或读取凭据。

### Task B12: Windows EXE 构建工具链锁定

- [x] PyInstaller 与 Windows 构建依赖拆到 `packaging/requirements-build.lock`，不再使用未锁定安装。
- [x] EXE `--onedir` 资源收集、冻结数据目录和清理路径保护写入脚本与 ADR-049。
- [x] `packaging/toolchain-doctor.ps1` 提供只读构建前检查，明确区分工具缺失 pending 与构建失败。
- [>] 当前环境未安装 PyInstaller，正式 EXE 生成、启动、升级和签名仍待批准构建环境。

### Task B13: 只读发布就绪审计

- [x] 新增 `token_tracker audit`，集中检查关键交付文件、1000 行门禁、作者头、Python docstring、Android KDoc、层间依赖边界、跨端 v8 SHA-256 和 Python 运行时。
- [x] 普通/JSON/strict 输出区分源码通过与 Android/EXE/Caddy 环境 pending，且不触碰数据库或敏感配置。
- [>] 正式流水线接入与全部外部工具链 pass 证据仍待部署环境。

### Task B14: 本地 Chart.js 供应链边界

- [x] 固定 Chart.js 4.4.7 UMD 构建与 MIT 许可证到 Web 静态目录，移除核心图表对 CDN 的运行时依赖。
- [x] 将 vendor SHA-256 固定到 ADR-051 和只读发布审计，防止静态资源漂移。
- [x] CSP `script-src` 收敛为 self，空态错误提示改为本地静态资源语义。
- [>] 浏览器 CDP 仍需复核图表真实渲染、空态和 reduced-motion。

### Task B15: Android 工具链版本契约

- [x] 按官方 AGP 9.3.0 兼容矩阵固定 Gradle 9.5.0、JDK 17 和 API 37；Compose BOM 固定为 2026.06.00。
- [x] `android/toolchain-doctor.ps1` 只读校验 JDK 主版本和 Gradle Wrapper distribution，不下载、不改 PATH、不生成 Wrapper。
- [x] Android README、开发文档、Gradle 配置和 `token_tracker audit` 的契约引用已同步。
- [>] JDK、官方 Wrapper、SDK API 37/build-tools、APK 构建和设备联调仍待用户批准的工具环境。

### Task B15: Provider 响应投影边界

- [x] 新增 `provider_projection.py`，将上游响应收敛为有界助手文本和稳定标识符。
- [x] Web/Android 继续使用 `choices[0].message.content`，不暴露供应商私有字段、工具参数或隐藏推理。
- [x] ADR-052、API 契约、架构模块表和只读发布审计引用已同步。

### Task B16: 外部客户端 Usage Ingest 边界

- [x] 新增 `usage_ingest_tokens` 表和 `ingest_auth.py`；只保存 SHA-256 digest，支持到期和撤销。
- [x] CLI 提供 `ingest-token create/list/revoke`，创建时只显示一次原文，并输出集成请求头提示。
- [x] 新增 `POST /api/v1/ingest/usage`；固定 `source=ingest`，复用统一 token 校验、幂等写入、限流和错误 envelope。
- [x] API 契约、架构模块表、企业级安全规范、角色复核和 ADR-053 已同步。
- [>] Kimi Code 等 stock 客户端仍不会自动调用该接口；后续必须单独实现 wrapper/Gateway，定义 provider Key 隔离和真实 usage 适配。

### Task B17: 本地 OpenAI-compatible Usage Gateway

- [x] 固定本地 Gateway 契约：`/v1/models`、`/v1/chat/completions`、流式 SSE、最终 usage 和失败语义。
- [x] provider Key 只从进程启动环境进入内存；Gateway 默认只监听 loopback，非 loopback 必须配置独立访问令牌。
- [x] Gateway 复用 provider adapter 做上游 HTTPS/大小/超时校验，并用 Usage Ingest Token 上报，不写中心 SQLite。
- [x] CLI 提供无密钥参数的启动方式，README 写明 Kimi Code/OpenAI-compatible 配置示例。
- [>] 没有用户合法 provider Key 的真实上游调用仍不能伪造；真实非流式/流式联调仍待用户授权。

### Task B18: Gateway 上报可靠性

- [x] 将 Usage Ingest HTTP 投递抽成独立 reporter，不让 Gateway 路由持有重试细节。
- [x] 中心短暂失败时进入有界内存队列，使用原幂等键、退避和最大重试次数，不写 provider Key/prompt。
- [x] Windows 默认使用 DPAPI 加密 SQLite 队列，跨实例恢复保持原幂等键；启动时校验 schema 和全部 payload，无法解密时 fail closed。
- [x] `--memory-only` 作为显式临时调试选项；默认路径、容量、尝试次数、payload 大小和源文件行数均有边界。
- [x] Gateway 响应读取异常、重定向、幂等键控制字符、非回环 HTTP 和队列调度元数据完整性已补齐（ADR-058/059）。
- [x] ADR-056/058/059、架构模块表、README、角色复核、发布审计和隔离 restart smoke 已同步。

### Task B19: Gateway 中心端点自动发现

- [x] 根入口动态端口与 Gateway 默认上报地址的分流风险已记录为 ADR-075。
- [x] `resolve-center-url.ps1` 只读扫描 `5000–5020/api/v1/ready`，失败时 fail-closed。
- [x] `start-gateway.ps1` 保留显式 `-IngestUrl` 覆盖，并对 HTTP 继续要求 `-AllowHttp`。
- [x] 隔离源码实例与假凭据 Gateway 联动、Gateway health、PowerShell 语法、compile、audit 和行数门禁已通过。
- [ ] 真实中心主机与 HTTPS 域名上线后重新执行正式边缘发现/证书门禁。

### Task B20: 备份清单与保留策略只读门禁

**目标：** 让管理员在不打开数据库业务内容、不删除文件的前提下，检查备份数量、年龄、容量和
可选完整性验证结果，并把保留策略变成可执行的本地 CLI 契约。

- [ ] `backup-inventory` 只扫描明确目录下的常规 SQLite 备份文件，不递归、不读取业务表、不删除或改名文件。
- [ ] 支持可选的最小数量、最大年龄、最大容量和完整性验证策略；策略失败返回稳定非零退出码并输出脱敏诊断。
- [ ] JSON/文本输出、README、发布矩阵、ADR、角色复核和只读审计引用保持一致。
- [ ] 使用隔离临时目录验证空目录、满足策略、策略失败、非备份文件和 `--verify` 错误边界；不触碰真实 `data/`。
- [ ] 真实部署仍需负责人确认保留周期、离线副本和恢复责任人。

## 目标

在保持本地优先和可直接体验的前提下，把 AI Token Tracker 交付为一个可持续演进的中心化产品：自动采集优先、前后端边界清晰、七个角色按五个 UI 模块交付、网页和 Android 通过统一契约集成。视觉目标是参考 Moonshot 官网的空间感、黑底大排版和克制动效，并用 Material 3 的语义 token、可读性和可访问性把信息体验做得更完整。

## 历史角色基线

本节保留早期六角色记录，仅用于追溯；当前角色以根目录 `AGENTS.md` 的七槽位模型为准。

1. UI-1 视觉系统负责人：品牌视觉、设计 token、排版、层级和组件外观。
2. UI-2 动效与交互负责人：轨道、扫描线、鼠标响应、页面状态机、过渡节奏。
3. UI-3 响应式与可访问性负责人：320/768/1024/1440 宽度、键盘、焦点、语义和错误状态。
4. 前端工程师：模板、CSS、浏览器状态、图表和 API client。
5. 后端工程师：Flask 路由、业务服务、数据库、provider adapter 和安全边界。
6. 架构师：模块边界、公共契约、ADR、交付闸门和集成风险。

角色目录是独立的交付包，不是 Git/Codex worktree；实际集成只能回到当前 checkout，由架构师按契约汇总。

## 依赖顺序

```text
角色约束与作者规范
        ↓
API / 动画 / 设计 token 契约
        ↓
后端自动采集与 provider 边界
        ↓
前端连接器与仪表盘动效
        ↓
七槽位独立复核
        ↓
根目录启动、浏览器验收、文档交付
```

## 阶段与验收

### Phase 0：基线（已完成）

- SQLite 本地持久化、CLI、Flask 登录和仪表盘可运行。
- 根目录 `start.bat` / `run.py` 可直接启动体验。
- 已有 Moonshot 风格视觉基线、Material 3 语义 token、自动采集表单和 provider 模块。

### Phase 1：协作框架与工程规范（已完成）

- [x] 七个当前角色目录和交付物模板存在；历史角色目录只用于追溯。
- [x] 统一作者名字、文件头、注释、命名和安全规范。
- [x] 任务清单、架构说明和集成闸门可被新代理直接读取。

### Phase 2：契约先行（已完成）

- [x] API 错误、状态、分页/范围和自动采集响应统一。
- [x] provider adapter 接口、模型发现和 usage 解析契约固定。
- [x] 动画状态、设计 token、reduced-motion 和焦点行为写入契约。

### Phase 3：自动采集垂直切片（已完成）

- [x] 连接器检测模型并保留最近的非敏感设置。
- [x] 非流式调用从真实响应自动归档模型、输入、输出、时间和来源。
- [x] 外部 provider 失败、缺 usage、限流、超时和白名单错误均有可读反馈。
- [x] 外部客户端 Usage Ingest Token + 本地 Gateway 已把 Kimi Code/OpenAI-compatible stock client 接入自动上报链路；真实 provider 联调仍需合法 Key。

### Phase 4：Moonshot 级视觉与交互（已完成）

- [x] 首屏轨道/光环、标题扫描、滚动 reveal 和数字变化有统一状态机。
- [x] 鼠标跟随只提供空间反馈，不干扰点击和键盘。
- [x] 桌面、平板、手机和 reduced-motion 状态均有静态可用内容。
- [x] v10 场景资产强化成熟二次元嵌入式工程师、Rust/RL 屏幕语义、专业笔记本和银色桌面工作站；v9/v8 保留回滚（ADR-076）。

### Phase 5：七槽位复核

- [x] UI-1 复核视觉层级和 token 使用。
- [x] UI-2 复核动效节奏、加载/错误/成功状态和性能。
- [ ] UI-3 复核响应式、焦点、语义、对比度和空状态。
- [x] 前端复核 API client、XSS 边界和状态管理。
- [x] 后端复核授权、参数化 SQL、SSRF、Key 生命周期和限流。
- [x] 架构师复核依赖方向、文档、ADR 和回滚边界。

### Phase 6：Windows 阶段性基线交付（非最终交付）

- [x] `start.bat` 根目录体验入口可用。
- [x] 个人入口遇到旧服务占用 5000 时自动选择 5001–5020 的空闲端口，并打印实际地址。
- [x] `python -m compileall`、CLI help、health/security header 检查通过。
- [x] 真实浏览器 DOM、截图、控制台和交互状态通过人工检查。
- [x] README、API 契约、角色交付包和后续扩展清单同步。

以上仅证明当前 Windows 本地基线可运行，不等同于长线目标的全部交付。

### Phase 7：长线最终交付门禁（进行中）

- [>] API、认证、RBAC、密钥、备份和部署安全按 B 阶段逐项验收。
- [>] 外部自动采集：安全 ingest API、本地 Gateway 和 DPAPI 跨重启重试已完成；真实 provider 联调仍需合法 Key。
- [>] UI-3 已完成登录页、v8/v9/v10 场景合成、首帧可读性和隔离空 schema 下仪表盘/管理员页的本地观察；真实 320px 设备指标及合法会话的焦点、错误态、导出和 API 失败路径仍待完成。
- [ ] Android 工具链获批准后完成可复现构建、安装、设备联调和 APK 产物校验。
- [>] 隔离 staging 恢复、应用/Werkzeug 访问日志脱敏和只读 Caddy edge preflight 已完成；正式 HTTPS/Caddy validate、生产 ACL/轮转、限流负载证据、真实数据恢复和回滚流程仍待部署演练。
- [x] 当前 checkout 已建立本地 `main` Git 基线；敏感数据忽略边界已用真实路径验证，远程仓库与上传仍未启用（ADR-036）。
- [ ] 全部交付项通过架构师最终审计后，才允许将长线目标标记完成。

APK 编译、安装和真实 Android 联动不属于本地源码交付的完成证明，继续由平台扩展清单中的工具链门禁单独跟踪。

## 验证原则

本项目按工作区约束执行非破坏性验证：编译诊断、CLI 帮助、服务健康检查、HTTP 响应头、真实浏览器运行状态和人工视觉复核。默认不创建或运行单元测试、mock、fixture、harness 或测试专用资产；如未来需要测试体系，应单独立项并先更新项目约束。

## 风险

| 风险 | 影响 | 应对 |
|---|---|---|
| 外部产品不暴露 usage | 无法凭空计算 token | 不猜测；提示用户，或增加专用 adapter/gateway |
| provider URL 由用户输入 | SSRF/凭据泄露 | HTTPS、白名单、超时、禁重定向、限流和 Key 不落盘 |
| 动效过多造成疲劳 | 可读性和性能下降 | 信息优先、低频循环、reduced-motion、移动端降级 |
| 多角色重复改同一文件 | 集成冲突 | 先改契约和交付包，再由架构师按顺序汇总 |
