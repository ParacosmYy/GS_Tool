# ADR-084：六个正式责任角色与执行子轨道

**作者：** AI Token Tracker Engineering Team  
**维护者：** ARCH / Project Owner  
**状态：** Accepted  
**日期：** 2026-08-10  
**作用：** 固化用户要求的 3 UI + 前端 + 后端 + 架构师企业级责任模型。

## 背景

项目早期为了拆分页面与发布工作，形成了“1 UI + 4 DEV + 2 ARCH”的七槽位文字；同时仓库里已经存在
UI-1/UI-2/UI-3、前端、后端和架构师的六类职责目录。两套说法会让新成员误以为每个执行轨道都拥有
独立的最终决策权，也不符合项目所有者明确提出的角色分工。

## 决策

1. 正式 accountable model 固定为六个角色：UI-1 视觉、UI-2 动效、UI-3 响应式/可访问性、FE 前端、
   BE 后端、ARCH 架构师。
2. `02-dev-shell-auth`、`03-dev-observatory`、`04-dev-connect`、`05-dev-runtime` 是 FE 的执行子轨道；
   `06-architect-system`、`07-architect-delivery` 是 ARCH 的执行子轨道。子轨道不新增 API owner、数据
   owner 或最终发布签署权。
3. 依赖顺序固定为 ARCH 契约 → BE 实现 → FE 消费 → UI 三角色复核 → ARCH 集成闸门。UI 反馈可以在
   任意阶段阻断合并，但不直接修改后端权限或数据库规则。
4. 旧目录和历史证据不删除；它们通过 `roles/README.md` 映射到六个正式角色，避免破坏已有引用。
5. 所有角色共享当前 checkout；隔离依靠目录、契约和 Git 提交，不依靠并行 worktree 或复制运行代码。

## 后果

- 每项交付都有明确责任人，页面拆分不会稀释架构师、前端或后端的最终边界。
- FE/ARCH 仍可以并行推进多个子任务，但 API、数据模型、provider 和安全规则只保留一个权威 owner。
- 现有七个实现目录继续可用，任务清单和审计必须引用六角色模型，防止新文档回退到七槽位表述。
- 角色数量本身不代表组织规模；未来扩容可以在责任角色内部增加执行者，但需要新增 ADR 才能改变 owner 边界。

## 验证

- `roles/README.md` 已列出六个正式角色与执行子轨道。
- `docs/role-review.md`、`tasks/plan.md`、`tasks/todo.md` 和项目级技能说明已同步。
- `token_tracker audit --json` 将 ADR、角色入口和契约引用纳入静态门禁。

