# ADR-0179：协议页与命令页局部响应式宽度契约

状态：Accepted  
日期：2026-08-12  
范围：协议页、命令页 presentation owner

## 背景

共享页面 scroll owner 的 `ScrollBarAlwaysOff` 不能替代布局修复。协议页与命令页在窄窗口中出现外层横向溢出时，必须由具体
页面/组件 owner 收敛 intrinsic width，同时保留数据叶子的原生内部滚动能力。

## 决策

- 协议页的 `_ResponsiveProtocolRow` 只存在于 `controllers/protocol.py`，负责组件 header、Dataset actions、Dataset header 等
  本地 action/label row 的 sizing；不抽取到 shared form helper，不持有业务状态、DTO、signals 或滚动容器。
- `protocol_config_context` 以及 `component_table`、`component_preview`、`dataset_preview` 这些已创建的实例设置
  `minimumWidth=0` 与水平 `Ignored`；只让叶子视图向外层让出宽度，不改变其内部 table/editor 的原生滚动语义。
- 命令页的 `_ResponsiveCommandActionRow` 只在 `command_workspace_builder.py` 内排列既有五个动作按钮；
  `_ResponsiveCommandEmptyHeader` 只在 `command_batch_empty_state.py` 内处理 glyph、文案和 CTA 的 regular/compact/narrow 行列。
  业务 signals、按钮 identity、focus/Tab 顺序和 empty-state API 保持不变。
- 不修改共享 `ResponsiveScrollArea`、页面滚动条策略或 `composition.py`。验收指标是外层实际
  `horizontalScrollBar().maximum()==0` 且 content 不超过 viewport；内部数据视图保留其自身滚动能力。
- 架构师评估后决定不为协议页现有局部 `QHBoxLayout` 或 `DatasetCurveWidget` 的 260px 叶子最小宽度做预防性扩大改造；
  只有可见状态真实造成外层溢出时才增加实例级收缩契约。

## 结果与证据

真实组合根离屏矩阵覆盖三主题与 `546/547/560/600/640/768/900/1120/1240px`，连接、协议、命令、扩展四页共 108 行：
全部 `hmax=0` 且 `content.width <= viewport.width`。另外将协议 table、component preview、Dataset preview 与 curve 同时设为可见，
协议页 27 行仍全部通过。相关四个 owner 文件分别为 891、321、474 与 332 行，均低于 1000 行。

架构师 `019ff646-9603-70e3-9d75-5e40bb286b44` `APPROVE`；独立代码审查 `019ff66c-28af-7561-9958-a4da1fea6c48` 最终
`APPROVE`（Critical/Required=0，保留两条 advisory）；简化评估 `019ff66c-290b-7cd0-9d06-736a955dd567` 无必须修改。
本轮未修改共享 scroll owner、业务链路或 embedded C/C++；public-vendor-source applicability=N/A。

