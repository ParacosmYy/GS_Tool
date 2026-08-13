# ADR-0163：紧凑顶栏隐藏重复状态标题

## 状态

已接受（ARCH-112 / UI-1.185，2026-08-12）

## 背景

在 `980×720`、`1040×720` 和 `1120×720` 窗口中，实际 app header 宽度分别约为 `948`、`1008` 和
`1088px`，仍处于紧凑态。状态组同时展示上下文、数据来源、“连接状态”、状态灯和当前状态文本，
与动效及主题控件共享一行；其中“连接状态”与状态灯/状态值存在视觉重复，挤压了可用间距。

## 决策

由 `src/serialforge/presentation/controllers/workspace.py` 的 `_sync_header_density()` 统一拥有该
响应式视觉策略。`build_app_header()` 将 QLabel 保存为 `_AdaptiveHeader._state_caption`；当实际
`header.width() < 1120` 时隐藏它，达到阈值时恢复。状态灯、状态值、状态组 accessible name/description、
动态状态投影、Tab/focus 顺序以及主题/动效控制不变。

该决策不引入新的状态源、timer、动画时钟、QSS、业务判断、scroll owner、transport、OTA/AES/RTT/
J-Link 或设备 I/O 依赖。

## 被否决的替代方案

- 仅继续压缩字体和间距：仍然保留重复语义，且会降低可读性。
- 隐藏状态灯或当前状态值：损害连接状态的快速识别与无障碍信息。
- 把状态组拆到第二行：改变顶栏高度和布局 owner，扩大本次切片风险。

## 验证

真实 Qt offscreen 三主题×`980×720`、`1040×720`、`1120×720`、`1240×820` 验证通过：紧凑态标题为
hidden，常规宽度恢复 visible，status cluster 约由 `579px` 收敛至 `536px`，宽屏约 `598px`，
`HMAX=0`，状态组 accessibility 保持。resize 序列、`已连接` 状态投影、accessible description、
pause/resume/hidden/close 生命周期通过，`MotionController` timer 数量仍为 1。静态检查与 compileall
在交付前复跑；真实 GUI/HIDPI、EXE startup、硬件、签名仍未验收。嵌入式 C/C++ public-vendor-source
applicability 为 N/A，不作固件或认证合规声明。
