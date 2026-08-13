# ADR 0134：工作区配置页自适应 focus

- 日期：2026-08-11
- 状态：accepted
- 关联切片：ARCH-6z / UI-1.142

## 背景

SerialForge 的默认总览同时展示 header、工作区、实时观测、终端和发送区。最小支持窗口
980×720 下，真实组合根把 workspace shell 压到约 150px，协议页的实际首屏 viewport
只有约 79px；1240×820 下协议 viewport 也只有约 149px。页面虽然有纵向滚动，但用户首次
看到的是被截断的多行控件，造成“组件挤在一起”的视觉和操作问题。

项目已经有 presentation-only 的 `workspace_focus_transition.py`：它可以用 220ms
`QParallelAnimationGroup` 收起实时观测、终端和发送区，并把工作区 tabs 扩展到可用高度。
重复实现 splitter、第二套高度策略或新的常驻 timer 会扩大生命周期和布局耦合。

## 决策

在 `controllers/workspace_runtime.py` 的 `on_workspace_tab_changed()` 中增加一个很窄的
路由同步：

- Tab index 0（“链路 / 连接”）继续是默认总览路由，保持下方实时观测、终端和发送区可见；
- Tab index 1/2/3（协议 / 遥测、命令管理、扩展 / 工具站）自动调用既有
  `set_workspace_focus_mode(window, True)`；
- 返回 index 0 自动调用既有 `set_workspace_focus_mode(window, False)`；
- 当前页的手动 focus button 仍然有效；手动操作不会改变 Tab 或业务状态，下一次路由变化
  可以重新执行路由同步。

自动切换只复用已有 focus owner、220ms 过渡、reduced-motion 静态回退和 lifecycle fence。
实时观测、终端、发送 widget 的隐藏是 presentation 几何行为，session、接收、记录、发送、
连接 gate 与后台 worker 不停止。

## 不纳入本次范围

- 不引入 `QSplitter`、第二套 responsive layout policy 或每控件 timer；
- 不改变 `MotionController` 的单时钟、120Hz 目标、PreciseTimer 或 8/9ms slot；
- 不改变 Tab 文案/index、焦点顺序、accessibility、主题 token、业务 callbacks 或 OTA/debug
  contract-only / attach-only 边界；
- 不将布局模式下沉到 application/domain，也不复制 session/ViewModel 状态；
- 不自动改变启动总览：初始 index 0 仍显示 onboarding terminal surface。

## 不变量与验证

- `on_workspace_tab_changed()` 是自适应路由的唯一调用点；不得在 builder 或 lifecycle 复制
  `setVisible()` / `maximumHeight` 策略；
- 980×720：总览 workspace 为 150px、protocol focus viewport 至少 435px；1240×820：总览
  workspace 为 220px、protocol focus viewport 至少 535px；
- 三主题 × 两种窗口尺寸 × 四个 Tab 均无 root child overlap，当前 settings page
  horizontal scrollbar maximum 为 0；
- lower surfaces 在配置 focus 中隐藏、总览中恢复；reduced-motion 直接静态收敛；
- 真实组合根通过 `MotionController.TARGET_HZ=120`、8/9ms slot、250ms 29 帧（约 116fps
  offscreen 观测）与 56 个唯一 motion surface 检查。

## Assurance

本轮只改 Python/Qt presentation，不涉及 MCU、embedded C/C++、vendor SDK、RTOS、ISR/DMA、
OTA firmware 或硬件，因此 public vendor source applicability 为 N/A；不宣称 MISRA、ISO 26262
或任何认证合规。架构师只读线程已按项目约束调用但超时关闭，未将超时视为通过；父代理完成
correctness/readability/architecture/security/performance 五轴复核与行为保持型简化评估。
授权验证仅为 compileall、Ruff、`scripts/check.ps1`、offscreen 真实组合根、主题/布局/动效
测量；真实 Windows GUI、HIDPI、读屏、UART/网络/BLE/RTT/J-Link 和目标板硬件验收未执行。
