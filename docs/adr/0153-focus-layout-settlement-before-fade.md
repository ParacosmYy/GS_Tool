# ADR-0153：专注模式先结算布局再执行透明度过渡

日期：2026-08-12

状态：已采用（ARCH-102 / UI-1.175）

## 背景

专注设置↔总览的高度动画虽然由共享 120Hz 目标时钟驱动，但 `QTabWidget` 会在每个中间帧重新分配
页面视口。连接配置表单因此被逐帧压缩，截图上表现为字段挤在一起或只剩横向条带。

## 决策

`workspace_focus_transition.py` 先应用当前模式的静态布局并激活根布局，再创建有限的
`MotionDrivenAnimationGroup`。专注进入只淡入已完整分配尺寸的 tabs，回到总览只淡入已处于最终
sibling 位置的实时观测、终端和发送 surface；不再把 `maximumHeight`、geometry 或 sibling allocation
作为动画属性。

顶栏密度继续由 `workspace.py` 这个 header owner 管理。980px 紧凑态收敛 shell margins、cluster
padding/spacing 和语义圆角，保留主题选择器、动效开关、连接状态、键盘焦点和 accessibility；1240px
宽度保留完整装饰。

## 验证与取舍

真实组合根离屏中，980×720 overview/focus 的 25/55/95/140ms 中间帧均无 sibling 几何重叠，focus
连接页 viewport 保持 514px；三主题×980/1240 四 workspace 横向 scroll 均为 0。共享时钟 1 秒采样
119 frames，均值 8.27ms、有效采样约 120.9Hz；最大 17.08ms 记录为 offscreen 调度抖动，不宣称
真实显示器精确 120fps。静态、compile、ruff、source-limit、theme audit 和 accessibility 通过。

本轮是 Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A；不作
MISRA、ISO 26262、ASIL 或认证声明。架构师 `019ff394-bb5d-7c60-9ce6-c60b65f1ac83`、布局复核
Terra `019ff39e-e6da-7ec3-8f96-7666b00d5574` 与独立 reviewer `019ff39b-8db0-7cc1-833e-009e2d0129cb`
均在等待窗口内超时关闭，未形成外部结论；父代理完成五轴 review 与行为保持简化评估。
