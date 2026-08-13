# ADR-0120：扩展工具站接入路线 glyph 与 typed panel bundle

## Status

Accepted

## Date

2026-08-11

## Context

“扩展 / 工具站”页已经拥有只读能力槽位、OTA 传输/安全和 RTT/J-Link attach-only
边界，但顶部接入概览只有静态指标，和 SerialForge 既有共享动效体系脱节。未来用户
需要把这里理解为调试工具站的入口，同时不能误以为任何真实后端已激活。

## Decision

在 `presentation/embedded_station_overview.py` 内增加资源无关的
`EmbeddedStationRouteGlyph` 与 `EmbeddedStationOverview`。glyph 只绘制三节点接入
路线，颜色来自 `ThemeSpec`，动态帧来自既有 `MotionController`；`stop()` 保留静态
路线。概览继续只消费 `ExtensionStationSummary`，不读取 capability policy，不新增
状态、动作、timer、线程、I/O、密钥、vendor SDK 或 OTA/debug 行为。

`embedded_extension_panel.py` 通过 frozen `ExtensionPanelWidgets` 返回 layout 与
overview，`controllers/workspace.py` 将 typed overview 引用交给窗口，
`controllers/lifecycle.py` 只把它加入既有 `_motion_surfaces()` fan-out。这样组合根
不需要对象搜索，页面 owner 仍然拥有组件，生命周期 owner 仍然拥有动效分发。

## Alternatives Considered

### 在扩展页里创建独立 QTimer

拒绝：会产生第二个动效时钟，破坏低动效、暂停、隐藏、最小化和关闭的统一停止门。

### 让 glyph 读取 capability state 或激活后端数量

拒绝：装饰层不应把只读规划 DTO 解释成运行时连接事实，也不能暗示 OTA/debug 已可用。

### 通过 `findChild()` 隐式注册动效控件

拒绝：typed panel bundle 明确声明组件边界，避免组合根依赖字符串 objectName 搜索。

## Consequences

- 扩展站入口拥有与终端/连接页一致的二次元路线 affordance，静态帧仍清晰可读。
- `ExtensionStationSummary`、七张能力卡、只读文案、accessibility 和 contract-only/
  attach-only 语义保持不变。
- 增加一个 presentation bundle 字段和一个生命周期 surface 引用，但不增加业务依赖。

## Verification

三主题 × 980×680/1180×780 真实组合根通过；overview 为 96×64 route glyph，
horizontal maximum=0；共享 frame 使 glyph 进入 animated，`stop()` 与隐藏窗口保持静态。
视觉证据为 `build/ui_review_ui133_extension.png`。scripts/check.ps1、compileall、
Ruff 与源码行数门禁通过。架构师线程 `019fed9e-6e2f-7722-9a21-b53cebea67ac` 和独立
审查线程 `019feda2-220e-7e13-b4da-7067f2f32bfa` 均超时，未计为独立通过；父代理完成
六轴审查与行为保持/简化评估。未修改嵌入式 C/C++，public vendor applicability=N/A，
真实硬件验证未运行。

