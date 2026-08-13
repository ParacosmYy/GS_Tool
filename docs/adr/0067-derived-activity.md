# ADR-0067：派生快照 Activity Pulse

- 日期：2026-08-10
- 状态：accepted
- 范围：presentation / Protocol → Component → Dataset → Curve

## 背景

协议派生页已有 status rail、pipeline summary、组件表、Dataset 预览和曲线等状态表面。实时 UART/TCP 的终端接收会触发共享 activity，
但历史回放或异步派生输出进入 Component/Dataset 时不一定与终端 pulse 同步；用户看到表格/预览内容跳变，却缺少轻量的“结果已进入界面”确认。

## 决策

在 `controllers/derived_data.py` 中保留一个 `_request_derived_activity()` presentation helper。只有非空且类型正确的
`ComponentFrameRow`/`DatasetSample` 快照进入 `on_component_rows_changed()`/`on_dataset_samples_changed()`，并且窗口可见、未最小化、未关闭、
Protocol workspace 当前可见时，才复用唯一 `MotionController.request_activity(360)`。该反馈不改变快照、stats、DTO、表格/预览/曲线内容、渲染节流、
清空语义或无障碍；shared controller 自己负责暂停、reduced-motion、隐藏和关闭的静态回退。

## 拒绝的方案

- 为派生管线增加事件总线、snapshot hash/cache 或第二套状态源：会把 presentation feedback 误升格为业务状态，并扩大 controller 依赖。
- 在 `derived_data.py` 创建新的 `QTimer` 或每个结果 widget 自持动画：破坏唯一 MotionController 和生命周期统一 fence。
- 在空快照、统计文字刷新或用户切换 series 时触发 pulse：这些不是新的派生结果，容易制造虚假 activity。

## 验证

- `scripts/check.ps1`：PASS；源码行数 149 个文件均不超过 1000 行，主题 token 审计 PASS。
- `.venv\Scripts\python.exe -m compileall -q src`：PASS。
- 真实 composition root + `QApplication`/Qt offscreen：隐藏 hydration guard、可见 360ms shared-controller probe、hidden/minimized/other-tab guard、
  no-local-timer 约束 PASS；三套主题应用 PASS。
- 主窗口未 `.show()`；可见 Windows GUI 动态帧、HIDPI、读屏、EXE 启动、硬件/网络与正式发行验收未运行，未据此宣称通过。

## 评审记录

六个职责角色在源码修改前调用，独立复核在实现后调用；均在窗口内超时并关闭，未把超时视为通过。父代理完成 correctness、readability/simplicity、
architecture、security、performance 五轴复核。嵌入式 C/C++ 适用性：N/A。

