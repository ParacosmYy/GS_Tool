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
- [x] ADR-025、README 和部署说明已同步；staging 恢复演练保留为后续部署门禁。

### Task B5: 共享/生产部署只读预检

- [x] 新增 `deployment_checks.py` 和 `preflight` CLI。
- [x] production 预检强制 Secret、Secure Cookie、HTTPS provider allowlist 和数据库路径门禁。
- [x] ADR-026、README 和部署说明已同步，不执行联网、部署或防火墙变更。

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
六角色独立复核
        ↓
根目录启动、浏览器验收、文档交付
```

## 阶段与验收

### Phase 0：基线（已完成）

- SQLite 本地持久化、CLI、Flask 登录和仪表盘可运行。
- 根目录 `start.bat` / `run.py` 可直接启动体验。
- 已有 Moonshot 风格视觉基线、Material 3 语义 token、自动采集表单和 provider 模块。

### Phase 1：协作框架与工程规范（已完成）

- [x] 六个角色目录和交付物模板存在。
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

### Phase 4：Moonshot 级视觉与交互（已完成）

- [x] 首屏轨道/光环、标题扫描、滚动 reveal 和数字变化有统一状态机。
- [x] 鼠标跟随只提供空间反馈，不干扰点击和键盘。
- [x] 桌面、平板、手机和 reduced-motion 状态均有静态可用内容。

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
- [>] UI-3 已完成 v5 登录页 320/768/1024/1440 独立浏览器证据、焦点、ARIA、对比度和横向溢出复核；仪表盘、连接、历史、管理员页面仍待合法认证会话和空/错误态复核。
- [ ] Android 工具链获批准后完成可复现构建、安装、设备联调和 APK 产物校验。
- [>] 隔离 staging 恢复、应用/Werkzeug 访问日志脱敏和只读 Caddy edge preflight 已完成；正式 HTTPS/Caddy validate、生产 ACL/轮转、限流负载证据、真实数据恢复和回滚流程仍待部署演练。
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
| 六角色重复改同一文件 | 集成冲突 | 先改契约和交付包，再由架构师按顺序汇总 |
