# ADR-0139：命令管理页选择与动作分行

- 状态：Accepted for ARCH-88 / UI-1.161
- 日期：2026-08-12
- 范围：`presentation/controllers/command_workspace_builder.py`、`terminal.py`、`workspace.py`

## 背景

命令管理页的批量命令下拉框与新建、编辑、删除、执行、停止五个按钮曾放在同一条横向 row。
在窄窗口中它们共同争抢最小宽度，降低选择器可读性，也让动作层级显得拥挤；`terminal.py` 同时
承担错误、历史和批量命令构造，职责边界不清晰。

## 决策

1. `command_workspace_builder.py` 成为命令管理页的唯一构建 owner，负责历史、批量选择、动作、
   状态、结果表和空态 CTA。
2. 批量区拆为 selector row 与 actions row；动作行保留 trailing stretch，允许外层 scroll 承载纵向
   增长，避免横向压缩。
3. `terminal.py` 只保留错误通知；`workspace.py` 直接把 builder 返回的 layout 放入原有 commandPage。
4. `CommandBatchControlBindings` 和所有既有 callback/accessibility/Tab order/close/motion 边界不变。

## 验证与风险

Ruff、compileall、scripts/check、import contract、source-limit、theme audit 已通过；命令 action 和
empty CTA 静态检索各保留一处。真实 GUI geometry、键盘焦点、三主题截图、显示器 FPS、硬件/HIL 未运行，
不把静态结果解释为运行时验收。ARCH-88 架构师调用在服务窗口内超时并关闭，父代理 fresh-pass
Required=0。Python/PySide6 presentation-only，embedded vendor-source applicability 为 N/A。
