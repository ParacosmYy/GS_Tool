# ADR-0068：历史回放终态 Activity Confirmation

- 日期：2026-08-10
- 状态：accepted
- 范围：presentation / historical replay

## 背景

回放区在 PLAYING 时已有共享 activity pulse，但 EOF、STOPPED、ERROR 到达后主要依赖文字状态和颜色。回放可能在用户观察其他控件时结束，
静态终态不容易被第一时间发现。

## 决策

`controllers/replay.py:_request_replay_activity()` 只对可见、未最小化、未关闭窗口复用唯一 `MotionController`：PLAYING 使用既有 520ms，
EOF/STOPPED/ERROR 使用一次 480ms；PAUSED/EMPTY 和隐藏 hydration 不请求。`ReplayActivityLabel` 继续只消费 `ReplayActivityProjection`，在
history/error 状态绘制静态终态 marker，保留现有 trace，不把记录数量解释成进度，也不创建局部 timer。

## 拒绝的方案

- 为回放终态增加独立 `QTimer` 或动画对象：会破坏 shared clock/lifecycle fence，并让单个 surface 维护自己的时间源。
- 把 records_emitted 映射为百分比或进度条：回放没有可供用户期待的总进度契约，容易误导。
- 改动 ReplaySnapshot、按钮 enable、source badge 或自动弹窗：扩大 domain/controller 语义并打断调试操作。

## 验证

- `scripts/check.ps1`：PASS；源码行数 149 个文件均不超过 1000 行，主题 token 审计 PASS。
- `.venv\Scripts\python.exe -m compileall -q src`：PASS。
- 真实 composition root + Qt offscreen：PLAYING/PAUSED/EOF/STOPPED/ERROR 隐藏 guard、可见 520/480 probe、三主题 history/error marker render PASS。
- 主窗口未 `.show()`；可见 Windows GUI 动态帧、HIDPI、读屏、EXE 启动、硬件/网络和正式发行验收未运行，未据此宣称通过。

## 评审记录

六个职责角色在源码修改前调用，独立复核在实现后调用；均在窗口内超时并关闭，未把超时视为通过。父代理完成 correctness、readability/simplicity、
architecture、security、performance 五轴复核。嵌入式 C/C++ 适用性：N/A。

