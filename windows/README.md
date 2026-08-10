# AI Token 用量追踪器

一个企业级模块化单体 AI token 观测平台：Windows 端集中承载 Web、CLI、SQLite 和管理员审计；浏览器与 Android 通过稳定的 `/api/v1` 契约接入。个人记录、团队汇总、工作事件、诊断日志和 CSV 导出分别建模。

## 一键打开（个人体验入口）

在项目根目录双击 `start.bat` 即可。根入口会转到 `windows/start.bat`，自动检查 `windows/.venv`、安装依赖、创建 `windows/.env`（如果不存在）、启动本机 Web 服务并打开浏览器。

默认地址：<http://127.0.0.1:5000>。如果 5000 已被其他本地服务占用，`run.py` 会在
5001–5020 中选择第一个可用端口，并让浏览器打开当前源码对应的实际地址；已有服务不会被终止。

`run.py` 是同一个入口的 Python 版本，适合在终端启动或调试。这个入口默认只监听本机，不会把个人数据暴露给局域网；需要分享时再按下面的部署说明显式使用 `--host 0.0.0.0`。

## 能做什么

- 记录模型名称、输入 token、输出 token、本地时间、备注和来源。
- SQLite 持久化，数据默认保存在 `data/token_tracker.sqlite3`。
- CLI 支持初始化、添加、按周期汇总、按日期范围汇总、CSV 导出和启动 Web。
- Web 支持注册/登录，每个普通用户只能看到自己的记录；管理员通过服务端 RBAC 查看团队汇总、指定成员明细、结构化事件和脱敏导出。
- Android 使用同一账号的 `/api/v1/auth/*` bearer token；SQLite 永远只在 Windows 服务端，移动端不接触数据库文件。
- 工作方向、效率评分、正确/错误码、request id、应用日志和管理员审计事件分开保存，避免把不同生命周期的数据揉成一张表。
- 以自动采集为主：填一次 Base URL + API Key，先自动检测模型，再发起真实调用并从响应 `usage` 自动记账；手动录入只保留为异常补录。
- 仪表盘显示总 token、输入/输出、模型占比、最近趋势和最近记录。

架构、API 契约、UI 动效规范和关键决策见：

- [`docs/architecture.md`](docs/architecture.md)
- [`docs/api-contract.md`](docs/api-contract.md)
- [`docs/ui-motion-spec.md`](docs/ui-motion-spec.md)
- [`docs/design-tokens.md`](docs/design-tokens.md)
- [`docs/motion-contract.md`](docs/motion-contract.md)
- [`docs/engineering-standards.md`](docs/engineering-standards.md)
- [`roles/README.md`](roles/README.md)
- [`skills/README.md`](skills/README.md)
- [`tasks/todo.md`](tasks/todo.md)
- [`docs/decisions/`](docs/decisions/)

## 1. 安装

建议使用 Python 3.11 或更高版本。PowerShell 示例：

```powershell
cd D:\Workplace\Agent_Workplace\ai-token-tracker\windows
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install -r requirements.txt
Copy-Item .env.example .env
```

根目录 `start.bat` 会自动创建同一个 `windows/.venv`，并把 pip 缓存固定到 `windows/.cache/pip`；运行依赖不会安装到系统 Python。

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

## 3. 启动 Web 仪表盘

本机开发：

```powershell
python -m token_tracker serve
```

浏览器打开启动窗口打印的实际地址（通常是 <http://127.0.0.1:5000>），注册账户即可使用。

启动前进行只读配置预检：

```powershell
python -m token_tracker preflight
python -m token_tracker preflight --production
```

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
TOKEN_TRACKER_ALLOWED_BASE_URLS=https://api.moonshot.cn/v1,https://api.openai.com/v1
```

不要直接把 Flask 开发服务器暴露到公网，也不要把 `.env`、SQLite 文件或真实 API Key 提交到 GitHub。正式部署还应在反向代理或网关层增加 HTTPS、访问日志脱敏和更严格的限流。

## 4. 如何自动接入 Kimi / 其他模型

Web 页面里的“调用并自动记账”面向 OpenAI-compatible 的 `POST /chat/completions` 接口：

1. 在 `.env` 的 `TOKEN_TRACKER_ALLOWED_BASE_URLS` 写入服务的 Base URL；当前自动适配器使用 OpenAI-compatible 的 `/models` 与 `/chat/completions` 形状。
2. 在页面的“自动采集”区域输入 Base URL 和自己的 API Key，点击“自动检测模型”。
3. 系统请求上游 `/models`，将返回的模型自动放入可选列表；你不需要手动填写输入/输出 token。
4. 输入 prompt 后点击“发送 · 自动记账”。服务端读取响应中的 `usage.prompt_tokens` 和 `usage.completion_tokens`，优先使用响应里实际返回的模型归档。
5. API Key 只存在于当前页面内存，页面刷新即清除；前端只持久化 Base URL 和最近选择的模型，不保存 Key，不写入 SQLite。

Android 使用同一套 provider 能力，但通过 bearer 版本接口调用：`POST /api/v1/provider/models` 和 `POST /api/v1/proxy/chat/completions`。Android 不接触 Windows SQLite，只接收模型列表、助手文本、usage 摘要和记录投影；调用请求携带 `Idempotency-Key`，网络重试不会重复计数。

当前代理先支持非流式请求，因为流式响应通常需要额外处理 usage 汇总。没有返回 usage 的 provider 不会被伪造记账，页面会提示使用折叠的异常补录。

注意：Kimi Code 等产品的实际接入地址、可用模型和计费口径可能与公开 Kimi API 不同，应以对应账户/产品的官方文档为准；如果它不是 OpenAI-compatible 接口，可以继续添加专用 provider adapter。

如果希望 Kimi Code、OpenAI SDK 或其他外部客户端也自动进入账本，调用必须经过本项目的代理（或增加对应客户端插件/适配器）。任何网页都不能凭空读取另一个进程或云产品的调用记录；代理模式是自动采集的边界。当前页面已经把“检测模型 → 发起调用 → 读取 usage → 入库 → 刷新图表”串成一条自动链路，不要求日常填写 token 数。

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

## 8. 本地备份与后续可扩展方向

创建经过 SQLite 原生在线备份和完整性检查的本地副本：

```powershell
python -m token_tracker backup
```

默认输出到 `windows/data/backups/`。备份文件是敏感数据，管理员需要自行制定保留周期、离线存放和恢复演练策略；当前命令不执行云端同步，也不会删除旧备份。

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
