# ADR 0044：主题切换一次性 fade + sweep 过渡

日期：2026-08-10

## 状态

已接受，UI-1.57。

## 决策

保留 `theme_transition.py` 作为主题切换过渡的唯一 owner，在现有 `appRoot` opacity fade 上增加一个短时
`ThemeTransitionSurface` 几何 sweep。overlay 只绘制当前 `ThemeSpec` 语义 accent，不参与焦点、键盘、无障碍或业务状态。

fade 与 sweep 使用同一 180ms 生命周期；自然完成、快速连续切换、低动效、暂停、隐藏、最小化和关闭均由同一 stop/finish 路径清理。
`apply_theme()` 仍是唯一主题事实入口，`MotionController` 不参与这次一次性过渡。

## 边界

- `theme_transition.py` 管理 effect、sweep overlay 和两个 animation 的创建/停止/释放；
- `theme_transition_surface.py` 只负责语义色几何绘制；
- lifecycle 继续决定何时允许/停止主题过渡；
- 不新增 timer、ViewModel 读取、业务进度、主题 token 或外部资源。

## 验证要求

覆盖 overlay 鼠标透明/空 accessibility、动画运行与 finish cleanup、reduced-motion 回退、三主题截图、near-white，以及最终
onefile/root EXE provenance。
