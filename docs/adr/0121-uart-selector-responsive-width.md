# ADR-0121：UART selector 响应式宽度边界

## Status

Accepted

## Date

2026-08-11

## Context

980×680 的链路 / 连接页出现真实横向溢出：UART 八列 grid 的最小宽度为 1013px，
而可用内宽为 908px，`QScrollArea.horizontalScrollBar().maximum()` 达到 105。
主要贡献来自可编辑端口 combo 的 240px presentation 上限和长文案流控 combo
约 218px 的 size hint；这会把配置页右侧控件推到视口之外。

## Decision

继续由 `presentation/controllers/connection_builder.py` 的 `_configure_bounded_combo()`
统一负责 presentation width。UART 端口上限收敛为 200px；数据位、校验、停止位和流控
分别使用 84/110、84/150、84/130、110/160px 的 min/max 边界；既有波特率保持
110/150px。下拉仍不可手输（端口除外），itemData、currentIndexChanged、tooltip、
accessibility、连接 gate 和真实选项不变。关闭态可能对长标签做 Qt elide，但 popup 与
accessible description 仍提供完整选择语义。

## Alternatives Considered

### 在 resizeEvent 中动态重排 UART 为多行布局

拒绝：需要新增布局状态和 resize 生命周期，改动面大于当前根因；bounded combo 已能在
980/1180 两个支持尺寸内稳定闭合。

### 缩小全局字体或删除长选项

拒绝：会损害全局可读性或改变用户可选值，不能把响应式责任转移给主题/业务。

### 保留横向滚动条

拒绝：连接参数是主要入口，用户不应通过滚动寻找端口、波特率或连接动作；也违背现有
980px 响应式门禁。

## Consequences

- UART grid 最小宽度收敛到 872px，980px 内宽 908px 下不再产生横向滚动。
- 长流控/校验标签在闭合 combo 中可能被 elide，但下拉选择、tooltip 和 accessibility
  语义保持；没有新增业务状态、timer、线程、I/O、依赖或 DTO。

## Verification

三主题 × 980×680/1180×780 × UART/TCP Client/TCP Server/UDP/BLE GATT/J-Link RTT，
并覆盖四个工作区：60 个组合检查全部 `hmax=0`、`exact-white=0`；UART itemData 和
连接页布局通过，视觉证据为 `build/ui_review_ui134_connection_responsive.png`。
scripts/check.ps1、compileall、Ruff、源码行数与 onefile provenance 通过。架构师线程
`019feda9-d39f-7d11-9433-1a6c34ea1ef3` 超时，未计为独立通过；父代理完成 owner、
可读性、响应式、accessibility、行为保持和简化评估。未修改嵌入式 C/C++，public vendor
applicability=N/A，真实硬件验证未运行。

