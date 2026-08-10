# AI Token 用量追踪器

一个企业级模块化单体 AI token 观测平台：Windows 端集中承载 Web、CLI、SQLite 和管理员审计；浏览器与 Android 通过稳定的 `/api/v1` 契约接入。个人记录、团队汇总、工作事件、诊断日志和 CSV 导出分别建模。

## 一键打开（个人体验入口）

在项目根目录双击 `start.bat` 即可。根入口会转到 `windows/start.bat`，自动检查 `windows/.venv`、安装依赖、创建 `windows/.env`（如果不存在）、启动本机 Web 服务并打开浏览器。

默认地址：<http://127.0.0.1:5000>。如果 5000 已被其他本地服务占用，`run.py` 会在
5001–5020 中选择第一个可用端口，并让浏览器打开当前源码对应的实际地址；已有服务不会被终止。

`run.py` 是同一个入口的 Python 版本，适合在终端启动或调试；正常本机入口使用锁定的 Waitress WSGI
运行时，不会出现 Flask development server 警告。这个入口默认只监听本机，不会把个人数据暴露给局域网；
即使设置了 `TOKEN_TRACKER_HOST`，个人入口也只接受 loopback 地址，非法或非 loopback 配置会在创建应用前拒绝启动；
可信 LAN 预览必须显式使用 `--lan-preview --host 0.0.0.0`，正式分享必须走 HTTPS edge，production 只允许 loopback。

## 能做什么

- 记录模型名称、输入 token、输出 token、本地时间、备注和来源。
- SQLite 持久化，数据默认保存在 `data/token_tracker.sqlite3`。
- CLI 支持初始化、添加、按周期汇总、按日期范围汇总、CSV 导出和启动 Web。
- Web 支持注册/登录，每个普通用户只能看到自己的记录；管理员通过服务端 RBAC 查看团队汇总、指定成员明细、结构化事件和脱敏导出。
- Android 使用同一账号的 `/api/v1/auth/*` bearer token；SQLite 永远只在 Windows 服务端，移动端不接触数据库文件。
- 工作方向、效率评分、正确/错误码、request id、应用日志和管理员审计事件分开保存，避免把不同生命周期的数据揉成一张表。
- 以自动采集为主：填一次 Base URL + API Key，先自动检测模型，再发起真实调用并从响应 `usage` 自动记账；手动录入只保留为异常补录。
- 仪表盘显示总 token、输入/输出、模型占比、最近趋势和最近记录；Chart.js 固定版本随项目静态发布，核心图表不依赖外部 CDN。

架构、API 契约、UI 动效规范和关键决策见：

- [`docs/architecture.md`](docs/architecture.md)
- [`docs/api-contract.md`](docs/api-contract.md)
- [`docs/ui-motion-spec.md`](docs/ui-motion-spec.md)
- [`docs/design-tokens.md`](docs/design-tokens.md)
- [`docs/motion-contract.md`](docs/motion-contract.md)
- [`docs/engineering-standards.md`](docs/engineering-standards.md)
- [`roles/README.md`](roles/README.md)
- [`docs/decisions/ADR-078-personal-launcher-loopback.md`](docs/decisions/ADR-078-personal-launcher-loopback.md)
- [`docs/decisions/ADR-094-cli-waitress-and-bind-gate.md`](docs/decisions/ADR-094-cli-waitress-and-bind-gate.md)
- [`skills/README.md`](skills/README.md)
- [`tasks/todo.md`](tasks/todo.md)
- [`docs/decisions/`](docs/decisions/)

## 1. 安装

建议使用 Python 3.11 或更高版本。PowerShell 示例：

```powershell
cd D:\Workplace\Agent_Workplace\ai-token-tracker\windows
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install -r requirements.lock
Copy-Item .env.example .env
```

`requirements.txt` 是直接依赖的宽范围契约，`requirements.lock` 是当前 Windows 运行时的可复现安装输入。根目录 `start.bat` 会自动创建同一个 `windows/.venv`，并优先使用锁定文件；pip 缓存固定到 `windows/.cache/pip`，运行依赖不会安装到系统 Python。

然后编辑 `.env`，至少替换 `TOKEN_TRACKER_SECRET_KEY`。如果需要使用 Web 代理，必须配置 `TOKEN_TRACKER_ALLOWED_BASE_URLS` 白名单。

## 2. CLI 使用

查看完整帮助：

```powershell
python -m token_tracker --help
python -m token_tracker add --help
```

初始化数据库：

```powershell
python -m token_tracker init
```

首次部署：先在网页注册账户，再用本机 CLI 授予管理员角色：

```powershell
python -m token_tracker admin set-role --username your-name --role admin
```

角色提升只开放给本机 CLI bootstrap，不提供普通网页自助提权接口。

添加一条记录：

```powershell
python -m token_tracker add `
  --model kimi-code `
  --input 1200 `
  --output 350 `
  --note "项目设计讨论"
```

省略 `--timestamp` 时使用当前本地时间。也可以指定本地时间：

```powershell
python -m token_tracker add --model gpt-4o --input 800 --output 120 --timestamp "2026-08-10 14:30" --note "日报总结"
```

查看今天、本周、本月或全部汇总：

```powershell
python -m token_tracker summary --period day
python -m token_tracker summary --period week
python -m token_tracker summary --period month
python -m token_tracker summary --period all
```

按日期范围查看（结束日期包含当天）：

```powershell
python -m token_tracker summary --from 2026-08-01 --to 2026-08-10
```

导出 CSV：

```powershell
python -m token_tracker export --period month --output exports/august.csv
```

个人和管理员 CSV 导出统一限制为最多 100,000 行、16 MiB；超出时服务端返回 `413`
（`EXPORT_TOO_LARGE`），CLI 不会写入半成品文件。

## 3. 启动 Web 仪表盘

本机开发：

```powershell
python -m token_tracker serve
```

普通 `serve` 使用 Waitress，不会出现 Flask development server 警告。浏览器打开启动窗口打印的
实际地址（通常是 <http://127.0.0.1:5000>），注册账户即可使用；只有需要调试器时才显式添加
`--debug`。本地命令拒绝非 loopback 监听，可信局域网必须使用下方的 `--lan-preview`。

启动前进行只读配置预检：

```powershell
python -m token_tracker preflight
python -m token_tracker preflight --production --host 127.0.0.1
```

预检只解析配置和数据库路径，不会创建数据库文件或父目录；目录创建仅发生在显式初始化
或启动服务流程中。

发布前可运行只读交付审计：

```powershell
python -m token_tracker audit
python -m token_tracker audit --json
python -m token_tracker audit --strict
```

普通审计会区分源码通过项与等待 Android/EXE/Caddy 工具链的 pending 项；`--strict` 会在仍有
外部环境门禁时返回非零，适合接入正式发布流水线。审计不会创建数据库、用户、备份、构建产物或
读取 `.env` 内容。

提交级源码质量闸门可以在本机复现：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\\ci\\quality-gate.ps1 `
  -PythonPath .\\.venv\\Scripts\\python.exe
```

该闸门执行 `git diff --check`、Python 编译、发布审计、CLI help 和 PowerShell AST 解析；`pending`
外部工具不会被伪装为通过。远程仓库连接后，`.github/workflows/quality-gate.yml` 会在 Windows runner
上调用同一入口。

局域网分享给同学（Windows 电脑作为中心服务和数据库宿主）：

```powershell
python -m token_tracker serve --host 0.0.0.0 --port 5000 --lan-preview
```

也可以直接双击 [`windows/start-lan.bat`](start-lan.bat)。它只使用项目内已经创建的
`.venv`，通过 Waitress 启动局域网服务；如果运行环境还没有准备好，请先双击
[`windows/start.bat`](start.bat) 完成一次本地初始化。
启动窗口会列出检测到的 IPv4 地址；Android 真机应填写其中一个地址并追加
`/api/v1`，例如 `http://192.168.1.20:5000/api/v1`（仅可信局域网调试）。

然后让同学访问你的局域网 IP，例如 `http://192.168.1.20:5000`。Windows 防火墙需要允许该端口；只在可信局域网临时使用 HTTP。正式分享必须使用 HTTPS 反向代理、持久化磁盘和生产 WSGI。

公网分享建议使用一台 VPS、HTTPS 反向代理和持久化磁盘，并设置：

```dotenv
TOKEN_TRACKER_SECRET_KEY=<随机长字符串>
TOKEN_TRACKER_SECURE_COOKIE=1
TOKEN_TRACKER_ALLOWED_BASE_URLS=https://api.moonshot.cn/v1,https://api.moonshot.ai/v1,https://api.openai.com/v1
```

不要直接把 Flask 开发服务器暴露到公网，也不要把 `.env`、SQLite 文件或真实 API Key 提交到 GitHub。正式部署还应在反向代理或网关层增加 HTTPS、访问日志脱敏和更严格的限流。

## 4. 如何自动接入 Kimi / 其他模型

Web 页面里的“调用并自动记账”面向 OpenAI-compatible 的 `POST /chat/completions` 接口：

1. 在 `.env` 的 `TOKEN_TRACKER_ALLOWED_BASE_URLS` 写入服务的 Base URL；当前自动适配器使用 OpenAI-compatible 的 `/models` 与 `/chat/completions` 形状。Kimi 开放平台使用 `https://api.moonshot.cn/v1`；Kimi Code 使用独立的 `https://api.kimi.com/coding/v1`，两者 API Key 不能混用，详见 [Kimi Code API access](https://www.kimi.com/code/docs/en/) 和 [Kimi Code error reference](https://www.kimi.com/code/docs/en/kimi-code/error-reference.html)。
2. 在页面的“自动采集”区域选择 Kimi Code、Kimi 开放平台或自定义预设，确认 Base URL 后输入自己的 API Key，点击“自动检测模型”。预设只填充地址，不会代填或保存 Key。
3. 系统请求上游 `/models`，将返回的模型自动放入可选列表；你不需要手动填写输入/输出 token。
4. 输入 prompt 后点击“发送 · 自动记账”。服务端读取响应中的 `usage.prompt_tokens` 和 `usage.completion_tokens`，优先使用响应里实际返回的模型归档。
5. API Key 只存在于当前页面内存，页面刷新即清除；前端只持久化 Base URL 和最近选择的模型，不保存 Key，不写入 SQLite。

Android 使用同一套 provider 能力，但通过 bearer 版本接口调用：`POST /api/v1/provider/models` 和 `POST /api/v1/proxy/chat/completions`。Android 不接触 Windows SQLite，只接收模型列表、助手文本、usage 摘要和记录投影；调用请求携带 `Idempotency-Key`，网络重试不会重复计数。

当前代理先支持非流式请求，因为流式响应通常需要额外处理 usage 汇总。没有返回 usage 的 provider 不会被伪造记账，页面会提示使用折叠的异常补录。

注意：Kimi Code 等产品的实际接入地址、可用模型和计费口径可能与公开 Kimi API 不同，应以对应账户/产品的官方文档为准；如果它不是 OpenAI-compatible 接口，可以继续添加专用 provider adapter。

如果希望 Kimi Code、OpenAI SDK 或其他外部客户端也自动进入账本，调用必须经过本项目的代理（或增加对应客户端插件/适配器）。任何网页都不能凭空读取另一个进程或云产品的调用记录；代理模式是自动采集的边界。当前页面已经把“检测模型 → 发起调用 → 读取 usage → 入库 → 刷新图表”串成一条自动链路，不要求日常填写 token 数。

对于已经由外部 wrapper/SDK 获得真实 `usage` 的客户端，可以为账户创建独立的 Usage Ingest Token：

```powershell
python -m token_tracker ingest-token create --username your-name --label kimi-code --expires-days 90
python -m token_tracker ingest-token list --username your-name
python -m token_tracker ingest-token revoke --username your-name --id 1
```

创建命令只在终端显示一次 `ait_...` 原文；wrapper 通过
`POST /api/v1/ingest/usage` 携带 `X-AI-Tracker-Ingest-Token` 和必填的
`Idempotency-Key` 上报模型、输入/输出 token、时间和短备注。中心服务只保存 token 摘要，固定
以 `source=ingest` 入账，不接受 API Key、原始 prompt 或完整 provider 响应。这个入口不会自动
拦截 stock Kimi Code 进程；要实现“调用即上报”，需要让客户端明确指向下方的本地 Gateway。

### 本地 Gateway：让外部客户端自动记账

本项目现在提供独立的本地 OpenAI-compatible Gateway。它默认只监听 `127.0.0.1`，provider
Key 和中心 ingest token 只从环境变量进入 Gateway 进程内存，不写入中心业务 SQLite、日志或命令行；
中心上报失败时默认只把有界 usage DTO 写入 Windows DPAPI 加密队列：

```powershell
# 1. 先为当前账户创建一次性 Usage Ingest Token，并把输出的 ait_... 保存到 windows/.env
python -m token_tracker ingest-token create --username your-name --label kimi-code --expires-days 90

# 2. 在 windows/.env 填入，不要提交该文件
TOKEN_TRACKER_GATEWAY_PROVIDER_KEY=<your-kimi-code-api-key>
TOKEN_TRACKER_GATEWAY_INGEST_TOKEN=<ait-token-from-step-1>

# 3. 启动本机 Gateway；本地 HTTP 中心只在明确调试时加 --allow-http
python -m token_tracker gateway `
  --upstream-url https://api.kimi.com/coding/v1 `
  --ingest-url http://127.0.0.1:5000/api/v1/ingest/usage `
  --allow-http
```

然后把 Kimi Code 或其他 OpenAI-compatible 客户端的 Base URL 指向
`http://127.0.0.1:8787/v1`。客户端自身要求填写的 API Key 只作为本机 Gateway 访问占位值；
Gateway 会忽略它并使用 `TOKEN_TRACKER_GATEWAY_PROVIDER_KEY` 调用真实上游。Kimi Code 官方
OpenAI-compatible Base URL 为 `https://api.kimi.com/coding/v1`，并支持第三方工具覆盖 Base
URL，详见 [Kimi Code API access](https://www.kimi.com/code/docs/en/)。

也可以使用 Windows 快捷入口减少命令行参数。脚本在省略 `-IngestUrl` 时会只读扫描
`127.0.0.1:5000–5020/api/v1/ready`，自动选择第一个就绪的当前中心服务；如果需要接入
指定地址，仍可以显式传入 `-IngestUrl` 覆盖发现结果：

```powershell
.\start-gateway.bat -AllowHttp
```

该入口默认监听 `127.0.0.1:8787`，默认上游为 Kimi Code；可用
`-UpstreamUrl`、`-IngestUrl`、`-BindHost` 和 `-BindPort` 覆盖非密钥配置。provider Key、
Usage Ingest Token 和可选 Gateway Token 仍只能放在 `.env`/进程环境，不会进入脚本参数。
如果中心已经通过 HTTPS 暴露，请不要添加 `-AllowHttp`。

Gateway 支持 `/v1/models`、非流式 Chat Completions 和 SSE 流式 Chat Completions；流式请求
只在完成 chunk 提供合法输入/输出 usage 时入账，否则响应头 `X-AI-Tracker-Usage` 为
`missing`，不会估算。中心短暂不可用时本次 provider 调用仍会返回，响应状态会标为
`queued`，失败记录默认在 `data/gateway-usage-queue.sqlite3` 中以 DPAPI 密文跨重启恢复；达到
容量/次数上限或持久队列运行时故障时会返回 `report-failed`。队列运行时故障会 fail-closed，
保留未确认的密文行并暂停该进程的持久投递，详见 ADR-068。临时调试可以显式加 `--memory-only`，此时重启会丢失
未上报记录。不要把 Gateway 绑定到公网或可信 LAN，除非另外配置
`TOKEN_TRACKER_GATEWAY_ACCESS_TOKEN` 并使用 HTTPS 边缘保护。

`--allow-http` 只允许与 loopback Gateway 一起使用；非 loopback 监听会在启动前拒绝该组合，避免把
provider 调用链降级为明文共享服务。DPAPI 队列启动时还会校验行数、尝试次数和调度元数据，损坏或
篡改会 fail-closed，不会静默重置队列。

Gateway 允许省略 `Idempotency-Key`，会自动生成本次调用的唯一键；如果客户端提供该请求头，必须不
超过 160 个字符且不能包含控制字符。非法幂等键会在调用 provider 前返回 `400`，不会静默替换成新键，
以确保客户端重试仍能正确去重。上游响应读取中断会返回 `502`，上游重定向也会明确返回 `502`，不会
把这两类边界错误伪装成普通 JSON 解析失败。

## 5. Android 与跨端协议

Android 工程位于同级目录 `../android/`，默认模拟器地址为 `http://10.0.2.2:5000/api/v1`；真机需要把 API Base URL 改为 Windows 主机的局域网 HTTPS 地址。Android 的登录流程为：

1. `POST /api/v1/auth/login` 获取短期 access token 和轮换 refresh token。
2. 使用 `Authorization: Bearer <access_token>` 请求 `/api/v1/me/summary`、`/api/v1/events/work` 和 `/api/v1/logs`。
3. access token 过期后调用 `/api/v1/auth/refresh`；服务端使旧 refresh token 立即失效。
4. 事件离线重试携带 `protocol_version=1`、`command`、`request_id` 和 `idempotency_key`；token 写入携带 `Idempotency-Key`，避免会话刷新或网络重试重复计数。

管理员登录后会进入 `/admin`，可以查看成员聚合和审计过的脱敏导出。`work_events` 用于方向/效率/结果码，`app_logs` 用于排障，`audit_events` 用于管理员和安全动作，三者不互相替代。

Android 工具链安装和构建说明见 `../android/README.md`。如果机器尚未安装 JDK/Gradle/Android Studio，只能验证工程静态配置，不能宣称 APK 已构建成功。

## 6. 代码规范与视觉规范

- `web.py` 只做路由、会话和响应；`providers.py` 只做上游 URL/HTTP/usage 边界；`services.py` 做业务校验；`db.py` 做持久化和聚合。
- 注释解释“为什么这样做”和安全边界，不重复翻译代码；复杂的时间区间、Key 生命周期和 provider 回退逻辑必须有注释。
- UI 使用 Material 3 风格的 design tokens（surface、on-surface、outline、primary），同时保留黑底大排版和轨道视觉；所有表单状态提供 loading、成功、失败和空状态。
- 动效提供 `prefers-reduced-motion` 降级；键盘焦点必须可见，正文和表单文字优先保证可读性。

## 7. 时间和数据格式

- 所有记录按运行机器的本地墙上时间保存，格式为 `YYYY-MM-DD HH:MM:SS`。
- 周统计按本地周一至周日计算。
- 月统计按本地自然月计算。
- CSV 字段：`id`、`model`、`input_tokens`、`output_tokens`、`total_tokens`、`timestamp`、`note`、`source`。

## 8. 本地备份、清单与恢复

创建经过 SQLite 原生在线备份和完整性检查的本地副本：

```powershell
python -m token_tracker backup
```

默认输出到 `windows/data/backups/`。备份文件是敏感数据，管理员需要自行制定保留周期、离线存放和恢复演练策略；当前命令不执行云端同步，也不会删除旧备份。

只读盘点备份数量、年龄、容量，并按显式策略返回状态：

```powershell
python -m token_tracker backup-inventory `
  --min-count 1 `
  --max-age-days 7 `
  --max-size-mib 2048 `
  --verify
```

清单默认读取数据库旁的 `backups/`，不会自动创建目录。状态为 `pass` 时退出码为 0；目录缺失、
没有备份、策略不满足或完整性验证失败时输出 `attention` 并返回退出码 2。需要脚本消费时追加
`--json`；JSON 只包含文件名、大小、当地修改时间、年龄、策略和完整性状态，不包含数据库记录。
该命令永远不会自动删除旧备份；正式部署的保留周期、离线副本和责任人必须由管理员确认，详见
`docs/decisions/ADR-077-backup-inventory-retention.md`。

发布预检默认不检查备份，保证新安装可以直接体验；管理员确认备份策略后，可显式加入同一条只读预检链：

```powershell
.\release-doctor.ps1 -Mode Local `
  -CheckBackups `
  -MinBackupCount 1 `
  -MaxBackupAgeDays 7 `
  -MaxBackupSizeMiB 2048 `
  -VerifyBackups
```

`release-doctor.bat` 支持同样的参数。`-CheckBackups` 开启后，备份缺失、策略不满足或完整性检查失败会使
发布预检返回失败；不传该开关时不会扫描、创建或修改备份目录。

验证已有副本（只读，不改数据库）：

```powershell
python -m token_tracker verify-backup --path data/backups/token_tracker-YYYYMMDD-HHMMSS.sqlite3
```

恢复到明确的 staging 目标（默认拒绝覆盖已有文件，先验证后原子发布）：

```powershell
python -m token_tracker restore-backup `
  --path data/backups/token_tracker-YYYYMMDD-HHMMSS.sqlite3 `
  --target data/staging/token_tracker.sqlite3
```

只有确认目标可被替换时才追加 `--overwrite`；不要把恢复目标直接指向正在运行的生产数据库。

- provider adapter：兼容 Anthropic、Gemini、Responses API、Kimi Code 专用接口。
- 费用估算：按模型配置输入/输出单价，计算成本。
- 管理员邀请链接和邀请码注册。
- SQLite 导入和多设备同步。
- 反向代理级 HTTPS、Redis 限流和结构化审计日志。
