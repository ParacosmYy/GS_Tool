# ADR-0112：链路连接状态轨道终态 marker

- 日期：2026-08-11
- 状态：accepted for local engineering build
- 范围：presentation-only UI-1.125

## 决策

在 `ConnectionStatusRail` 既有四节点路径上增加两个非颜色终态 affordance：`open` 在末节点绘制
success 完成勾，`error` 在首节点绘制 error 叉号。opening/closing/open 的共享 frame 彗尾与脉冲
保持现有节奏，closed/discovered 不增加新动画。

## 边界与取舍

- marker 只读取已有 `_state`、`_phase`、`_animated` 和 ThemeSpec，不创建连接状态源或业务判断。
- 不新增 timer、MotionController、线程、I/O、控制器回调、外部资源或 OTA/AES/RTT/J-Link 依赖。
- `stop()`、reduced-motion、暂停、隐藏/最小化/关闭、NoFocus、鼠标透明、可访问状态文本和
  980/1180 布局保持不变；几何符号补充颜色之外的可读性。
- 架构师线程 `019fed4f-d2a8-7d33-9cdd-b536efe3e106` 在限定窗口内超时，父代理完成 owner、
  token、重绘、性能、accessibility 和简化审查。

## 证据

三主题真实 production host（`QWidget#appRoot` + `QFrame#connectionControlBand`）六状态、动态
帧、stop 回退和 exact-white=0 vector 通过；`scripts/check.ps1`、compileall、Ruff、onefile package
与 provenance verify 通过。canonical artifact 为 `dist/release/0.1.0/core/onefile/app/SerialForge.exe`，
47,942,017 bytes，SHA-256 `ACC5FCA833E17EF0D648FCBE7CB7DB1ECB8557B43BD4C920371EF2CCDEF49142`。
