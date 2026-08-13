# ADR-0103：工作区 Tab signal underline 与共享生命周期

日期：2026-08-11  
状态：accepted（UI-1.115）

## 背景

工作区已经具备主题化 Tab、矢量 icon、路线 beacon 和一次性页面淡入，但当前选中 Tab 的反馈主要依靠静态边框。用户需要更清晰的二次元信号层级，同时不能牺牲 Qt 原生 Tab 导航和可访问性。

## 决策

- 新增 `AnimatedWorkspaceTabBar`，继承 `QTabBar`，保留 native paint 与所有原生交互，只在选中 Tab 上叠加短 underline、静态轨道和共享 frame 驱动的 halo/pulse。
- `workspace.py` 通过 `QTabWidget.setTabBar()` 显式组装；`lifecycle.py` 将该 widget 加入已有 `_motion_surfaces()`，不建立第二套动画时钟。
- 动态 surface 只消费 `currentIndex()`、`tabRect()`、`ThemeSpec` 和 frame payload；不读取 ViewModel、不派发业务 signal、不改变 `currentChanged` 或 Tab icon。
- `stop()` 只冻结装饰动画；低动效、暂停、隐藏、最小化和关闭继续由既有 MotionController/lifecycle 规则处理。

## 后果与限制

该切片把职责收敛为：Qt 负责导航，workspace 负责组装，Tab surface 负责绘制，lifecycle 负责 frame/stop fan-out，主题系统负责语义色。没有新增 timer、状态源、registry、线程、I/O 或跨层依赖。真实 GUI/EXE 启动、硬件、OTA、RTT/J-Link 和正式发行验收仍未运行。

## 验证

`UI115_WORKSPACE_TAB_RAIL_VECTOR_PASS`（3 themes、4 tabs、native navigation、stop pass）、`UI115_AUDIT_RENDER_PASS`（1180x780）、`scripts/check.ps1`、compileall、Ruff 和 provenance verify 均通过。架构师与独立质量审查线程超时，未将超时记作通过；父代理完成行为保持、复用/简化、绘制边界和行数审查。本轮不包含嵌入式 C/C++，embedded applicability=N/A。
