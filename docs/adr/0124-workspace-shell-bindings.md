# ADR 0124：Workspace shell typed bindings

状态：Accepted（ARCH-6q）  
日期：2026-08-11

## 背景

workspace 的 Tab、路线 beacon、滚动提示、专注按钮和动画 TabBar 原先以多个
`window._workspace_*` 动态字段在 controller 之间传递。字段本身没有业务语义，
但会扩大 MainWindow 的隐式 facade，并让启动顺序和生命周期边界难以审计。

## 决策

新增 `presentation/workspace_bindings.py` 的 frozen/slots
`WorkspaceShellBindings`，集中持有 `shell`、`tabs`、`tab_bar`、`route`、
`scroll_hint` 和 `focus_button` 六个 Qt widget 引用。`controllers/workspace.py`
是唯一组装 owner，完成组装后把 bundle 放入 window；bootstrap 只消费
`bundle.shell`。workspace runtime、专注过渡、焦点顺序、生命周期、派生数据和终端
runtime 通过 `workspace_bindings_for()` 消费 bundle。

## 边界

- bundle 是 presentation wiring DTO，不持有 ViewModel/application/domain 状态、timer、
  transport、导航策略或业务 callback；
- `workspace_bindings_for()` 不导入 MainWindow，启动早期 bundle 不存在时安全返回 `None`；
- Tab index、currentChanged、Tab 顺序、route/scroll hint/专注模式、共享 MotionController
  和一次性 transition 行为保持不变；
- 不改变 application/domain/API，不引入循环依赖；所有源码文件继续保持 ≤1000 行。

## 验证

真实 composition root 通过 bundle 类型、四 Tab、路线条层级、滚动提示、专注模式展开/恢复
和三主题 × 980/1180 × 四 Tab 共 24 组 vector；当前页 horizontal maximum=0，
exact-white/near-white=0。`compileall`、Ruff、source limit、theme token audit 通过。
架构师线程 `019fedd1-fa7c-72a3-81c4-96ac4cb0f808` 与独立审查线程
`019fedd6-db35-7d12-a58b-f7405b884fa9` 均在两个限定等待窗口内未返回并已关闭；
父代理完成 owner、生命周期、依赖方向、行为保持与简化审查。
