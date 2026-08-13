# ADR-0117：UART 参数摘要 rail 固定垂直几何

## Status

Accepted

## Date

2026-08-11

## Context

UI-1.129 之后的真实 Qt 几何审计发现，专注设置模式的 UART 参数区中，
`UartTimingSummarySurface` 稳定后高度为约 109px，而一行/两行摘要的 size hint 仅约 38px。
蓝色状态 rail 因此吸收了剩余纵向空间，形成比信息量大得多的空面板，削弱了 UART 字段的层级和
配置密度。

## Decision

在 `presentation/uart_timing_surface.py` 的 presentation owner 内，把摘要 QLabel 的纵向
`QSizePolicy` 从 `Preferred` 收敛为 `Fixed`，保留横向 `Ignored` 以适配 980/1180 的可用宽度。
摘要仍由既有连接 builder 投影，继续消费已有 UART selector 值；不设置硬编码高度、不改变文案、
参数 DTO、连接 gate、无障碍描述或主题语义。

## Alternatives Considered

### 在连接页 QSS 中给摘要 selector 设置固定高度

拒绝：布局几何属于摘要组件自身，向全局 QSS 泄漏会影响其他状态 surface，也无法表达长文案的
size hint 约束。

### 在 `connection_builder.py` 中设置 `setMaximumHeight()`

拒绝：这会让 builder 介入组件内部呈现策略；组件 owner 能直接保持高内聚，后续文案变化也能
通过自身 size hint 收敛。

### 删除摘要 rail

拒绝：该 rail 为用户提供波特率、帧格式和流控的只读确认，删除会降低可发现性和错误预防能力。

## Consequences

- 专注模式下 UART 摘要由 109px 收敛为 38px，更多空间回到真实配置内容，视觉层级更紧凑。
- 横向仍可伸缩，摘要的文字、tooltip、accessible description、主题颜色和 selector 读取路径不变。
- 不新增状态源、timer、动画、线程、I/O、依赖或 OTA/AES/RTT/J-Link 行为。

## Verification

真实组合根覆盖星轨霓虹、月影深海、樱雾夜航三主题及 980×680/1180×780：摘要高度均为 38px，
当前页 horizontal maximum=0，截图 exact-white=0；视觉证据为
`build/ui_review_ui130_uart_summary.png`。scripts/check.ps1、compileall、Ruff 与 onefile
provenance 通过。架构师线程 `019fed87-2dc8-7160-9148-192ecd61726f` 在限定窗口内超时，
未计为独立通过；父代理完成 owner、响应式、可访问性、五轴质量和简化评估。未修改嵌入式 C/C++，
public vendor applicability=N/A，真实硬件验证未运行。
