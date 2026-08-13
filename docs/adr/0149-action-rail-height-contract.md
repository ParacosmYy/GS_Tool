# ADR-0149：共享动作按钮内容安全高度归口

日期：2026-08-12

状态：已采用（ARCH-98 / UI-1.171）

## 背景

`ActionRailButton` 使用共享主题 stylesheet 和系统 CJK 字体。当前环境下按钮内容安全最小高度为
`36px`，但命令空态、组件空态和终端空态曾分别调用 `setFixedHeight(30)`，运行时形成
`minimumHeight=36`、`maximumHeight=30` 的矛盾约束。Qt 仍会绘制出 36px，但布局 contract 不一致，
后续换字体、主题或父布局时容易再次挤压。

## 决策

在 `presentation/action_surface.py` 中让 `ActionRailButton` 统一声明 `36px` 最小高度；移除三个
presentation caller 的局部固定高度。按钮仍由 native Qt 绘制，既有 signal、callback、焦点、
accessibility 和共享动效 fan-out 不变。空态 card 的业务文案和页面 scroll owner 不迁移。

## 验证与边界

三主题、四 workspace 的真实组合根验证中，相关 primary buttons 均满足 `min=36`、
`max=16777215`，横向 scroll 最大值均为 `0`。共享 MotionController 保持 `TARGET_HZ=120`、
`8ms` PreciseTimer；1 秒离屏采样为 `119Hz`，这只证明调度目标在当前环境达到，不等同于物理显示器
精确刷新率。Ruff、compileall、`scripts/check.ps1` 和 provenance verify 通过。

本轮为 Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A；
架构师 `019ff352-98d1-79f2-a457-1dc781315fac` 与独立 reviewer
`019ff355-5852-7742-93e6-43193230a952` 均在等待窗口内超时关闭，未形成外部结论；父代理完成
correctness、architecture、security、performance、readability 五轴 review 与简化评估。不作固件
标准或认证声明。
