# ADR-0122：扩展工具站能力分组摘要

## Status

Accepted

## Date

2026-08-11

## Context

扩展工具站已经展示 XMODEM/YMODEM/TFTP、AES-256-GCM/AES-128-CCM 和 RTT/J-Link
的只读能力卡，但接入概览只有总槽位、激活后端和当前动作三个指标。未来继续增加
嵌入式工具能力时，用户需要先判断能力属于 OTA 传输、OTA 安全还是调试输出，不能
只看到一个总数。

## Decision

在 `application/extension_station.py` 增加 frozen、bounded 的
`ExtensionStationGroupSummary`，由唯一的 `extension_station_summary()` 从既有
capability catalog 派生三个分组计数。`ExtensionStationSummary` 只携带 immutable
分组摘要，并校验分组总数与总能力数一致。

`group_summaries` 作为新增可选字段追加在既有 `ExtensionStationSummary` 字段之后，内部工厂
使用 keyword 传值；这样旧的 positional `current_action`、`prerequisite`、`read_only` 前缀
保持兼容。

`presentation/embedded_station_overview.py` 只消费该 DTO，把分组计数投影到只读
metrics grid 和 accessible description；它不解释 `contract_only`/`attach_only`，不
创建动作、timer、后端连接或 OTA/debug 依赖。当前六项 metrics 使用六列网格保持接入
概览首屏可见，未来增加分组超过六项时自动换行，不修改主窗口、能力卡和业务状态。

## Consequences

- 扩展页首屏能直接看到 OTA 传输、OTA 安全和调试输出的槽位数量；
- application 仍是摘要事实的唯一 owner，presentation 仍是纯渲染 owner；
- 当前 catalog 的激活后端仍为 0，所有真实 OTA/J-Link 行为仍保持未激活；
- 当前 overview 保持单行 metrics，未来超过六项时才增加高度；滚动策略和横向宽度边界不变。

## Verification

需覆盖三主题 × 980×680/1180×780、分组总数/总能力数一致性、overview accessibility、
横向 scroll maximum=0、共享 frame/stop/隐藏静态回退，以及 compileall、Ruff、源码行数
和 onefile provenance。未修改嵌入式 C/C++，public vendor applicability=N/A，真实硬件
验证保持未运行。
