# ADR-0104：主窗口 action signal rail 统一

日期：2026-08-11  
状态：accepted（UI-1.116）

## 背景

SerialForge 的连接、协议、终端和批量页面已经有状态 rail 与 busy rail，但许多普通动作仍是未接入共享 frame 的原生按钮，页面之间的视觉反馈密度不一致。需要统一装饰层，同时保持用户熟悉的 Qt action 语义。

## 决策

- 连接配置保存/删除、BLE 读取、协议应用/重置、回放开始/停止、错误清除、终端清空、发送、快捷命令保存、发送历史清除和批量新建/编辑/删除/执行复用 `ActionRailButton`。
- `ActionRailButton` 仍继承 `QPushButton`，只消费 ThemeSpec 和 shared frame，绘制 signal rail；不改变原有 callback、signal payload、objectName、enabled gate、focus/Tab、文本或业务语义。
- `lifecycle.py` 通过显式 widget 名称收集 16 个动作；`BusyActionButton` 保持原异步 busy contract，不把 busy 状态搬进通用 action surface。
- 动画是 presentation-only：没有新的 timer、MotionController、registry、状态源、DTO、线程、I/O 或跨层 callback。

## 后果与限制

动作 owner 仍然高内聚，公共绘制逻辑集中，lifecycle 统一管理静态回退；新增动作必须明确选择普通 rail 或 busy rail，不能在 controller 内复制绘制代码。真实 GUI/EXE 启动、硬件、OTA、RTT/J-Link 和正式发行验收仍未运行。

## 验证

`UI116_ACTION_RAIL_VECTOR_PASS`（3 themes、16 actions、shared lifecycle、stop pass）、`UI116_AUDIT_RENDER_PASS`（1180x780）、`scripts/check.ps1`、compileall、Ruff 和 provenance verify 均通过。架构师与独立质量审查线程超时，未将超时记作通过；父代理完成行为保持、复用/简化、owner 边界和行数审查。本轮不包含嵌入式 C/C++，embedded applicability=N/A。
