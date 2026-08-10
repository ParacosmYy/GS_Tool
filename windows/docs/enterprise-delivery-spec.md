# AI Token Tracker 企业级持续交付规格

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**状态：** Accepted / Living Specification  
**版本：** 1.0  
**日期：** 2026-08-10

## 1. Objective

AI Token Tracker 是一个本地优先、可分享部署的 AI 使用观测平台。它记录不同
AI provider 和模型的 token 消耗，并把 token 事实、工作方向、效率信号、正确/错误码、
运行日志和管理员审计分成独立数据边界。

长期交付目标是：

- Windows 端提供中心服务、Web 仪表盘、CLI、SQLite 数据和一键体验入口。
- Android 端通过稳定的 `/api/v1` 契约使用同一账号，不直接访问数据库。
- 普通用户只能看到自己的数据；管理员通过服务端 RBAC 查看团队聚合和脱敏审计导出。
- 页面优先自动检测 provider 模型和 usage；外部服务不提供 usage 时明确提示，不猜测 token。
- 所有模块可以独立维护、验证和替换，文件不超过 1000 行，接口边界先于实现。

“字节跳动同级”在本项目中解释为公开工程原则：模块化单体优先、契约先行、
高内聚低耦合、可观测、质量门禁、渐进式演进；不声称复制任何公司的内部制度或代码。

## 2. Tech Stack and deployment

| 层 | 当前基线 | 演进方向 |
|---|---|---|
| Windows service | Python 3 + Flask + Waitress | Go API 仅在规模和运维收益明确后承接 |
| Web | Flask templates + vanilla JavaScript + Chart.js | Vue 3 只替换 Presentation 层 |
| Mobile | Kotlin + Jetpack Compose | 通过 `/api/v1` 保持服务端边界 |
| Storage | SQLite、本地备份、参数化 SQL | PostgreSQL 迁移前先完成并发/回滚演练 |
| Provider | OpenAI-compatible adapter registry | Kimi/Anthropic/Gemini 等专用 adapter |

当前部署是“代码前后端分离、同一进程部署”的模块化单体，不是独立 Web 和 API
服务器。个人体验使用 `start.bat`；多人分享必须使用 HTTPS、生产 WSGI、持久化磁盘、
RBAC、限流和备份策略。

## 3. Commands and evidence

从项目根目录执行：

```text
双击 start.bat（默认打开 5000；若 5000 被占用，入口会打开当前源码自动选择的可用端口）

cd windows
python -m token_tracker --help
python -m compileall token_tracker
python run.py
```

交付验证不把“能导入”当作完整证明。每个切片至少留下：

- 变更文件和依赖方向；
- API/数据字段和错误语义；
- Python/JavaScript/Kotlin 编译或语法诊断；
- CLI、健康检查、HTTP 响应头和关键请求的运行证据；
- Web 变更的真实浏览器截图、DOM/ARIA、控制台和响应式记录；
- Android 若缺 JDK/Gradle/SDK，只记录静态配置和工具链门禁，不伪造 APK 成功。

项目约束禁止默认创建、修改或运行单元测试、mock、fixture、测试 harness 和测试专用
资源。因此当前以非破坏性静态、运行和人工交互证据为主；未来若建立测试体系，必须单独
立项并先更新约束。

## 4. Project structure and dependency direction

```text
ai-token-tracker/
├── start.bat                         # 根目录体验入口
├── windows/
│   ├── token_tracker/
│   │   ├── web.py                    # HTML/session controller
│   │   ├── api_v1.py                 # versioned API controller
│   │   ├── services.py               # usage application use cases
│   │   ├── events.py                 # event/log/audit use cases
│   │   ├── provider_service.py       # provider application boundary
│   │   ├── providers.py              # provider ports/adapters
│   │   ├── db.py                     # SQLite infrastructure/repository
│   │   └── api_contract.py            # shared response contract
│   ├── templates/                    # semantic page structure
│   ├── static/                       # API client, charts, motion, visual tokens
│   ├── docs/                         # specs, ADRs and operational notes
│   ├── tasks/                        # executable delivery plan and checklist
│   ├── roles/                        # role-specific review packages
│   └── skills/                       # project-scoped skill snapshots
└── android/
    └── app/src/main/java/            # Compose feature/data/ui layers
```

依赖方向固定为：

```text
UI / Android / CLI
        ↓
HTTP controller / presentation
        ↓
Application use case / policy
        ↓
Domain contract / provider port
        ↓
Repository / provider adapter / local infrastructure
```

上层不得拼接 SQL、持久化 Key 或复制校验规则；低层不得导入 Flask request、session、
模板或 Compose。每个公共接口必须明确输入、输出、权限、失败语义和幂等不变量。

## 5. Code style and module contract

新增源文件必须包含 `Author`、`Maintainer`、`Purpose` 和模块归属。注释解释设计原因、
安全边界和不可变约束，不逐行翻译代码。Python 公共函数使用类型标注和 docstring；
JavaScript 模块只暴露最小函数；Kotlin 公共 API 使用 KDoc。

```python
def get_usage_summary(
    user_id: int,
    period: str,
    date_from: str | None,
    date_to: str | None,
    path: str,
) -> dict[str, object]:
    """Return one user's bounded summary; never cross the user boundary."""
```

路由只解析请求、调用用例、映射稳定响应和状态码。应用用例负责验证、授权策略、
幂等和组合查询。基础设施负责参数化 SQL、事务、索引、备份和数据投影。

## 6. Security and data boundaries

- 密码只保存 scrypt 哈希；access/refresh 原文不入库，数据库只保存 digest。
- API Key 只存在当前请求内存，不写 SQLite、日志、CSV、localStorage 或响应日志。
- provider 输入使用字段级资源预算：Key 4096、Base URL 2048、模型名 200、消息 100 项。
- 所有跨用户查询必须带服务端 user id；admin 权限由服务端再次判断。
- 所有外部 JSON、URL、模型 ID、事件和日志字段都在边界校验并限制长度。
- provider 请求使用 HTTPS、白名单、超时、禁止重定向和响应大小上限。
- 写请求使用会话 CSRF 或 bearer 认证；认证接口限流；错误响应不暴露 traceback。
- 管理员查看、导出、角色变更和安全动作写入审计事件。

## 7. Success criteria

长线目标只有在以下证据全部成立后才可宣称完成：

1. Windows 端个人记录、自动 provider 采集、汇总、CSV、登录和管理员观测台可运行。
2. `/api/v1` 契约被冻结并被网页、Android 和未来采集器一致消费。
3. 普通用户、管理员、未登录用户和过期令牌的权限边界经运行请求验证。
4. Android 完成工具链批准后的可复现构建、设备联调、安装和 APK 产物验收。
5. Web 五个 UI 模块完成 320/768/1024/1440 响应式、可访问性、动效和性能证据。
6. 备份恢复、HTTPS、限流、脱敏日志、发布回滚和部署文档完成演练或明确记录未执行项。
7. 每个文件小于 1000 行，代码审查、架构决策、变更记录和交付清单保持同步。

## 8. Open questions and approval gates

- 何时从 SQLite 迁移 PostgreSQL：以并发写入、备份恢复和共享部署指标触发，不以概念升级触发。
- 何时拆 Go/Vue：以独立发布、性能或团队边界的真实收益触发，先冻结 API 再迁移。
- Android 工具链下载、设备安装和正式公网部署属于需要明确授权的外部状态变更。
- 生产 HTTPS 域名、管理员初始账号策略和正式备份保留周期由项目负责人确认。
