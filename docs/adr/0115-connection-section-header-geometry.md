# ADR-0115：连接页 section 标题固定垂直几何

## Status

Accepted

## Date

2026-08-11

## Context

UI-1.127 的专注设置模式为连接配置释放了完整纵向空间，但在 980×680 下，连接页的 UART
section 标题使用默认 `QLabel` 垂直 size policy，会吸收父级剩余空间并被拉伸到约 106px。标题
和参数面板之间因此出现大块空白，降低配置密度，也让网络/BLE 的同一布局边界不稳定。

## Decision

在 `presentation/controllers/connection_builder.py` 内增加 `_section_label()`，统一创建 UART、
网络和 BLE section 标题。保留 `role="section"`，设置横向 `QSizePolicy.Preferred` 与纵向
`QSizePolicy.Fixed`，让标题高度由字体和主题 padding 决定（当前真实组合根为 26px）。该策略
只属于连接页组装 owner，不扩散到全局 QSS 或通用布局工具。

## Alternatives Considered

### 在 QSS 中给所有 section QLabel 设置固定高度

拒绝：会影响协议、命令和扩展页的不同内容密度，并把连接页特例泄漏到共享主题模板。

### 在 resizeEvent 中动态修正标题高度

拒绝：引入持续尺寸状态和生命周期耦合；布局 policy 能在首次布局和窗口 resize 时自然收敛。

### 为每个连接方式复制一套 section 标题设置

拒绝：三处重复且容易产生 UART/网络/BLE 漂移；单一 `_section_label()` 保持连接 builder 内高内聚。

## Consequences

- 连接页 section 标题不再吞掉剩余高度，专注模式的配置空间直接给参数面板使用。
- 不增加 timer、动画、业务状态、transport 分支、依赖或 I/O；主题、焦点和无障碍语义保持原样。
- 需要把 section title/panel 几何加入三主题、980/1180、六种连接方式的 UI 回归向量。

## Verification

真实组合根 36 组通过：标题高度为 26px，active title/panel 无重叠，horizontal maximum=0，
exact-white=0；`scripts/check.ps1`、compileall、Ruff、onefile provenance 均通过。架构师线程
`019fed71-9d02-7731-b268-c48f59327b28` 超时，未计为独立通过；父代理完成五轴审查与简化评估。
未修改嵌入式 C/C++，public vendor applicability=N/A，硬件验证未运行。
