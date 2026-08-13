# ADR-0066：错误通知 Activity Pulse

- 日期：2026-08-10
- 状态：accepted
- 范围：presentation / error notification

## 背景

SerialForge 已有 `ErrorSignalSurface`、Header signal field 和 status rail，用于承载错误状态、严重性和清除动作。错误文本本身是静态投影，
在信息密集的嵌入式调试工作区中可能不容易被第一时间注意到。需要增加即时确认，但不能把视觉反馈误认为新的错误状态或业务进度。

## 决策

在 `lifecycle.on_error_changed()` 确认收到非空 `ErrorInfo` 后，只有当窗口可见且已有 `_motion_controller` 时，向窗口唯一的
`MotionController` 请求一次 `request_activity(520)`。该 pulse 只驱动既有 presentation consumers，不改变 `ErrorInfo`、错误文案、严重性、
清除按钮、焦点、Tab 顺序或 accessibility 语义。`info is None` 的清除路径不请求 pulse；控制器尚未初始化、窗口隐藏/最小化/关闭时安全静态返回。

## 拒绝的方案

- 在 lifecycle 中新建本地 `QTimer` 或动画对象：会破坏共享动效时钟和生命周期统一暂停策略。
- 增加 error pulse counter、错误事件总线或 view-model 字段：会扩大错误 DTO/状态边界，且该反馈不是业务事实。
- 错误出现时自动聚焦、弹窗或修改清除流程：会打断串口调试操作并改变既有键盘/无障碍语义。

## 验证

- `scripts/check.ps1`：PASS。
- `.venv\Scripts\python.exe -m compileall -q src`：PASS。
- 真实 composition root + `QApplication`/Qt offscreen 三主题向量：错误出现/清除、隐藏 guard、暂停和 reduced-motion 静态策略 PASS。
- 主窗口未 `.show()`；可见 Windows GUI 的实际 520ms pulse、HIDPI、读屏、EXE 启动、硬件/网络和正式发行验收未运行，未据此宣称通过。

## 评审记录

六个职责角色均在源码修改前调用，但在窗口内超时并关闭；独立质量复核在实现后同样超时并关闭。父代理完成 correctness、readability/simplicity、
architecture、security、performance 五轴复核，并确认没有新增 timer、输入面、依赖或业务状态。嵌入式 C/C++ 适用性：N/A。

