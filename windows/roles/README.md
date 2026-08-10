# 七角色独立交付区

**作者：** AI Token Tracker Engineering Team  
**原则：** 目录隔离交付内容，当前 checkout 统一集成；不创建 Git/Codex worktree。

## 当前角色模型

当前正式模型是 1 名 UI 美化师、4 名开发者、2 名架构师；交付模块位于 `windows/ui-modules/`。

| 目录 | 角色 | 主要交付 |
|---|---|---|
| `01-ui-director` | UI-1 视觉与动效总监 | 全局视觉、鼠标跟随、动效质量 |
| `02-dev-shell-auth` | DEV-1 | Shell、认证、错误页 |
| `03-dev-observatory` | DEV-2 | 首屏、图表、数据状态 |
| `04-dev-connect` | DEV-3 | 自动采集、API client、Key 状态 |
| `05-dev-runtime` | DEV-4 | 历史、响应式、性能、可访问性 |
| `06-architect-system` | ARCH-1 | API、数据、权限、安全、演进 |
| `07-architect-delivery` | ARCH-2 | 集成、证据、回滚、发布闸门 |

旧的 `01-ui-visual` 至 `06-architect` 目录保留为历史交付资料；新的变更必须写入当前七角色目录。

## 历史角色目录

| 目录 | 角色 | 主要交付 |
|---|---|---|
| `01-ui-visual` | UI-1 视觉系统负责人 | design tokens、排版、组件层级、视觉审查 |
| `02-ui-motion` | UI-2 动效与交互负责人 | motion contract、状态机、过渡、性能降级 |
| `03-ui-accessibility` | UI-3 响应式与可访问性负责人 | breakpoints、键盘、ARIA、焦点、可读性 |
| `04-frontend` | 前端工程师 | templates、static、API client、浏览器状态 |
| `05-backend` | 后端工程师 | Flask、services、db、providers、安全和数据隔离 |
| `06-architect` | 架构师 | contracts、ADR、依赖方向、集成和最终闸门 |

每个目录的 README 是角色输入/输出和验收协议。角色可以独立编写交付说明和设计资产，但共享运行代码只能在契约确认后汇总到 `token_tracker/`。

本轮集成复核记录见 [`docs/role-review.md`](../docs/role-review.md)；角色目录是隔离的交付边界，不是并行 checkout。

## 合并顺序

`06-architect` 先确认契约 → `05-backend` 固化接口 → `04-frontend` 消费接口 → UI-1/UI-2/UI-3 进行视觉/动效/可访问性复核 → 架构师最终集成。
