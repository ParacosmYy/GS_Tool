# ADR-0106：Qt scrollbar signal rail 可见性

日期：2026-08-11  
状态：accepted（UI-1.118）

## 背景

980×680 下连接、协议、命令和扩展页的内容需要在既有 `QScrollArea` 中滚动，但深色主题的 thumb
与 track 对比度偏低，用户不容易发现还有配置内容。需要增强发现性，同时保留 Qt 原生滚动语义和
terminal 的滚动行为。

## 决策

- 只修改 stable/theme controls stylesheet 的现有 `QScrollBar` selector。
- 默认 thumb 使用已有 `BORDER_STRONG`，主题 override 使用 `accent_purple`；hover 使用既有粉色
  accent，pressed 使用既有薄荷 accent。
- 保留无箭头按钮、原有 vertical/horizontal 尺寸、最小 thumb、surface/corner、scroll range、焦点
  和键盘行为；不创建新 widget、子类、事件过滤器或动画时钟。

## 备选方案

### 为每个页面增加滚动提示控件

拒绝：增加布局占用和第二套滚动状态源，且不能可靠覆盖 terminal、popup 和原生 scroll area。

### 用事件过滤器动态绘制 thumb

拒绝：扩大生命周期与事件边界，容易改变原生 drag/keyboard 处理；现有 QSS 状态已经足够表达默认、
hover 和 pressed。

### 只提高 track 亮度

拒绝：大面积亮 track 会增加视觉噪声，且不如 thumb 直接表达可拖动区域。

## 后果与限制

thumb 在三套主题中更容易发现，仍是静态主题化的 native Qt 控件；pressed 只表示用户正在拖动。
QScrollArea 的真实内容和滚动范围不被切片修改。Windows native style、HIDPI、读屏和长内容性能仍需
授权环境验收。

## 验证

`UI118_SCROLL_SIGNAL_VECTOR_PASS`（980/1180、3 themes、4 tabs、scrollbars、pressed）；24 张
离屏视觉截图；source limit、compileall、Ruff、theme token audit 和 provenance verify 通过。
架构师与独立质量审查线程在限定窗口内超时，未计为通过；父代理完成 token/selector/native behavior
和简化审查。本轮不包含嵌入式 C/C++，embedded applicability=N/A。
