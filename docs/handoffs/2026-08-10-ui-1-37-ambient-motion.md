# 2026-08-10 UI-1.37 共享环境动效交接

## 范围

在既有 presentation-only `MotionController` 上增加可显式启停的 `ambient_active`。目标是解决窗口已显示、没有连接或接收活动时，
顶栏信号场完全静止的体验缺口；不新增控件级常驻 timer，不改变业务状态和传输语义。

## 实现

- `src/serialforge/presentation/widgets.py`：保留唯一 96ms `QTimer`，`_can_animate()` 在 ambient、transition 或短 activity 任一条件
  成立时出帧；`set_ambient_active(False)` 清理短活动窗口的残留唤醒条件。
- `src/serialforge/presentation/controllers/lifecycle.py`：show 时开启 ambient；隐藏/最小化继续使用 `suspended` 停表；关闭时
  先关闭 ambient，再调用 `close()`。
- reduced-motion、显式暂停、隐藏、最小化、关闭仍优先于 ambient；renderer 仍收到既有 frame 并保留静态绘制回退。

## 架构师调用与父级审计

架构师 Luna max `019fea0c-02b7-7e72-b8a8-4f6712a3ec3c` 被调用为只读审查角色；两次等待共约 90 秒未返回，随后关闭，未给出 GO/NO-GO。
父代理审计了 `widgets.py`、`lifecycle.py`、workspace 接线和现有 motion surface：改动只跨共享 presentation clock 与生命周期 owner，
没有循环导入、业务依赖、第二个时钟或无障碍文本变化，因此按最小切片实施。

## 性能与行为证据

```text
BASELINE_MOTION   timer_active=False frames=0 animated=0
UI137_MOTION      PASS ambient_timer=True animated=3 suspended=False resumed=True reduced=False restored=True closed=False
UI137_COMPOSITION PASS ambient=True ambient_timer=True animated_frames=5 hidden_timer=False restored_timer=True closed_timer=False
IMPORT_SMOKE      PASS modules=123 qapplication=none
PIXEL_AUDIT       PASS 980x680 near_white=0 white=0
STATIC            PASS scripts/check.ps1; 123 files <= 1000; theme token audit; ruff
SCREENSHOT        build/ui_review_ui137_ambient_motion.png (人工查看通过)
```

空闲离屏结果约 9 帧/秒，与 96ms 共享间隔一致；没有 CPU/内存 profile，因此不将性能预算写成已通过。offscreen 环境缺少 PySide6
font directory，中文方框是环境字体告警，不代表 Windows 运行时字体结果。

## 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录副本：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.37`
- size：`47,789,846` bytes
- SHA-256：`32A6B0DDC824065A970811549A88593ADD6D69715E558D15189CA515866CBB90`
- archive listing SHA-256：`2C686B9AFB0B1E4C116CD0A51740BCA02C524E4F485E6F1942078B12C5F822BF`
- signature：`NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 嵌入式 R&D assurance gate

本轮是 Python/PySide6 桌面 presentation 变更，不涉及 embedded C/C++、MCU、BSP/HAL/CMSIS、RTOS、ISR/DMA、driver、bootloader、
OTA firmware、Flash/NVM、power 或 motor-control；`mcu`、`embedded-enterprise-workflow` 和 `embedded-code-review-simplifier` 的
厂商源适用性均为 N/A。未声称 MISRA、ISO 26262、WCAG、认证、签名发布或硬件合规；未启动 GUI/EXE、串口、网络、RTT/J-Link、OTA 或目标硬件。

简化评估：保持单一共享时钟和现有 lifecycle stop fan-out，未创建测试专用资产，未复制业务状态，未增加 renderer timer。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
