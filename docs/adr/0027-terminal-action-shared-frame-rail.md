# ADR 0027：终端空态动作的共享帧 signal rail

状态：已接受（2026-08-10）  
范围：`presentation/terminal_surface.py` 的终端空态动作视觉层

## 背景

UI-1.39 已为终端空态提供显式连接导航 CTA，但按钮与空态观测网格之间仍是静态关系。用户需要更
连续的二次元信号场体验，同时不能为一个装饰效果再引入控件级定时器或业务状态。

## 决策

- 新增 `TerminalActionButton` presentation-only renderer，仍继承原生 `QPushButton`，保留原文字、
  点击信号、焦点、accessible name/description、tooltip 和 `primaryButton` QSS。
- `set_frame(phase, animated)` 在原生按钮绘制完成后，于按钮底部绘制低对比度主题 accent signal rail；
  `stop()` 只关闭 animated flag 并刷新静态 rail。
- `TerminalEmptyState.set_frame/stop` 转发到 CTA；现有 `lifecycle._motion_surfaces` 已持有空态，
  因此 `MotionController` 是唯一 frame source，不新增 QTimer、signal、业务字段或 lifecycle 注册。
- rail 只消费 `ThemeSpec.accent`、phase 和 animated；它不读取 SessionState、RX、transport、连接进度、
  tab 状态或用户输入，不参与 CTA 导航。
- hidden CTA state (`paused/history`) 不进入可见绘制；低动效、暂停、隐藏、最小化和关闭沿用已有统一
  `set_frame/stop` fan-out，保留静态回退。

## 被拒方案

### 为按钮新增 QTimer

拒绝：会产生第二个动效时钟、重复生命周期和额外 CPU 唤醒；现有共享 MotionController 已满足低频
装饰帧需求。

### 用 CSS 无限动画或渐变替代 painter

拒绝：Qt Widgets 主题切换与 reduced-motion/关闭停止边界不易统一；共享 frame 能让所有装饰 surface
共享同一 phase，且没有外部资源。

### 把 rail 画进终端业务文本或连接状态

拒绝：会把装饰效果与数据事实耦合，可能误导用户把“视觉活动”当成设备已连接或正在传输。

## 后果

- 空态 CTA 与观测背景形成一致的信号轨迹，主题切换仍只由现有 `ThemeSpec` 驱动。
- 共享帧 fan-out 的 surface 数量增加一个子 renderer，但没有增加 timer 或业务复杂度。
- rail 是低对比度装饰，不替代按钮文字、焦点环或可访问性信息；静态模式仍有可见基线。

## 验证与限制

- `scripts/check.ps1`：通过；124 个 Python 文件均不超过 1000 行，theme token audit 与 Ruff 通过。
- Qt offscreen：`π/2` 与 `3π/2` 两端 phase 的 CTA 像素差 `34`；共享 MotionController 约 0.35 秒后
  phase 从 `0.000` 变为 `0.300` 且 `animated=True`；pause/hide/close timer 与 rail 均静态停止。
- 980×680、三主题近白像素为 0；UI-1.39 的 1180×780 响应式结果保持不变。
- offscreen 环境缺少 PySide6 fonts directory，中文方框不代表 Windows 运行时字体结论；未运行持续 GUI、
  EXE 启动、真实传输、RTT/J-Link、OTA 或硬件验收。

嵌入式适用性：本 ADR 仅涉及 Python/PySide6 presentation，不涉及 MCU、BSP/HAL/C/C++、RTOS 或
厂商资料；不宣称 MISRA、ISO、WCAG、认证或硬件合规。
