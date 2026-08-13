# ADR 0135：连接 onboarding CTA 进入配置 focus

- 日期：2026-08-11
- 状态：accepted
- 关联切片：ARCH-70 / UI-1.143

## 背景

默认总览保留实时观测、终端和发送区，这是未连接用户的重要 onboarding 画布。空终端中的
“打开链路连接”按钮目前只切换到 index 0；在 980×720 下 connection page 仍只有约 79px
首屏，UART/网络/BLE 表单的后续行需要用户再手动点击“专注设置”。这与 CTA 已明确表达的
“我要开始配置连接”意图不一致。

ARCH-6z 已将协议、命令和扩展页的 Tab route 自适应接入既有 focus transition，因此本次只
补齐空态 onboarding 的显式入口，不重做布局系统。

## 决策

新增 `workspace_runtime.open_connection_setup(window)`，由 `bootstrap.py` 把
`TerminalEmptyState.connection_requested` 接入。入口执行以下 presentation 操作：

1. 选择 bounded connection tab index 0；
2. 复用 `set_workspace_focus_mode(window, True)` 展开连接配置；
3. 把键盘焦点交还真实 `QTabWidget`。

它不触碰 `SessionViewModel`、session、transport config、连接 gate 或业务 callback。默认启动
index 0 不会自动 focus；只有用户显式点击 onboarding CTA 才进入 connection focus。用户可用
既有“返回总览”按钮恢复实时观测、终端和发送区。

## 生命周期与不变量

- 关闭中、workspace bundle 缺失或 index 无效时安全返回；重复点击不创建第二个 timer 或连接动作；
- focus transition 继续由 `workspace_focus_transition.py` 负责，沿用 220ms、reduced-motion、
  hidden/minimized/close 静态收敛；
- hidden 的下方 widget 仍由原 owner 和后台 worker 持有，接收、记录、发送和 session 不暂停；
- CTA、Tab、focus button、主题和 accessibility 的现有文案保持；`MotionController` 单时钟、
  120Hz target、PreciseTimer、8/9ms slot 不变；
- terminal empty component 只发出现有 signal，不知道 workspace focus 或连接策略。

## 验证

真实 offscreen 组合根 980×720 点击 `_connection_action` 后：connection viewport 为 471px，
实时观测/terminal/send 均隐藏，ViewModel state 仍为 `closed`；点击“返回总览”恢复 150px
workspace 与三个 lower surface；普通 protocol route 的 adaptive focus 仍通过。compileall、Ruff、
`scripts/check.ps1`、source-limit、theme-audit 通过。

## Assurance

本轮只改 Python/Qt presentation，不涉及 MCU、embedded C/C++、vendor SDK、RTOS、ISR/DMA、OTA
firmware 或硬件；public vendor source applicability 为 N/A，不宣称 MISRA、ISO 26262 或认证合规。
Luna/max/Fast 架构师只读线程已调用但等待超时并关闭，未将超时视为通过；父代理完成 correctness、
readability、architecture、security、performance 五轴复核与行为保持型简化评估。授权验证未包含
真实 Windows GUI/EXE 启动、HIDPI、读屏、UART/网络/BLE/RTT/J-Link 或目标板硬件验收。
