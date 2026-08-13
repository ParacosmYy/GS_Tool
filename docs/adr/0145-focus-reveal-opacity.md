# ADR 0145：工作区 reveal 使用临时 opacity 避免布局中间帧拥挤

- 日期：2026-08-12
- 状态：accepted
- 范围：`presentation/workspace_focus_transition.py`

## 背景

专注设置返回总览时，Tab 页与实时观测、终端、发送区需要在固定窗口高度内同时让位。原实现把
四组 `maximumHeight` 动画并行执行，几何目标正确，但下方 surface 在高度尚未足够时就绘制内部
控件，用户会看到按钮和输入框被压缩到同一块区域。

## 决策

继续使用现有一次性 `QParallelAnimationGroup`、实测高度目标和 220ms easing；仅为三块新显现的
下方 presentation surface 安装临时 `QGraphicsOpacityEffect`，把 opacity 从 0 渐显到 1。effect
由同一 owner 记录并在自然完成、停止、快速反转、低动效、隐藏、最小化、resize 和 close 路径
统一解绑。已有 graphics effect 不会被覆盖。

## 结果与边界

该方案不改变布局、业务状态、ViewModel、session/transport、Tab/focus/accessibility 或唯一
`MotionController`。effect 不是第二个时钟，也不表达业务进度；它只遮住高度分配期间的视觉拥挤。
Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A。

## 验证

`ARCH94_FOCUS_MOTION_PASS mid_effects=3 settled_heights=(96, 162, 98) frames=185 target_hz=120
interval_ms=8 effects_clean=True`；Ruff、compileall、`scripts/check.ps1` 和 source-limit/theme audit
通过。真实显示器 FPS、EXE 启动、硬件连接和正式发行资格未由本 ADR 宣称。
