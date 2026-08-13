# ADR-0160：专注/总览切换的首帧可读淡入

日期：2026-08-12  
状态：accepted  
范围：`presentation/workspace_focus_transition.py`

## 问题

专注设置返回总览时，布局会立即结算并让出空间，但实时观测、终端和发送 surface 原先从
`opacity=0.0` 开始淡入。用户在约 220ms 的共享过渡期间只能看到空白区域，感觉组件被挤走或
动画帧丢失；实际 geometry 已经稳定，问题属于首帧视觉可读性。

## 决策

继续由 `workspace_focus_transition.py` 作为 focus/overview 过渡 owner：

1. 保持一次性静态布局结算和根布局 activation 不变；
2. 保持 `MotionDrivenAnimationGroup` 与唯一 `MotionController` 的 220ms opacity track；
3. 将 overview surface 的 reveal 起点设为命名常量 `_REVEAL_START_OPACITY = 0.82`，再淡入到 `1.0`；
4. 不改变 widget geometry、可见性策略、业务状态、终端/记录/连接语义或滚动 owner。

选择 0.82 是为了让第一帧已经可辨识，同时保留轻微的层级过渡；它不是新的主题 token，也不
进入 QSS 或应用状态。专注态仍按原策略立即隐藏下方 surface。

## 备选方案

### 继续从 0.0 淡入

拒绝：首帧到中间帧会呈现大块空白，无法满足用户对组件连续可读和“动画不卡/不挤”的要求。

### 动画 geometry 或 `maximumHeight`

拒绝：历史证据已表明逐帧压缩 `QTabWidget` 会造成配置组件挤压；本轮只允许 opacity 视觉过渡。

### 新增 timer 或独立动画时钟

拒绝：会破坏窗口级唯一 `MotionController`、120Hz scheduler contract 和生命周期收敛。

## 验证边界

真实 Qt offscreen 事件循环覆盖：

- `980×720`、`1240×820`：overview 首帧三个 lower surface 的 opacity 为 `0.82`，约 110ms
  为 `0.976~0.977`，约 220ms 为 `1.0`；geometry 在整个过渡中保持稳定且可见；
- 反向切回专注态：三个 surface 最终 `visible=False`、高度为 `0`，页签视口恢复；
- 三主题截图：`star_trail`、`moonlit_ocean`、`sakura_night` 均无白色断层或横向溢出；
- 980/1240 四工作区：Tab 切换、route/context/scroll hint geometry 与横向 scrollbar `0`；
- 用户路径生命周期：低动效、暂停、隐藏/恢复、关闭均通过；
- 共享 `MotionController`：`77` frames、均值 `8.322ms`、p95 `9.919ms`、有效约 `120.16Hz`，
  target `120Hz`、timer interval `8ms`。

本轮没有创建、修改或运行 unit test、mock、fixture、harness 或 test-only asset。嵌入式 C/C++
public-vendor-source applicability 为 N/A；不作 MISRA、ISO 26262、ASIL、ASPICE 或认证声明。

## 审查状态

架构师调用与独立 reviewer 调用均在等待窗口内超时关闭，未形成外部结论，未伪造 PASS。父代理
完成 correctness、architecture、security、performance、readability 五轴 review，以及保持行为
不变的简化评估；确认本轮只有一个常量和一个 presentation reveal 起点变化。
