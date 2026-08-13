# ADR-0157：工作区模式上下文投影

日期：2026-08-12  
状态：accepted  
范围：`presentation/workspace_context_surface.py`、`presentation/workspace_focus_transition.py`

## 决策

复用 route strip 已有的 `WorkspaceContextLabel`，把现有的 `workspaceShell[mode]` 模式同步投影到
上下文文案：

- 总览：`总览 · 链路配置`
- 专注：`专注 · 解析与遥测`

页面名称仍由现有 Tab index 提供，focus/overview 模式仍由
`workspace_focus_transition.py` 统一拥有。标签只负责呈现，不新增业务状态、timer、scroll owner、
transport/session 依赖或第二个动画时钟；accessible description 和 tooltip 同步解释“暂时收起”
与“仍在后台工作”的语义。

## 取舍

不新增独立模式徽章或新的 route strip 控件，避免在 31px 固定高度内继续挤压滚动提示和路线节点；
不改变窄窗口自动 focus 策略，因为它承担连接配置页的可读性约束。本决策只补足模式可理解性。

## 验证边界

真实 Qt offscreen 三主题、`980×720`、四 workspace、focus/overview 文案矩阵通过；最长文案宽度
`85px`，上下文标签 geometry `158×22`，scroll hint geometry 为 `112×22`（命令页 complete 文案
因 Qt size hint 扩展为 `126×22`），未发生 sibling overlap。GUI/EXE 启动、真实 Windows 可见窗口、
高刷新显示器/HIDPI、高负载、硬件连接和 OTA/RTT 实连仍未运行或未授权。

本轮无嵌入式 C/C++ 修改，public-vendor-source applicability 为 N/A；不作 MISRA、ISO 26262、
ASIL、ASPICE 或认证声明。

## 交付

`local-arch-106` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe`；三者均为 `48,019,150` bytes，SHA-256 为
`F1F6BFC10CDE0103B9B117C69FC14ED9AA28887F00619AD212FA089570223BE9`。archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过；
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
