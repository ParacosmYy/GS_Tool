# AI Token Tracker

> 本地优先（local-first）的 AI Token 用量追踪平台。Windows 端承担中心服务、SQLite 账本与 Web 控制台；Android 端通过统一的 `/api/v1` 契约登录、查看与写入用量数据。管理员可查看成员汇总、用量明细、工作事件、错误码与脱敏日志导出。

![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Android-blue)
![License](https://img.shields.io/badge/license-Proprietary-yellow)
![Backend](https://img.shields.io/badge/backend-Flask%20%2F%20SQLite-005571)
![Client](https://img.shields.io/badge/client-Kotlin%20%2F%20Jetpack%20Compose-4285F4)

---

## 1. 项目简介

AI Token Tracker 解决跨客户端（网页、CLI、Android、外部 SDK）AI Token 用量无法统一入账的问题。它以「本地账本 + 中心服务」为基线：

- **Windows 中心服务**：Flask 提供 Web 控制台与 `/api/v1` REST 契约，SQLite 持久化账本，内置 CLI（`token_tracker`）用于账户、角色、Gateway Token 管理，并可将服务打包为 EXE。
- **Android 客户端**：Kotlin + Jetpack Compose 实现，通过同一套 `/api/v1` 契约登录、查看与写入数据。
- **Gateway（网关）**：本地代理外部兼容客户端（Kimi Code、OpenAI SDK 等）的 Base URL，自动从真实 `usage` 中提取并入账，无需手工填写。
- **安全边界**：个人入口 fail-closed 绑定 `127.0.0.1`；正式分享必须走 HTTPS + 长随机 Session Secret + 备份/恢复策略。

> 原项目索引（含各子模块入口）已归档至 [`windows/docs/README.project-index.md`](windows/docs/README.project-index.md)。

## 2. 目录结构

```text
ai-token-tracker/
├─ windows/                # Windows 中心服务：Flask / SQLite / CLI / Web UI / EXE / 部署
│  ├─ token_tracker/       # 应用代码（web.py · api_v1.py · cli.py · services · events · db ...）
│  ├─ ui-modules/          # 5 个前端 UI 模块：01-shell / 02-auth / 03-observatory / 04-connect / 05-history
│  ├─ roles/               # 角色化协作目录（UI 总监 / DEV / ARCH）
│  ├─ skills/              # 项目级技能与上游技能快照
│  ├─ docs/                # 企业架构、发布就绪、角色评审、ADR、索引
│  ├─ deployment/          # HTTPS（Caddy）部署配置（Caddyfile 已 gitignore）
│  ├─ packaging/           # EXE 打包配置
│  ├─ ci/                  # CI 脚本
│  ├─ data/                # 本地 SQLite 账本（gitignored，不入库）
│  ├─ run.py               # Windows 运行入口
│  └─ README.md            # Windows 服务与 CLI 文档
├─ android/                # Kotlin + Jetpack Compose 客户端
│  ├─ app/                 # 应用模块
│  ├─ gradle/ · gradlew    # Gradle 构建工具链
│  ├─ build.gradle.kts     # 模块构建脚本
│  └─ README.md            # Android 构建与联调
├─ run.py                  # 根目录 Python 体验入口（仅转发至 windows/run.py）
├─ start.bat               # 根目录一键体验入口
└─ README.md               # 本文件（企业级总览）
```

## 3. 架构说明

采用「代码前后端分离、同一 Flask 进程部署」的企业级模块化单体（modular monolith），分层基线如下：

```text
Browser / Android / CLI
        │
        ▼
Presentation:  web.py · api_v1.py · cli.py
        │  stable DTO + error envelope（稳定数据契约与错误信封）
        ▼
Application:   services.py · events.py · admin_data.py · mobile_auth.py
        │
        ├── Provider boundary: providers.py   （外部 provider 边界）
        ▼
Infrastructure: db.py ── SQLite
```

关键约束：

- **依赖方向固定**：UI → API client / HTTP contract → route/controller → service/use case → repository/provider；低层模块不得反向依赖 UI 或请求上下文。
- **契约先行**：跨端字段、状态、错误码、事件必须先进入契约文档，再由 Windows 网页与 Android 分别实现；禁止客户端私自发明同名字段的另一种含义。
- **安全靠代码强制**：认证、校验、参数化 SQL、HTTPS、密钥隔离由代码保证，注释只补充原因。
- **单文件硬门禁**：任何源文件不得超过 **1000 行**；接近 800 行即需拆分，拆分后能力必须通过稳定函数/类/模块导出或 HTTP 契约暴露。

## 4. 环境依赖

| 组件 | 依赖 | 说明 |
|---|---|---|
| Windows 服务 | Python 3.11+（建议虚拟环境） | 详见 `windows/requirements.txt` |
| Web 框架 | Flask | 内置开发服务器 + SQLite |
| 数据库 | SQLite（Python 标准库） | 账本落盘于 `windows/data/` |
| Android 客户端 | JDK 17+ · Android SDK (API 34) · Gradle 8.x · Kotlin 1.9+ | `android/` 目录内构建 |
| 网关接入 | Provider Key 配置 | 外部客户端 Base URL 指向 `http://127.0.0.1:8787/v1` |
| 正式部署 | Caddy（反向代理 + TLS） | 见 `windows/deployment/` |

## 5. 部署方式

### 5.1 本地体验（个人）

双击根目录 `start.bat`，或在根目录执行：

```powershell
windows\.venv\Scripts\python.exe run.py
```

默认地址 `http://127.0.0.1:5000`（端口被占用时自动选择附近空闲端口），自动打开浏览器。

### 5.2 局域网预览（仅可信网络）

使用文档中的显式 `--lan-preview` 参数；**禁止**通过 `TOKEN_TRACKER_HOST` 隐式把账本暴露到局域网。

### 5.3 正式 HTTPS 部署

- 使用 Caddy 反向代理并启用 TLS；`windows/deployment/` 提供配置模板（实际 `Caddyfile` 因含密钥已 gitignore）。
- 必须配置真实长随机 Session Secret、备份/恢复策略与受控访问日志。
- 纯 HTTP LAN 预览仅适合可信网络，不得作为生产边界。
- EXE、APK 与正式 Caddy/HTTPS 产物需在批准的外部工具链/部署主机完成，不会在未批准时自动下载、安装或伪造产物。

详细步骤见 [`windows/deployment/README.md`](windows/deployment/README.md) 与 [`windows/packaging/README.md`](windows/packaging/README.md)。

## 6. 使用说明

### 6.1 注册与管理员

首次进入网页后注册账户；将账户设为管理员：

```powershell
cd D:\Workplace\Agent_Workplace\ai-token-tracker\windows
.\.venv\Scripts\python.exe -m token_tracker admin set-role --username your-name --role admin
```

### 6.2 网页自动采集

网页「自动采集」在模型检测与调用完成后读取上游真实 `usage` 并自动入账，无需日常手工填写。

### 6.3 外部客户端接入（Gateway）

让独立客户端（Kimi Code、OpenAI SDK 等）的 Base URL 指向本地 Gateway：

```powershell
cd D:\Workplace\Agent_Workplace\ai-token-tracker\windows
python -m token_tracker ingest-token create --username your-name --label kimi-code --expires-days 90
# 将命令只显示一次的 ait_... 写入 windows/.env，并设置：
# TOKEN_TRACKER_GATEWAY_PROVIDER_KEY=<你的 provider key>
# TOKEN_TRACKER_GATEWAY_INGEST_TOKEN=<上一步的 ait_...>
.\start-gateway.bat -AllowHttp
```

随后将兼容客户端 Base URL 改为 `http://127.0.0.1:8787/v1`。`-AllowHttp` 仅用于本机调试；跨设备/正式服务必须改为 HTTPS。

## 7. 贡献规范

- **提交规范**：遵循 Conventional Commits（`feat:` / `fix:` / `docs:` / `refactor:` / `test:` / `chore:`）。
- **代码规模门禁**：单文件 ≤ 1000 行（硬门禁），接近 800 行即拆分。
- **文件头元信息**：每个源文件必须包含 `Author` / `Maintainer` / `Purpose` / 所属模块；公共函数、类、接口、路由、API 响应须说明输入、输出、失败语义、权限要求与不可变不变量。
- **契约先行**：跨端字段、状态、错误码、事件先入契约文档，再分别实现。
- **安全边界**：靠代码强制，不依赖注释；API Key 仅留页面内存，禁止写入 localStorage / SQLite / 日志 / CSV / 响应正文。
- **禁止入库**：`.env`、SQLite 数据库、真实 API Key、密码、日志中的敏感信息。
- **工作区限制**：只在当前本地 checkout 工作；不创建或操作 Git/Codex worktree；不提交单元测试/mock/fixture；不宣称认证、合规或生产安全已完成（须说明实际验证范围）。

---

> 详细子模块文档入口：
> - Windows 服务与 CLI：[`windows/README.md`](windows/README.md)
> - Android 构建与联调：[`android/README.md`](android/README.md)
> - 企业级架构：[`windows/docs/enterprise-architecture.md`](windows/docs/enterprise-architecture.md)
> - 发布就绪审计：[`windows/docs/release-readiness.md`](windows/docs/release-readiness.md)
