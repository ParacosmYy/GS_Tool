# 项目级技能包

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**用途：** 随 `windows/` 服务端一起交付的 UI、接口、性能和质量工作流。

## 来源与边界

以下六个目录从 [addyosmani/agent-skills](https://github.com/addyosmani/agent-skills) 的 `skills/` 目录安装，当前核对 ref 为 `7676817c12a1317454ae3898a0c5c1eacf5dd3d5`（2026-08-10）。它们是项目资料，不是全局 Codex 配置；后续升级必须显式记录来源 commit/ref、变更摘要和影响面。

| 技能 | 用途 | 本项目落点 |
|---|---|---|
| `frontend-ui-engineering` | 生产级 UI、响应式、可读性、WCAG | 五个 UI 模块 |
| `browser-testing-with-devtools` | 真实浏览器 DOM、截图、网络、控制台证据 | UI/集成闸门 |
| `performance-optimization` | 前端动效、请求、数据库和运行时预算 | UI-1/DEV-4/ARCH-2 |
| `api-and-interface-design` | v1 契约、跨端字段、错误 envelope | ARCH-1 |
| `code-review-and-quality` | 多维质量审查和合并前门禁 | ARCH-2 |
| `documentation-and-adrs` | ADR、契约和工程决策记录 | ARCH-1/ARCH-2 |

项目专用组合规则见 [`project-ui-orchestration/SKILL.md`](project-ui-orchestration/SKILL.md)。它把上游通用技能绑定到五个模块、Moonshot-inspired 视觉目标和企业级注释要求。

## 使用顺序

1. ARCH-1 先读 `api-and-interface-design` 与 ADR/契约。
2. UI-1 与 DEV-1/2/3/4 读 `frontend-ui-engineering` 和 `project-ui-orchestration`。
3. 修改浏览器代码后使用 `browser-testing-with-devtools` 采集真实证据。
4. ARCH-2 最后使用 `performance-optimization`、`code-review-and-quality` 和文档门禁。

技能只提供方法，不会替代项目约束；1000 行硬上限、禁止默认测试专用资产、密钥隔离和依赖方向以根目录 `AGENTS.md` 为最高优先级。
