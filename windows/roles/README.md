# 六角色责任模型与执行轨道

**作者：** AI Token Tracker Engineering Team
**维护者：** Project Owner
**原则：** 以 3 个 UI 角色、1 个前端角色、1 个后端角色和 1 个架构师作为正式责任人；细分执行轨道只能服务于责任人，不改变对外的责任边界。所有角色在当前 checkout 集成，不创建 Git/Codex worktree。

## 正式责任模型

用户要求的企业级交付模型固定为六个 accountable roles：

| 角色 | 责任人边界 | 规范入口 | 主要交付 |
|---|---|---|---|
| UI-1 视觉系统负责人 | 品牌视觉、设计 token、排版、组件层级 | `01-ui-director`、`01-ui-visual` | 视觉基线、色彩可读性、场景资产和视觉验收 |
| UI-2 动效交互负责人 | 过渡、鼠标跟随、状态机、性能降级 | `02-ui-motion` | motion contract、交互节奏、reduced-motion 方案 |
| UI-3 响应式可访问性负责人 | viewport、键盘、ARIA、焦点、对比度 | `03-ui-accessibility` | WCAG 证据、错误/空状态、真实设备验收 |
| FE 前端负责人 | 模板、CSS/JS、API client、浏览器状态 | `04-frontend` | Web 页面、跨端 API 消费、前端发布证据 |
| BE 后端负责人 | Flask/API、用例、SQLite、Provider、安全 | `05-backend` | 服务端契约、数据隔离、自动采集和运维接口 |
| ARCH 架构师 | 依赖方向、ADR、集成、质量和最终闸门 | `06-architect` | 架构决策、接口治理、发布矩阵、最终签署 |

角色名称和顺序是项目约束，不因当前实现使用 Flask、Compose 或未来迁移 Go/Vue 而改变。架构师可以
拆出执行任务，但不能把责任重新扩展成没有明确 owner 的平行开发线。

## 执行子轨道

为保持高内聚低耦合，正式角色下允许以下实现子轨道；它们不是新的 accountable role：

| 责任角色 | 执行子轨道 | 范围 |
|---|---|---|
| FE | `02-dev-shell-auth`、`03-dev-observatory`、`04-dev-connect`、`05-dev-runtime` | Shell/Auth、观测台、连接器、历史与运行时 |
| ARCH | `06-architect-system`、`07-architect-delivery` | 系统契约、安全演进、证据和发布 |
| UI-1/UI-2/UI-3 | `01-ui-director` 与对应规范目录 | 视觉、动效、响应式和无障碍协作 |
| BE | `05-backend` | 服务端领域、基础设施和数据边界 |

子轨道只能通过稳定的 API、类型契约、CSS/JS 模块接口或 ADR 交付，不复制数据库查询、权限规则或
Provider 分支。共享运行代码最终由 ARCH 集成回当前 checkout。

## 合并顺序

```text
ARCH 契约确认
    ↓
BE 固化 API / 数据 / 安全边界
    ↓
FE 消费稳定接口
    ↓
UI-1 视觉 → UI-2 动效 → UI-3 响应式与可访问性
    ↓
ARCH 五轴审查、发布证据和回滚确认
```

每个角色交付包必须说明变更目的、触及文件、契约影响、风险、验证证据和未完成项。任何文件超过
1000 行、缺少企业级头部或绕过责任边界，均不得进入最终集成。

## 规范资料

- [`docs/architecture.md`](../docs/architecture.md)：模块与依赖方向。
- [`docs/api-contract.md`](../docs/api-contract.md)：Web/Android `/api/v1` 契约。
- [`docs/role-review.md`](../docs/role-review.md)：六角色集成复核。
- [`docs/decisions/ADR-084-six-accountable-roles.md`](../docs/decisions/ADR-084-six-accountable-roles.md)：角色模型决策。
