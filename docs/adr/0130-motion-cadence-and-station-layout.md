# ADR-0130：120Hz 共享动效与站台布局边界

日期：2026-08-11  
状态：accepted（presentation-only）  
关联切片：ARCH-6v / UI-1.138

## 背景

旧 `MotionController` 使用 96ms `CoarseTimer`，装饰动画约 10Hz；共享 fan-out 还会向隐藏工作区
页发送 repaint。实时观测和发送带的固定提示宽度又会反向撑大网格列，在最小窗口下形成拥挤的控件组。

## 决策

- 保留唯一 `MotionController` 和现有 lifecycle gate；目标 cadence 改为 120Hz。
- 使用 `PreciseTimer`，按 8/9ms slot 交替调度，使平均 slot 为 8.333ms；phase 根据实际 elapsed
  time 推进，Qt/桌面 compositor 未能按目标刷新时不产生速度跳变。
- `lifecycle.on_motion_frame()` 只向当前 `isVisibleTo(window)` 的既有装饰 surface fan-out，并对
  surface 使用确定性小相位差。相位差只改变绘制时刻，不产生状态源或业务语义。
- 实时观测/发送/历史 layout 由 `terminal.py` 保持 owner，使用 bounded 标题宽度、明确行高和
  horizontal/vertical spacing；bootstrap 默认窗口调整为 1240×820，最小高度调整为 720。

## 不在本 ADR 内

不新增控件级 timer、线程、GPU/外部动画库、业务状态、事件总线或 domain/application layout policy；
不改变连接、发送、记录、回放、Tab、accessibility、OTA/debug contract-only/attach-only 边界。

## 验证与风险

- `MOTION_LAYOUT_STATIC_PASS`：compileall、Ruff、source-limit、theme token audit 通过。
- `UI138_MIN_SCREENSHOT_PASS` 与 `UI138_SCREENSHOT_PASS`：980×720 与 1240×820 offscreen 截图人工查看，
  realtime/send/terminal 无控件重叠，视觉间距恢复；输出位于 `build/` 视觉复核目录。
- offscreen cadence probe：目标 `120`、`PreciseTimer`、single-shot、26 frames/240ms；该数字受
  offscreen/event-loop 调度影响，不能宣称 Windows compositor 固定 120fps。
- `local-arch-6v` onefile 已通过 provenance verify，并覆盖 canonical、`SerialForge.exe` 与
  `SerialForge-latest.exe`；三者均为 `47,972,295` bytes，SHA-256
  `C9ABCAE17940617A6C81A01F56B91BE09828FC5B996E6181176A82F8656A9DA5`，archive listing SHA-256
  `E01F1A9B8689B822A7806272966DEF1637F1F5F6828ED7EBC8F739E925816876`。
- 已调用架构师线程 `019ff133-ef65-7270-862c-792cfe1ab8d3`，限定窗口内超时关闭，未计独立通过；
  父代理完成依赖方向、生命周期、可见性、性能、布局、accessibility 与行为保持型简化审查。

嵌入式 C/C++/固件适用性：N/A；本轮只有 Python/PySide6 presentation 变更，无适用 public vendor
source；无硬件、刷写、部署或破坏性操作。
