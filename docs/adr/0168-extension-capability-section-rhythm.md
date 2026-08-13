# ADR-0168：扩展工具站能力分区节奏

## 状态

已接受（ARCH-117 / UI-1.190，2026-08-12）

## 背景

ARCH-116 解决了能力卡列数和单卡孤岛，但扩展页仍把每组标题、说明和卡片网格作为根布局的三个平级项。
在三组能力连续出现时，视觉层级不够清晰，用户会感知为组件挤在一条长流中。

## 决策

在 `embedded_extension_panel.py` 内增加 `_build_capability_section()`，用一个无状态、不可聚焦的
`QFrame#extensionCapabilitySection` 统一承载既有 title、hint 和 `ResponsiveCapabilityGrid`：

1. 分区只拥有 presentation 外框与垂直节奏，不拥有卡片选择、详情、滚动或业务状态；
2. 上下 margin 为 10px、内部 spacing 为 7px，水平 margin 为 0，以保留 1180px OTA 三卡三列；
3. 不新增 QSS、timer、动画或独立 MotionController；既有主题的 `role="surface"` 语义继续提供背景；
4. 原生 QScrollArea、卡片 identity、焦点/accessibility/Tab 顺序、OTA/AES/RTT/J-Link 边界保持不变。

## 被否决的替代方案

- 只调根布局 spacing：无法形成每个能力组的视觉 ownership；
- 给 section 增加水平 padding：会使 1180px OTA 三卡退回两列；
- 在 helper 内复制卡片 selection/detail 或创建新的动画：破坏高内聚低耦合边界；
- 用 nested scroll 或 geometry animation：增加滚动/时钟竞争，不能解决静态层级问题。

## 验证与审查

架构师 `019ff4d6-6e24-74a2-941a-b406376c5763` 批准分区 owner 边界；架构师
`019ff4dd-c011-7891-bcd4-5924116a0263` 批准水平 margin 修正。独立 reviewer
`019ff4df-b76b-7c81-8cbd-2ea3ae392e5d` 在等待窗口内未返回，随后关闭，未形成外部结论；父代理完成
correctness、architecture、accessibility、performance/lifecycle、theme visual、readability 六轴审查与
behavior-preserving simplification assessment。无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。

Qt offscreen 分区矩阵 318 checks、生命周期矩阵 38 checks 均 0 failures；静态 check、compileall、ruff、
source-limit 通过。`local-arch-117` onefile 已覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；
三者均为 `48,031,424` bytes，SHA-256 为
`4488BDB5E31EEBF6C61D9BE4925C829E64A090E9C158396E6CA3B1A9F98400C0`，archive listing SHA-256 为
`ED4576D41DB716016BD327B502599A4E3580510BD990902B3F71C997C1A53621`，provenance verify 通过；签名
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。真实 GUI/HIDPI、EXE startup、硬件、
签名和 OTA/RTT 实连未运行。
