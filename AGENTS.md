# AI Token Tracker 项目约束

本文件是本项目的角色分工、架构边界、视觉目标和交付闸门。它适用于本目录及其子目录；修改项目时必须先遵守这里的约束，再遵守工作区级 `AGENTS.md`。

## 一、角色分工

本项目固定按 **1 名 UI 美化师 + 4 名开发者 + 2 名架构师** 的七个角色槽位协作。角色可以由同一个执行者在不同阶段承担，但交付物必须按角色分别检查，不能用“已经写过代码”替代其他角色的审查。五个 UI 模块是交付边界，不是 Git/Codex worktree；所有集成仍回到当前 checkout。

### UI-1：视觉与动效总监（唯一 UI 美化师）

- 负责品牌基调、色彩、字体、排版比例、空间层级、轨道/光场、鼠标跟随和全站动效语言。
- 视觉目标：借鉴 Moonshot AI 官网的黑底、极简灰阶、超大排版、细线框、强留白和空间运动，但不复制商标、Logo、文案或受版权保护的资源。
- 每次视觉变更都必须检查五个 UI 模块的统一 token、桌面首屏、数据区、表单区、表格区和空状态。
- 不能把动效数量当成质量；每个循环动效都要说明它表达的信息、持续时间、降级策略和性能成本。

### DEV-1：Shell / Auth 开发者

- 负责 `windows/ui-modules/01-shell` 与 `windows/ui-modules/02-auth`，落地 `base.html`、全局 token、登录/注册/错误页、认证动效和入口体验。
- 只消费架构师批准的页面状态和 API 契约；不改变认证安全边界。

### DEV-2：Observatory / Analysis 开发者

- 负责 `windows/ui-modules/03-observatory`，落地仪表盘首屏、总量轨道、趋势/模型图表、滚动 reveal 和数据加载状态。
- 业务数据只能通过 JSON API 进入页面，图表不得读取 SQLite 或猜测 token。

### DEV-3：Connect / Frontend State 开发者

- 负责 `windows/ui-modules/04-connect`，落地自动检测模型、代理调用、Key 生命周期、表单状态、成功/失败/空状态和 API client 编排。
- API Key 只允许留在当前页面内存；不得写入 localStorage、SQLite、日志、CSV 或响应正文。

### DEV-4：History / Runtime 开发者

- 负责 `windows/ui-modules/05-history`，落地记录、导出、响应式、键盘/ARIA、性能预算和根目录启动体验。
- 负责跨模块的 320px、768px、1024px、1440px 验收记录，但不能借此绕过 UI 总监的视觉评审。

### ARCH-1：系统架构师

- 负责 API 契约、模块依赖、认证/数据边界、Flask 当前部署和未来 Vue/Go 演进路线。
- 当前架构是“前后端代码分离、同一 Flask 进程部署”的企业级模块化单体；分层基线见 `windows/docs/enterprise-architecture.md`：

  ```text
  Browser / Android / CLI
      │
      ▼
  Presentation: web.py / api_v1.py / cli.py
      │ stable DTO + error envelope
      ▼
  Application: services.py / events.py / admin_data.py / mobile_auth.py
      │
      ├── Provider boundary: providers.py
      ▼
  Infrastructure: db.py ── SQLite
  ```

- 若未来拆成 Vue 独立前端或 Go 后端，必须先保留 `/api/v1` 契约，再更新 CORS、会话、CSRF、部署和迁移策略。

### ARCH-2：交付与质量架构师

- 负责五模块交付顺序、跨角色依赖、性能/可访问性/浏览器证据、回滚边界和最终集成闸门。
- 不重复实现业务代码；只通过契约、审查记录和可复现证据推进交付。

### 五个 UI 模块与唯一开发归属

| 模块目录 | 产品范围 | 主开发者 | UI 总监评审 | 架构师门禁 |
|---|---|---|---|---|
| `windows/ui-modules/01-shell` | 全局布局、字体、token、光场、鼠标跟随、页面转场 | DEV-1 | 必须 | ARCH-1/2 |
| `windows/ui-modules/02-auth` | 登录、注册、错误页、认证表单状态 | DEV-1 | 必须 | ARCH-1 |
| `windows/ui-modules/03-observatory` | 首屏、总量轨道、趋势、模型占比、加载状态 | DEV-2 | 必须 | ARCH-1/2 |
| `windows/ui-modules/04-connect` | 模型检测、自动代理、Key 反馈、异常补录 | DEV-3 | 必须 | ARCH-1 |
| `windows/ui-modules/05-history` | 历史记录、导出、响应式、可访问性、运行时性能 | DEV-4 | 必须 | ARCH-2 |

开发者允许修改自己模块的共享样式/脚本，但必须在交付说明中列出影响面；同一文件不得由多个角色同时作为未声明的写入目标。

### 文件规模与接口硬门禁

- 任何项目文件不得超过 **1000 行**；这是集成前的硬门禁，不是建议值。
- 接近 800 行时就要开始拆分；超过 1000 行必须先按高内聚职责拆成新文件，再继续添加功能。
- 拆分后的能力必须通过稳定的函数、类、模块导出或 HTTP/API 契约暴露；禁止通过循环导入、全局变量、复制代码或隐式 DOM 选择器耦合模块。
- `web.py` 负责 HTTP 编排，业务用例、provider、数据库和管理员查询必须独立模块；前端 `app.js` 只负责页面编排，API、图表和 motion 分开。
- 每次交付必须记录受影响文件的行数、对外接口和依赖方向；代码审查发现超限时不得合并。

### 企业级代码注释与架构纪律

- 每个源文件顶部必须有统一元信息：`Author`、`Maintainer`、`Purpose`、所属模块；新文件不能省略。
- Python 模块用模块 docstring；Java/Kotlin 用文件级 KDoc 或类 KDoc；JavaScript/TypeScript/CSS/HTML 用文件头注释；配置文件用注释说明版本和边界。
- 公共函数、类、接口、路由和 API 响应必须说明输入、输出、失败语义、权限要求和不可变不变量；实现细节不重复写成无价值注释。
- 注释解释“为什么这样设计”和“什么不能做”，不翻译下一行代码，不保留注释掉的旧实现，不写无期限 TODO。
- 安全边界必须靠代码强制，注释只能补充原因，不能代替授权、校验、参数化 SQL、HTTPS 或密钥隔离。
- 依赖方向固定：UI → API client/HTTP contract → route/controller → service/use case → repository/provider；低层模块不得反向依赖 UI 或请求上下文。
- 任何跨端字段、状态、错误码和事件都必须先进入契约文档，再由 Windows 网页和 Android 分别实现；禁止客户端私自发明同名字段的另一种含义。

## 二、Moonshot 风格 UI 约束

- 页面底色优先使用近黑色；内容使用低对比灰阶，关键数据用单一高亮色，不使用浅绿后台模板风格。
- 首屏必须有内容叙事，不把 4 个相同统计卡片当作主要视觉；总 token 是中心 signal，输入/输出/模型围绕它展开。
- 使用大字号排版、细边框、黑色面板、留白和线性分隔；圆角、阴影和渐变只能承担层级作用，不能堆叠制造“AI 模板感”。
- 动效至少包含：页面入场、中心轨道/光环、标题轻微 glitch、滚动 reveal、数字变化和 hover/focus 过渡；每个动效都要有静态降级。
- 可借鉴 `https://www.moonshot.ai/` 的视觉语言，但不得抓取、复制或提交对方品牌资源。实现应保持 AI Token Tracker 的独立品牌。

## 三、实现顺序与交付闸门

每个有实质影响的功能按以下顺序执行，并在交付说明中标注证据：

1. 架构师：明确影响的模块、API、数据和风险。
2. UI-1/2/3：明确视觉、动效、响应式和无障碍验收点。
3. 后端：先完成契约和边界校验，再实现数据/API。
4. 前端：消费契约，完成状态、交互和视觉实现。
5. 角色复核：分别检查视觉、动效、响应式/可访问性、前端边界、后端安全和架构一致性。
6. 集成证据：浏览器截图、DOM 状态、控制台信息、CLI 帮助/启动、编译诊断或现有可用构建证据。

## 项目级 skills

- 可复用的项目技能放在 `windows/skills/`，不得只安装到全局 Codex 配置后声称项目已具备。
- UI 工作必须先读取 `windows/skills/project-ui-orchestration/SKILL.md`，并按其引用的上游技能执行。
- 技能来源、快照日期和升级影响写在 `windows/skills/README.md`；技能文件不允许绕过本项目的 1000 行、注释、安全和验证约束。

## 四、工作区限制

- 只在当前本地 checkout 工作；不得创建或操作 Git/Codex worktree。
- 不创建、修改、生成或默认运行单元测试、mock、fixture、测试 harness 或测试专用资产。
- 不闪存、擦除、部署或操作目标硬件；本项目只做本地 Web/CLI 非破坏性验证。
- 不提交 `.env`、SQLite 数据库、真实 API Key、密码或日志中的敏感信息。
- 不宣称认证、合规或生产安全完成；必须说明实际验证范围和未运行项目。
