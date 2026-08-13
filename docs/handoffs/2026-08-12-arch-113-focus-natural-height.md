# ARCH-113 / UI-1.186 专注工作区自然高度交接

日期：2026-08-12  
范围：`workspace_focus_transition.py`、workspace route refresh、native resize refresh。

## 结果

专注模式现在由 `workspace_focus_transition.py` 单一 owner 进行静态自然高度结算：隐藏实时观测与
发送区，复用已有透明 `terminalSurface` 作为 layout slack，并保留 terminal 子控件与父 surface 的
可见性/尺寸策略快照。短页面在可用窗口空间内收敛，长页面保留原生滚动；focus/overview opacity
过渡仍复用唯一 `MotionController`，没有逐帧 geometry 动画。

## 当前验证状态

本交接初始状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH113_LAYOUT_PASS=pass`；`ARCH113_RESPONSIVE_PASS=pass`；
`ARCH113_VISIBILITY_PASS=pass`；`ARCH113_LIFECYCLE_PASS=pass`；`ARCH113_120HZ_OWNER_PASS=pass`；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；
`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。

真实 Qt offscreen 三主题×980/1180/1240×四 workspace 共 36 cases 通过：focus/overview、before-show
focus、重复切换、route 切换、resize、暂停/隐藏/恢复/关闭、横向 HMAX=0、terminal hidden/restore、
live/send hidden/restore 和单一 MotionController。代表性 1240 geometry：connection shell/page
`536/462px`、`vmax=0`；command `331/257px`、`vmax=58`；extension `485/411px`、`vmax=623`。

## 架构与审查记录

架构师 Hubble、Pasteur、Russell 的只读方案/生命周期约束已纳入；Kant 的 refresh wiring 调用与独立
reviewer Pauli 在等待窗口超时关闭，未形成外部结论，没有将超时写成 PASS。父代理完成 correctness、
architecture、security、performance、readability 五轴 review 与 behavior-preserving simplification
assessment。嵌入式 C/C++、MCU、RTOS、ISR/DMA、厂商公开源适用性均为 N/A；不作固件或认证合规声明。

## 交付产物

`local-arch-113` onefile 已写入 canonical，并覆盖根目录 `SerialForge.exe` 与
`SerialForge-latest.exe`。三者均为 `48,024,709` bytes，SHA-256 为
`ED76F5E158A5B951B4A58B4EE1732277B3093C8A08FDD270EA19D57DB539703A`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名为 `NotSigned`，manifest 的 `release_eligible=false`，`hardware_acceptance=not_run`。

## 未运行项目

没有启动真实 GUI/EXE、HIDPI/真实显示器 120fps、硬件连接、OTA/RTT 实连或签名验收；这些需要后续
授权或具备相应环境。offscreen 120Hz scheduler 证据不等同真实显示器 120fps。
