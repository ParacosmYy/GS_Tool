# ARCH-111a / UI-1.184 交接：共享动效 elapsed frame budget

日期：2026-08-12

## 结果

`src/serialforge/presentation/widgets.py` 的唯一 `MotionController` 不再按整数 8ms 槽位固定
累积 `0.96` 帧，而是使用已有受 cap 保护的实测 monotonic elapsed 累积 `elapsed × TARGET_HZ`。
8ms `PreciseTimer`、`TARGET_HZ=120`、每 tick 最多一帧、phase speed/cap、共享
`frame_changed` fan-out 和 pause/reduced-motion/hidden/minimized/close 生命周期保持；没有新增
timer、线程、组件动画、业务状态、scroll owner、设备 I/O 或 OTA/AES/RTT/J-Link coupling。

## 验证

- `scripts/check.ps1`：待本轮代码与文档完成后最终执行。
- `compileall`：待本轮最终执行。
- 真实 Qt offscreen 主循环：`87` frames，均值 `8.360ms`，p95 `16.000ms`，有效约 `119.61Hz`，
  timer interval `8ms`，固定预算常量不存在。
- 暂停 timer inactive、恢复 active；隐藏 inactive、显示后 active；关闭 inactive。
- 三主题×980×720/1240×820×四 workspace：横向 scrollbar maximum 均为 `0`；focus/overview
  terminal/send 可见性保持。

## 审查边界

架构师 `019ff438-6600-7a80-89ed-fc7694a209d6` 与独立 reviewer
`019ff439-b809-7db3-b3ff-36157aa8dabf` 在等待窗口内超时关闭，未形成外部结论，未伪造 PASS。
父代理完成 correctness、architecture、security、performance、readability 五轴审查与行为保持
简化评估。offscreen scheduler 证据不外推为真实显示器 120Hz、HIDPI 或 EXE 正式性能。

本轮没有嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A；不作 MISRA、ISO 26262、
ASIL、ASPICE 或认证声明。GUI/EXE startup、真实 Windows 可见窗口、高刷新显示器/HIDPI、高负载、
硬件连接、OTA/RTT 实连和签名验收仍需授权环境验证。

## 交付

`local-arch-111a` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe`；三者均为 `48,021,974` bytes，SHA-256 为
`56133D65BBE514AB18AF505D0446354CA948B96A6BBCEE39D16CCCBA2EC0084F`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
