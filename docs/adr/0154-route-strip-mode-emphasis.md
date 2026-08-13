# ADR-0154：路线条按工作区模式提供视觉层级

日期：2026-08-12

状态：已采用（ARCH-103 / UI-1.176）

## 背景

工作区路线条已经拥有 focus/overview 的动态 `mode` 属性、专注按钮和滚动提示，但两种模式的
视觉上沿接近，用户需要依赖按钮文字才能确认当前布局策略。需要增强模式层级，同时不能再引入
一个 presentation 状态或改变布局尺寸。

## 决策

在稳定 controls stylesheet 中增加 `workspaceShell[mode] workspaceRouteStrip` selector：focus 使用
info→history 渐变与 accent-blue 上沿，overview 保持 neutral 上沿；variant stylesheet 只映射
当前主题的 `ThemeSpec` token。route strip 保持固定 31px，`WorkspaceFocusButton`、`WorkspaceScrollHint`
和 Tab focus/keyboard contract 不变。

## 验证与取舍

三主题（star_trail、moonlit_ocean、sakura_night）focus/overview 实跑均显示正确 `mode`、按钮文案
和滚动状态；980/1240 四 workspace 横向 scroll 均为 0。0.5 秒离屏时钟采样 60 frames、均值
8.41ms、有效约 118.8Hz；不把离屏调度或真实显示器合成宣称为精确 120fps。静态、compile、ruff、
source-limit、theme audit 和 accessibility 通过。

本轮是 Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A；不作
MISRA、ISO 26262、ASIL 或认证声明。架构师 `019ff3ac-0f16-7633-8304-1cb3a1cecb54` 与独立 reviewer
`019ff3b1-8e10-7b63-94c3-af41db79e1a5` 均在等待窗口内超时关闭，未形成外部结论；父代理完成五轴
review 与行为保持简化评估。
