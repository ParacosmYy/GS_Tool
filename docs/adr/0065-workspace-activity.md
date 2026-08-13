# ADR-0065：工作区可见导航 activity pulse

- 日期：2026-08-10
- 状态：accepted
- 范围：presentation / workspace navigation

## 背景

工作区已经有 `WorkspaceRouteSurface`、主题化 Tab glyph 和 180ms page fade，但用户点击或键盘切换工作区时，首个确认反馈只来自控件本身，
路线 beacon 的共享 frame 不一定立即进入 activity window。需要一个短、可撤销且不改变导航语义的即时反馈。

## 决策

1. `workspace.py` 将现有 `QTabWidget.currentChanged` 接入 `workspace_runtime.py:request_workspace_activity()`。
2. runtime owner 仅在窗口未关闭、未隐藏、未最小化时调用唯一 `MotionController.request_activity(320)`；由既有 lifecycle fan-out 让 route beacon 和
   其他 presentation signal surfaces 消费共享 frame。
3. `on_workspace_tab_changed()` 继续负责 route index、derived renderer suspension 和 pipeline refresh；`animate_workspace_transition()` 继续负责
   180ms page fade。新增 pulse 不改变 Tab index、文案、focus、accessibility、ViewModel 或业务状态。

## 被拒绝方案

- 给 Tab 新增 active/activity 状态字段：会复制 Qt 的 currentIndex，并扩大 workspace navigation 状态源。
- 为 route beacon 单独创建 QTimer/QPropertyAnimation：会制造第二套动画时钟，破坏 reduced-motion、暂停和关闭边界。
- 在 hydration/showEvent 中直接请求 pulse：会把初始化误报成用户行为，导致首屏不必要闪烁。

## 验证

- `scripts/check.ps1`：pass；149 个 Python 文件均不超过 1000 行，theme token audit pass。
- `python -m compileall -q src`：pass。
- 真实 composition root + `QApplication`/Qt offscreen：隐藏主窗 hydration 不请求；三主题下可见导航 probe 的三 tab 切换请求 320ms；
  hidden/minimized/closing/paused guard pass。
- 未显示主窗口、未启动 EXE、未接入硬件/网络/OTA；未创建或运行 test-only 资产。

## 复核记录

六角色与独立复核均按项目约束调用，等待窗口内超时并关闭，未把超时视为通过。父代理完成 correctness、readability/simplicity、architecture、
security、performance 五轴复核。嵌入式 C/C++ 适用性：N/A。

