# ADR-0102：派生动作 signal rail 与生命周期 fan-out

日期：2026-08-11  
状态：accepted（UI-1.114）

## 背景

UI-1.113 已为四个派生动作建立了动作专属 tooltip/accessibility contract，但按钮仍缺少与协议、组件、Dataset 视觉轨道一致的动态 signal rail。该反馈属于 presentation affordance，不应把动画状态扩散到协议 gate、业务状态或组合根。

## 决策

- 四个动作继续由 `controllers/protocol.py` 创建，并改为复用 `presentation/action_surface.py:ActionRailButton`；原有 callback、文本、accessible name、tooltip contract 和布局位置保持不变。
- `ActionRailButton` 继续只负责基于 ThemeSpec 的绘制和 `set_frame()`/`stop()` 契约；不新增 timer、状态源、业务判断或跨层依赖。
- `controllers/lifecycle.py` 将四个 widget 纳入既有 `_motion_surfaces()`，统一消费现有 frame 与 stop fan-out；隐藏、最小化、暂停、低动效和关闭仍由同一 lifecycle policy 收敛。
- `controllers/protocol_config.py` 继续拥有 source gate 及 enabled/disabled 文案投影；动画不能改变按钮是否可用，也不能替代业务状态。

## 结果与限制

该边界满足高内聚/低耦合：动作 owner 组装，公共 presentation surface 绘制，lifecycle 负责 fan-out，gate owner 负责可用性。没有引入全局 registry、第二套 MotionController、DTO、线程、I/O 或设备行为。真实 GUI、EXE 启动、硬件、OTA、RTT/J-Link 和正式发行验收仍未运行。

## 验证

`UI114_DERIVED_ACTION_RAIL_VECTOR_PASS`（3 themes、4 actions、enabled/disabled、lifecycle shared、stop pass）、`scripts/check.ps1`、compileall、Ruff 和 provenance verify 均通过。架构师与独立 UI 复核在限定窗口内超时，未将超时记作通过；父代理完成行为保持、复用/简化、owner 边界和行数审查。该切片不包含嵌入式 C/C++，embedded applicability=N/A。
