# ADR-0119：批量命令空态文案几何

## Status

Accepted

## Date

2026-08-11

## Context

命令管理空态卡片会在专注模式中使用父级剩余空间，原有三个 QLabel 使用默认纵向
`Preferred`，在 1180×780 下分别被拉伸到约 119px；实际 eyebrow/title/hint 只需要约 14/19/30px。
文字因此被分散成三段很高的空白区域，CTA 与 glyph 的关系也不再紧凑。

## Decision

在 `presentation/command_batch_empty_state.py` 的 `CommandBatchEmptyState` 内，只把
`_eyebrow`、`_title`、`_hint` 三个 copy QLabel 的纵向 size policy 设置为 `Fixed`，保留
空态卡片自身 `Expanding/Preferred`，让卡片继续承载大尺寸 empty state；glyph、CTA、共享
MotionController frame、`new_requested` signal、文本投影和无障碍契约均不变。

## Alternatives Considered

### 给整个空态卡片设置 Fixed 高度

拒绝：会丢失命令页在专注模式下的可用留白和视觉容器，且与终端/组件空态的空间策略不一致。

### 删除 glyph 或 CTA，只保留一行文案

拒绝：会降低第一步操作的可发现性，也破坏现有二次元几何 affordance 和键盘可达 CTA。

### 把空态文案移动到全局状态栏

拒绝：命令状态和下一步操作应在命令工作区内就地表达，不应扩散到 shell 状态区域。

## Consequences

- 空态卡片继续可扩展，但 copy 文字按真实内容高度渲染，垂直节奏更稳定。
- 长 hint 仍由 QLabel word-wrap 计算高度；主题、动画、焦点、signal、命令 DTO 和业务动作不变。
- 不新增状态源、timer、线程、I/O、依赖或 OTA/AES/RTT/J-Link 行为。

## Verification

真实组合根覆盖三主题与 980×680/1180×780：eyebrow/title/hint 高度稳定为 14/19/30px，
长 hint 投影不裁切，当前页 horizontal maximum=0，exact-white=0；视觉证据为
`build/ui_review_ui132_command_empty.png`。scripts/check.ps1、compileall、Ruff 与 onefile
provenance 通过。架构师线程 `019fed95-29fe-7e73-b091-0acf973dbd97` 超时，未计为独立通过；
父代理完成 owner、响应式、可访问性、五轴质量和简化评估。未修改嵌入式 C/C++，public vendor
applicability=N/A，真实硬件验证未运行。
