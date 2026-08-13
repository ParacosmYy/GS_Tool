# ARCH-6v / UI-1.138：120Hz 动效与站台布局

日期：2026-08-11  
父代理：Codex  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`

## 交付

- `MotionController` 从 96ms 粗定时器迁移到 120Hz 目标的 PreciseTimer；8/9ms slot 交替，按
  elapsed time 推进 phase，保持低动效、暂停、隐藏、最小化和关闭门禁。
- lifecycle 只向当前可见装饰 surface fan-out，并使用确定性相位错峰，避免 Header、route、状态 rail
  和发送 rail 同步脉冲；没有新增控件级 timer、业务状态或事件总线。
- 实时观测/发送/历史站台布局去除固定提示宽度的反向撑列，补齐 bounded 标题宽度、row height 和
  horizontal/vertical spacing；默认窗口 1240×820，最小高度 720。

## 验证

- `MOTION_LAYOUT_STATIC_PASS`
- `UI138_MIN_SCREENSHOT_PASS build/ui_review_ui138_station_layout_980.png`
- `UI138_SCREENSHOT_PASS build/ui_review_ui138_station_layout.png`
- offscreen cadence probe：`TARGET_HZ=120`、`PreciseTimer`、`singleShot=True`、`26 frames/240ms`
- 980×720 与 1240×820 geometry：station bands 无 overlap，terminal/send 控件均保持非负间距；
  三主题主题 token audit 与 source line limit 通过。
- `PACKAGE_ARCH6V_FINAL`：canonical/root/root-latest 均为 `47,972,295` bytes，SHA-256
  `C9ABCAE17940617A6C81A01F56B91BE09828FC5B996E6181176A82F8656A9DA5`；archive listing SHA-256
  `E01F1A9B8689B822A7806272966DEF1637F1F5F6828ED7EBC8F739E925816876`；provenance verify pass，
  `ROOT_EXE_OVERWRITE_PASS`。
- 已知 PySide6 fonts 目录 warning 仍存在；系统字体注册路径正常。未运行持续 GUI/EXE 启动、读屏、
  真实传输、硬件、刷写、部署或正式签名验收。

## 审查记录

架构师线程 `019ff133-ef65-7270-862c-792cfe1ab8d3` 已调用但在限定窗口内超时关闭，未计为独立通过；
父代理完成五轴架构审查与行为保持型简化评估：单一时钟、可见 fan-out、相位错峰、layout owner、
Qt 生命周期和 accessibility 均保持在 presentation 边界。

嵌入式 C/C++/固件适用性：N/A；无适用 public vendor source；无硬件授权操作；不作 MISRA/ISO/认证声明。

## Assurance gate

```text
EMBEDDED_VENDOR_SOURCE_APPLICABILITY=N/A
EMBEDDED_INDEPENDENT_REVIEW=not_applicable_to_python_ui; architect_timeout_recorded
SIMPLIFICATION_ASSESSMENT=passed_parent_review_no_behavior_change
AUTHORIZED_NON_DESTRUCTIVE_VERIFICATION=compileall+ruff+source-limit+theme-audit+offscreen-geometry+cadence-probe+visual-screenshot+provenance+hash
```
