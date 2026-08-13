# ADR-0118：网络与 RTT 说明 hint 固定垂直几何

## Status

Accepted

## Date

2026-08-11

## Context

六传输真实 Qt 几何审计发现，TCP Client 的网络说明 QLabel 在专注模式稳定后被拉伸到约
151px，RTT 说明被拉伸到约 111px，而长文案实际只需要约 50–67px。UDP、BLE 和 UART 没有
同样的异常，但它们共享连接页的响应式 hint 约束；这会让不同传输方式在切换时出现不一致的
垂直密度。

## Decision

在 `presentation/controllers/connection_builder.py:_configure_responsive_hint()` 内保留
`wordWrap`、120/520px 宽度边界、tooltip 和 accessible description，仅将 QLabel 的纵向
`QSizePolicy` 从 `Preferred` 改为 `Fixed`，让高度跟随当前文案的 size hint。横向继续使用
`Ignored`，由 grid 为长文案提供可用宽度；说明仍是静态 presentation affordance，不拥有传输状态。

## Alternatives Considered

### 通过 QSS 或 resize handler 强制所有网络 panel 高度

拒绝：panel 的高度属于内容布局，硬编码父级高度会把 TCP Server allowlist、BLE 和未来字段
绑定到同一尺寸，破坏响应式边界。

### 删除网络/RTT 说明文案

拒绝：这些文案明确 TCP stream、UDP datagram 和 RTT attach-only 边界，是用户防止误用的重要
上下文，不应通过删文案换取紧凑布局。

### 为每一种传输复制不同的 hint helper

拒绝：会使 TCP/RTT/BLE 的主题、无障碍和布局策略漂移；共享 helper 保持高内聚且改动可回滚。

## Consequences

- TCP Client/RTT 的多余纵向空白消失，长文案仍可换行并保持可读性。
- TCP Server 的 allowlist 区域、UDP、BLE、UART、连接动作和 transport DTO 不变。
- 不新增状态源、timer、动画、线程、I/O、依赖或 OTA/AES/RTT/J-Link 业务行为。

## Verification

真实组合根覆盖三主题、980×680/1180×780、UART/TCP Client/TCP Server/UDP/BLE GATT/J-Link RTT
共 36 组：说明文本按 size hint 收敛，标题/active panel 无重叠，horizontal maximum=0，
exact-white=0；TCP Client 和 RTT 视觉证据为 `build/ui_review_ui131_tcp_hint.png`、
`build/ui_review_ui131_rtt_hint.png`。scripts/check.ps1、compileall、Ruff 与 onefile provenance
通过。架构师线程 `019fed8d-6fdc-7532-8fdd-387290c71924` 超时，未计为独立通过；父代理完成
owner、响应式、可访问性、五轴质量和简化评估。未修改嵌入式 C/C++，public vendor applicability=N/A，
真实硬件验证未运行。
