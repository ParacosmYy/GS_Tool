# AI Token Tracker 交付清单

> 状态：`[x]` 已完成，`[>]` 进行中，`[ ]` 待处理。作者：AI Token Tracker Engineering Team。

## Platform expansion — current

- [x] 架构治理切片 A：将用量汇总/分页/导出和日志读取收敛到 Application 用例边界，保持 API 兼容
- [x] 架构治理切片 A-补充：集中 Web 与 `/api/v1` 的限流状态和替换接口
- [x] 架构治理切片 A-补充：将 v1 凭据校验收敛到认证 Application 边界
- [x] 架构治理切片 A-补充：拆分 SQLite schema 生命周期与查询实现
- [x] 架构治理切片 A-补充：统一 API 404/405 错误 envelope 和 X-Request-ID 关联策略
- [x] 架构治理切片 A-补充：管理员读模型、导出和审计编排进入 Application 层
- [x] 安全门禁 B1：共享/生产模式对 Session Secret fail-closed，本机模式保持零配置体验
- [x] 安全门禁 B2：provider API Key、URL、模型和消息输入执行字段级资源预算
- [x] 安全门禁 B3：Web 与 `/api/v1` 共享凭据校验，bearer token 生命周期独立
- [x] 安全门禁 B4：新增 SQLite 备份只读验证命令和必要 schema 检查
- [x] 安全门禁 B5：新增共享/生产部署只读预检和 HTTPS 配置门禁
- [x] 安全门禁 B6：Web/Android 共用账号写入预算，记录、工作事件、日志和管理员导出按用户滑动限流

- [x] 已将 Windows 个人账本升级为中心化多用户服务设计，保留根目录 `start.bat` 体验入口
- [>] `windows/` 服务端与 `android/` 客户端目录已建立，Android Compose 工程骨架待工具链编译验证
- [x] 写入 1 UI + 4 DEV + 2 ARCH 角色约束、五个 UI 模块归属和 1000 行文件硬门禁
- [x] 写入企业级文件头、公共接口 KDoc/docstring、跨端契约和高内聚低耦合注释规范
- [x] users.role、admin RBAC、管理员概览、审计事件和安全导出（Windows 首切片）
- [x] work_events/app_logs 结构化模型：方向、效率、正确/错误码、request_id、日志等级
- [>] `/api/v1` 登录/会话/统计/记录/事件/日志契约与网页、Android 双端接入（Windows 已接入，Android 登录/汇总/记录分页读取、幂等写入与管理员只读竖切片已完成）
- [>] 企业级模块化单体、网站/EXE 双部署形态与项目级 skills 已写入架构和交付文档
- [x] EXE 构建工具链独立锁定，PyInstaller 资源收集和用户数据目录边界已写入 ADR-049
- [x] 新增只读 `token_tracker audit` 发布审计入口，区分源码通过与外部工具链 pending
- [x] Chart.js 4.4.7 固定为项目本地静态资产，核心图表不再依赖 CDN
- [x] Provider 响应投影：Web/Android 只接收有界助手文本和稳定字段，不透传原始上游 JSON
- [x] 外部 Usage Ingest Token：按用户签发/撤销、摘要存储、固定 `source=ingest`、幂等写入和限流边界
- [>] Android 登录、个人仪表盘、token 分页历史、结构化工作信号写入、最近活动历史和管理员只读观测台已接入；待工具链批准后编译联调
- [x] Android 首屏背景资产、低干扰遮罩、Compose edge-to-edge 约束和 Rust/RL 工程师视觉主题已落地
- [x] Web/Android 统一切换 v9 场景资产；端侧遮罩、漂移实现保持解耦，v8 及之前版本保留回滚
- [x] Android 构建入口已固定项目内 SDK、Gradle 和 Android 用户缓存路径；未获批准不下载工具链
- [x] Android 工具链契约已按官方兼容矩阵固定为 AGP 9.3.0、Gradle 9.5.0、JDK 17、API 37 和 Compose BOM 2026.06.00；只读 doctor 会校验版本，实际工具仍待批准
- [ ] 用户批准工具链后完成 Android Studio/Gradle/SDK 编译、设备联调与 APK 产物验收
- [x] Kimi provider 快速连接预设：Kimi Code / Kimi 开放平台 / OpenAI / 自定义 URL，Key 仍只留在页面内存
- [>] Windows 分享入口：已提供需确认的可信局域网预览、本地校验备份命令、隔离 staging 恢复演练和隐私说明；正式 HTTPS、生产 ACL/轮转与真实数据恢复仍是部署门禁
- [x] 分享 handoff 预检：`deployment/share-doctor.ps1` 统一 LAN/Production 只读配置门禁，不启动服务或修改主机状态（ADR-069）
- [x] 统一交付预检：`release-doctor.ps1/.bat` 编排源码、EXE、Android 和 Local/LAN/Production 门禁，退出码区分通过、pending、失败（ADR-071）
- [x] 建立最终交付验收矩阵：逐条映射用户需求、工程约束、直接证据和未关闭门禁，作为架构师最终签署入口
- [x] Web 个人工作信号竖切片：方向、结果、效率、正确/错误码写入既有 v1 Application 边界，最近活动表仅渲染脱敏结构化字段（ADR-072）
- [x] Admin 成员详情补齐脱敏诊断日志读模型：等级、事件、错误码、消息和 request id 使用 textContent 展示，不复制 SQL

## Foundation

- [x] SQLite schema、local-time range 和 CSV export
- [x] CLI init/add/summary/export/serve
- [x] Flask auth、CSRF、security headers、per-user isolation
- [x] 根目录 `start.bat` 和 `run.py` 体验入口
- [x] 个人入口检测默认端口冲突并回退到可用端口；不终止已有服务，浏览器打开当前源码实际地址（ADR-031）
- [x] 当前 checkout 建立本地 `main` Git 基线，真实验证数据库、凭据、缓存和部署日志不会进入提交（ADR-036）

## Role framework

- [x] 建立七槽位独立交付目录（1 UI + 4 DEV + 2 ARCH）
- [x] 为每个角色记录输入、输出、验收点和集成边界
- [x] 统一源文件作者头和企业级注释规范
- [x] 将所有公共决策链接到 ADR/API/UI 契约

## Automatic collection

- [x] provider URL 白名单和 HTTPS 默认策略
- [x] `/api/provider/models` 自动模型发现
- [x] `/api/proxy/chat/completions` 自动读取 usage 并归档
- [x] 统一结构化 API error envelope，保持现有客户端兼容
- [x] provider adapter registry，支持按 provider 扩展而不改路由
- [x] 评估 OpenAI-compatible 外部 gateway 的用户归属和密钥边界（见 ADR-003/053）
- [x] 本地 OpenAI-compatible Gateway：loopback 默认、独立访问令牌、Kimi/OpenAI-compatible 上游转发、SSE usage 解析和 Usage Ingest 上报
- [x] Gateway 上报失败的有界内存重试、幂等键复用和退避；Windows DPAPI 加密 SQLite 队列已支持跨重启恢复（ADR-056）
- [x] Gateway 响应读取/重定向/幂等键和非回环传输门禁、队列元数据 fail-closed 已补齐（ADR-058/059）
- [x] Gateway 持久队列运行时 SQLite/DPAPI 故障 fail-closed，避免 worker 静默退出或透传未封装 500（ADR-068）
- [x] per-user Usage Ingest Token 和 `/api/v1/ingest/usage` 已落地（ADR-053）；stock client 可通过本地 Gateway（ADR-054）自动上报，真实 provider 联调仍待合法 Key

## UI / motion

- [x] Moonshot-inspired black canvas、large type、orbit signal
- [x] Material 3 semantic color tokens and readable form controls
- [x] pointer aura/ring、count-up、reveal、glitch、scanline
- [x] 将动画从分散 CSS 规则收敛为可追踪的 motion state contract
- [>] 320/768/1024/1440 四档浏览器验收记录（v9/v10/v11 已补显式主图层、本机 Chrome 只读视觉截图、首帧可读性、窄屏尺寸收敛、注册页 heading 顺序和 v10 场景可见度；v15/v16/v17/v18 已补隔离合法会话下 Dashboard/Admin、ARIA、动态错误播报、四档 viewport、焦点回流、历史非空、服务端 CSV 200 和 Provider 502 错误态证据；真实设备指标、浏览器下载落盘、真实 Provider 成功、对比度与 reduced-motion 真实环境仍待补齐）
- [x] 背景品牌资产升级：v9 御姐二次元嵌入式工程师、MacBook Pro/Mac Studio 形态、Rust/RL 工作站、鼠标视差和移动端低幅漂移；v8 及之前版本保留回滚
- [x] loading/connection/empty 状态统一视觉组件
- [x] UI-3 静态语义切片：单一仪表盘 h1、表格 scope/caption、管理员详情焦点回流和密码长度边界
- [x] UI-3 v14/v15：Dashboard glitch 标题单次无障碍名称、隔离合法会话 Dashboard/Admin 空态/焦点/320/768 viewport 运行时证据
- [x] UI-3 v16：共享 live-region 状态契约、Dashboard 错误/恢复播报和 Admin 正常状态运行时证据（ADR-067）
- [x] UI-3 v17：Dashboard/Admin 320/768/1024/1440 viewport、无横向溢出、四档焦点和明细焦点回流运行时证据
- [x] UI-3 v18：隔离合法会话下历史非空、周期切换、服务端 CSV 200、Provider 502 错误态和 ARIA 恢复运行时证据
- [>] UI-3 v19：个人工作信号写入、最近活动历史、错误播报、响应式表格与隐私边界运行时证据
- [x] `token_tracker audit` 增加 Web UI 静态契约门禁：label、table caption/scope、图片 alt、跳过链接和动效降级（ADR-063）
- [x] `token_tracker audit` 扩展代码/配置/文档文本 1000 行门禁，作者头仍按自有代码边界检查（ADR-064）
- [x] UI-3 v14：Dashboard glitch 标题的装饰副本从无障碍树隔离，保留一次稳定可读标题

## Review gates

- [x] UI-1 visual review
- [x] UI-2 motion/performance review
- [>] UI-3 responsive/accessibility review（登录/注册页语义顺序、Dashboard/Admin 隔离合法会话空态、动态错误播报、四档 viewport、焦点回流、历史非空、服务端导出 200 和 Provider 网络失败已补齐；真实设备指标、浏览器下载落盘、真实 Provider 成功、对比度与 reduced-motion 真实环境仍待补证）
- [x] frontend boundary review
- [x] backend security/data review
- [x] architect integration review

## Delivery

- [x] Windows 阶段基线 README quick start、API contract 和 ADR 已更新
- [x] 当前阶段 compile/help/health/security-header 和浏览器证据已保存
- [x] ADR-066 固化多人分享场景的 per-user 写入预算和 `429 RATE_LIMITED` 契约
- [x] 当前阶段未提交真实密钥、测试数据或生成 QA 数据
- [>] 长线最终交付审计仍进行中，不能把阶段基线当作全部项目完成

### Long-term completion audit

- [>] Windows 安全/部署 B 阶段持续交付
- [>] Web UI-3 四档响应式、焦点、ARIA、对比度和空状态独立证据（Dashboard/Admin 四档、动态错误播报、历史非空、服务端导出 200 和 Provider 网络失败已有隔离证据；浏览器下载落盘、真实 Provider 成功和真实设备环境仍待）
- [ ] Android JDK/Gradle/SDK 获批准后的构建、安装、设备联调和 APK
- [>] HTTPS 中心部署、备份恢复演练、限流和脱敏访问日志：隔离 staging 恢复、应用/Werkzeug 日志脱敏已通过；正式 Caddy/HTTPS、生产 ACL/轮转、限流压测和真实数据恢复仍待执行
- [ ] 架构师最终审计、回滚说明和正式交付归档

## Architecture governance evidence

- [x] `services.py` 统一个人用量 summary / page / CSV 用例。
- [x] `events.py` 统一个人 app log page 用例。
- [x] Web 与 `/api/v1` 控制器的本切片范围内无 SQL、汇总或导出直接调用。
- [x] 使用 `windows/.venv` 完成 `compileall`、CLI help、health、login、静态资源和只读应用函数 smoke check。
- [x] 未创建测试数据、未下载依赖；默认 Python 缺 Flask 的事实已由项目 `.venv` 解决。
- [x] 认证用户查询、管理员读模型和 provider 代理已分别收敛到 Application 边界；后续仅保留正式部署和跨端验证门禁。
- [x] ADR-017 记录 local 内存与 shared/production/lan SQLite 共享限流的替换边界。
- [x] 限流适配器完成隔离 SQLite 并发冒烟：8 并发请求得到 3 放行/5 拒绝，原始身份未写入表。
- [x] ADR-018 记录 v1 认证控制器与凭据校验的边界。
- [x] ADR-019 记录 `schema.py` 与 `db.py` 的基础设施边界。
- [x] ADR-020 记录 API 错误和请求关联 ID 的稳定边界。
- [x] ADR-021 记录管理员 Application 与审计边界。
- [x] ADR-022 记录共享部署 Session Secret 的 fail-closed 策略。
- [x] ADR-023 记录 provider 输入字段级预算和超限语义。
- [x] ADR-024 记录凭据应用边界与 bearer token 生命周期拆分。
- [x] ADR-025 记录只读备份验证和 staging 恢复边界。
- [x] ADR-026 记录无副作用部署预检和实际部署演练边界。
- [x] ADR-028 记录显式恢复目标、默认拒绝覆盖和 SQLite 原子恢复边界。
- [x] ADR-029 记录 Caddy HTTPS 边缘、Waitress 启动包装器和回滚顺序边界。
- [x] `serve --production` 在 Waitress 监听前复用 HTTPS/Secret/Cookie/provider allowlist 生产预检，拒绝不安全启动。
- [x] HTTP 可信局域网分享改用显式 `serve --lan-preview`，不再把 LAN 预览误标为 production。
- [x] shared/production/lan 使用 SQLite 共享滑动窗口；限流 key 哈希存储，控制器接口保持不变。
- [x] ADR-031 记录个人体验入口的无副作用端口回退，避免旧进程遮蔽当前源码。
- [x] 应用层访问日志只记录脱敏 method/path/status/duration/request_id；不记录 query、Header、Cookie、Key、IP 或 body，并过滤 Werkzeug 默认 request-line（ADR-032）。
- [>] ADR-032 已提供 Caddy 显式日志过滤模板；正式 HTTPS 主机上的 Caddy validate、ACL、轮转和真实日志样本仍待部署权限。
- [x] ADR-028 备份恢复边界通过隔离空 schema staging 演练；Windows SQLite 连接显式关闭，避免文件句柄阻塞原子发布。
- [x] 备份验证覆盖当前 `usage_ingest_tokens` schema，重复秒级备份使用唯一后缀避免覆盖（ADR-060）。
- [x] ADR-033 提供只读 Caddy/日志目录/宽泛 ACL 预检器；当前环境未安装 Caddy，真实 edge validate 仍待正式部署环境。
- [x] ADR-034 将 `windows/start-lan.bat` 收敛为带 `SHARE` 确认的 canonical LAN 预览 wrapper，消除无确认绑定 `0.0.0.0` 的入口。
- [x] ADR-035 补齐 data 子目录 SQLite、backups、staging 的仓库忽略边界，避免敏感数据库误提交。
- [x] ADR-036 建立当前 checkout 的本地 `main` 版本控制保存点，不配置远程、不上传，并通过 `git check-ignore` 验证敏感路径。
- [x] ADR-037 收敛密码长度、业务日志脱敏/合法 JSON 和认证响应 `no-store` 边界。
- [x] ADR-038 固化 Windows Python runtime lock，启动器和 EXE 打包入口统一消费锁定依赖。
- [x] ADR-039 将管理员 CSV 改为有界游标导出，超限返回 `413 EXPORT_TOO_LARGE` 并写拒绝审计。
- [x] ADR-040 让个人 Web/API/CLI CSV 复用共享有界序列化器，保持字段与 BOM 契约不变。
- [x] Android 跨端 Remote/Repository/DTO/Compose 公开边界补齐 KDoc，保留工具链未批准的构建门禁。
- [x] ADR-041 增加 Android Release HTTPS 构建门禁，保留 Debug 模拟器 HTTP 默认值。
- [x] ADR-042 增加 `/api/v1/ready` SQLite schema readiness 探针，和 liveness health 分离。
- [x] ADR-043 固化 Android Keystore 会话 blob 的同步提交与清除失败语义，避免 bearer 生命周期异步落盘。
- [x] Android Release 运行时沿 endpoint/repository/HTTP adapter 传播 HTTPS 门禁，补充 ADR-044。
- [x] 修正 `preflight` 只读语义：路径解析不再创建数据库父目录，补充 ADR-045。
- [x] Caddy 示例显式固定 access log 轮转/保留策略，补充 ADR-046；正式主机验证仍待部署权限。
- [x] 生产 Waitress 强制 loopback 绑定，防止绕过 Caddy HTTPS；LAN 预览保持独立确认路径（ADR-061）。
- [x] production wrapper 将实际 `-BindAddress` 同时传给 preflight 和 Waitress，并清除旧的非 loopback production 示例。
- [x] `token_tracker audit` 固定 production/Caddy/LAN 启动边界，防止部署示例回退（loopback/Caddy/SHARE）。
- [x] 增加无密钥参数的 `start-gateway.ps1/.bat` 快捷入口，默认 loopback + DPAPI 队列，HTTP/LAN 仍需显式确认。
