# ARCH-102 / UI-1.175 交接：紧凑顶栏与过渡布局先结算

日期：2026-08-12

## 结果

本轮修复了用户反馈的两个视觉问题：紧凑窗口顶栏留白过大，以及专注设置↔总览动画中连接页
组件被逐帧挤压。`workspace.py` 的 header owner 现在在 compact density 下收敛顶栏上下边距、
品牌与控制簇间距、状态/动效/主题簇内边距和主题化圆角；`workspace_focus_transition.py` 则先
一次性结算最终布局，再只对完整尺寸的 tabs 或 overview surfaces 做 opacity 过渡。

这轮没有改动 transport/session/ViewModel/OTA/debug contract，未新增 timer、线程、scroll owner、
业务状态或设备 I/O。共享 `MotionController` 仍是唯一 120Hz target/8ms PreciseTimer。

## 证据

- `uv run ruff check ...`：pass。
- `uv run python -m compileall -q src`：pass。
- `scripts/check.ps1`：179 files ≤1000、3 themes/22 semantic tokens、legacy_qss_literals=0：pass。
- 980×720 overview/focus 中间帧 25/55/95/140ms：sibling geometry `overlap=False`；focus tabs
  `(1, 1, 946, 514)` 稳定，连接配置不再逐帧裁切。
- 三主题（star_trail、moonlit_ocean、sakura_night）× 980/1240 四 workspace：
  `horizontal_max=[0, 0, 0, 0]`。
- 1 秒离屏时钟：`frames=119`、`mean=8.27ms`、`median=8.03ms`、`p95=10.02ms`、
  `max=17.08ms`、`effective_hz≈120.9`。最大间隔只作为 offscreen scheduler jitter 记录，
  不等同真实显示器 FPS。
- accessibility：theme、motion、workspace、focus name/description 均非空；compact header
  height 实测 113px（不同字体后约 108–113px）。
- 主题和过渡截图：`C:\Users\Gs\AppData\Local\Temp\serialforge-arch102-fixed-overview-55.png`、
  `serialforge-arch102-fixed-focus-55.png`。

## 架构与审查记录

- 架构师调用：`019ff394-bb5d-7c60-9ce6-c60b65f1ac83`，等待窗口超时并关闭，未形成外部结论。
- 布局复核 Terra：`019ff39e-e6da-7ec3-8f96-7666b00d5574`，等待窗口超时并关闭，未形成外部结论。
- 独立 reviewer：`019ff39b-8db0-7cc1-833e-009e2d0129cb`，等待窗口超时并关闭，未形成外部结论。
- 父代理复核：correctness、architecture、security、performance、readability 五轴；简化判断为
  删除 `maximumHeight` 动画 track 与废弃的测量/高度辅助，保留单一 focus owner、effect cleanup、
  reduced-motion/pause/resize/hide/minimize/close 边界。
- embedded C/C++ public-vendor-source applicability：N/A。本轮没有固件、MCU、BSP/HAL/RTOS、OTA
  实现或 C/C++ 修改；不作 MISRA、ISO 26262、ASIL、ASPICE 或认证声明。

## 交付与未运行项

本轮 onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和 `SerialForge-latest.exe`；三者均为
`48,019,127` bytes，SHA-256 为
`73593393356CC3938291E62D4DD5F66DBE09B3AA300C64AB6BA65BA55A6200E7`，archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance revision 为
`local-arch-102`，verify 通过。签名为 `NotSigned`，`release_eligible=false`，
`hardware_acceptance=not_run`。
EXE 启动、真实 Windows 可见窗口/高刷新显示器/HIDPI、高负载、硬件连接、OTA/RTT 实连、签名和正式
硬件验收仍未运行或未授权；预计 provenance `release_eligible=false`、`hardware_acceptance=not_run`。
