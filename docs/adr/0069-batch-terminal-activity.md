# ADR-0069：批量命令终态 activity confirmation

日期：2026-08-10  
状态：Accepted for UI-1.82  
范围：`presentation/controllers/commands.py`、`presentation/command_batch_surface.py`

## 背景

批量命令已有 RUNNING activity，但 COMPLETED、STOPPED、FAILED 只改变状态文本和步骤 projection。结果落地很快时，用户难以从统一的 signal rail 判断终态边界；同时，批量“完成”只代表本地发送队列接受，不能被界面误读为设备应用层已经处理。

## 决策

复用现有 `MotionController`：可见且未最小化、未关闭的 RUNNING 快照请求既有 520ms activity，COMPLETED/STOPPED/FAILED 各请求一次 480ms terminal confirmation。`CommandBatchSurfaceLabel` 只从既有 bounded `CommandBatchSurfaceProjection` 读取终态，在步骤 rail 绘制主题语义色的勾、横线或叉。

IDLE、bootstrap hydration、隐藏/最小化、暂停和 reduced-motion 不制造 activity。关闭继续由现有 lifecycle stop/close fence 负责。此增量不新增 DTO、事件总线、计时器、百分比、时间进度、发送队列、结果表或 accessibility 语义。

## 拒绝的方案

- 不在 surface 中创建局部 `QTimer`：会产生第二个动画生命周期，且关闭时更难统一清理。
- 不把 accepted steps 绘成百分比或时间进度：它不是设备处理进度，也没有应用层 ACK 语义。
- 不新增 batch terminal event bus：现有 snapshot projection 已提供足够的 bounded 状态，新增总线会扩大耦合面。

## 验证

静态检查、`compileall`、五状态隐藏 vector、可见 520/480 activity probe、隐藏/最小化/关闭 guard 和三主题 marker render 均通过，详见 [最新交接](../handoffs/current.md)。主窗口未 `.show()`；Windows GUI 动态帧、HIDPI、真实设备/网络、OTA、签名及正式发行未运行。

## 评审与适用性

六个职责角色在源码修改前按项目约束调用，均在窗口内超时并关闭；独立质量复核在源码修改后调用，同样超时并关闭。父代理完成 correctness、readability/simplicity、architecture、security、performance 五轴复核；嵌入式 C/C++ 适用性为 N/A。
