# ADR 0047：Component 空态卡片与近场 Profile/Codec CTA

日期：2026-08-10  
状态：accepted  
范围：`presentation` Component telemetry surface

## 背景

Component 没有可显示帧时，旧界面同时保留空表头和一行普通空态文字；用户需要在较远的工具栏按钮与提示之间
来回寻找下一步。这个问题属于 presentation 层级和引导，不需要改变 codec、过滤或 parser 行为。

## 决策

- 用 `ComponentEmptyStateSurface` 替换普通 `component_empty` QLabel：保留兼容的 `setText()`、`text()`、`clear()`
  接口，将 controller 的完整文案拆成标题/提示，并加入资源无关的 pipeline glyph。
- 卡片公开无参数 `load_requested` presentation intent，直接接入既有 `on_load_component_codec` callback；它不
  读取 ViewModel，也不执行文件选择或 codec 加载。
- 空态显示时由 `derived_data.py` 隐藏空表头；已有 visible rows 时恢复原生 QTableWidget，保留行选择、过滤、导出
  和无障碍表格契约。
- `set_derived_controls_enabled()` 只把已有 source gate 传给 CTA；卡片和 `ActionRailButton` 共享 lifecycle 的
  MotionController frame/stop，无本地常驻时钟。

## 被拒绝的替代

- 仅把旧 QLabel 换成更长文案：无法解决空表头占位和 CTA 距离问题。
- 在空态卡片中直接调用文件对话框或 ViewModel：会让 presentation surface 夺走 action owner，扩大耦合。
- 新建通用的派生数据状态机：当前只需表达空态和现有 visible rows，通用状态机会复制业务事实。

## 后果与验证

空态高度从约 28 px 增加到约 98 px，并把 Profile/Codec 下一步放到用户当前视线；有数据时恢复原生表格，
不改变数据管道。验证包括源码质量门禁、无数据空态/CTA 信号、空表头互斥、共享 frame/stop、三主题 1180×780
截图无近白像素及 onefile/provenance。真实设备、读屏、硬件和正式发行验收仍不在本 ADR 范围内。
