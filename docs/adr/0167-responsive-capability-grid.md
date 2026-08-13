# ADR-0167：扩展工具站能力卡响应式网格

## 状态

已接受（ARCH-116 / UI-1.189，2026-08-12）

## 背景

扩展工具站原先所有 capability section 固定两列。OTA 传输组有三张卡，在宽屏时第二行只剩一张卡，
造成单卡孤岛和大量无语义空区；同时两列宽度过大，卡片信息扫描密度下降。需要改善展示，但不能把
contract-only/attach-only 页面误变成真实 OTA、AES 或探针控制入口。

## 决策

新增 `ResponsiveCapabilityGrid`：

1. 接收已有 card tuple，卡片仍由 extension panel 创建并持有；
2. 计算扣除 margins/spacing 后的 available width，按正的 `card_min_width` 选择最多三列；
3. resize 只在列数变化时按原顺序重排同一批 QWidget，保留 10px spacing 与等权 column stretch；
4. 容器 NoFocus，不参与 card selection/detail/accessibility/tab order；
5. 原生 QScrollArea、focus/overview、shared MotionController 与 OTA/debug contract 边界保持。

## 被否决的替代方案

- 永远固定三列：980px 下会压缩中文文案和焦点可读性；
- 每个 section 自己复制 resize 公式：增加多个 layout owner；
- 重建 card 或把 selection 放进 helper：破坏现有焦点/详情/无障碍契约；
- 使用 nested scroll 或卡片级 animation：增加滚动/时钟竞争，不能解决结构性空区。

## 验证与审查

架构师 `019ff4c3-7140-7ac1-b7ec-de6e736da7b1` 条件批准 helper 边界；架构师 `019ff4c8-5889-7ed3-b938-4c90949ab328`
批准扣除 margins/spacing、正宽度校验和 1 列保底。独立 reviewer `019ff4cc-806a-7380-bb99-680d604241ca`
等待窗口超时关闭，未形成外部结论；父代理完成 correctness、architecture、performance、lifecycle/accessibility、
security、readability 六轴 review 与 behavior-preserving simplification assessment。无嵌入式 C/C++ 改动，
public-vendor-source applicability 为 N/A。

Qt offscreen 网格矩阵 627 checks、生命周期矩阵 662 checks 均 0 failures；静态 check、compileall、ruff、
source-limit 和 theme-token audit 通过。真实 GUI/HIDPI、EXE startup、硬件、签名和 OTA/RTT 实连未运行。

## 交付

`local-arch-116` onefile 已完成并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；canonical、
root、root-latest 均为 `48,030,866` bytes，SHA-256 为
`334406DFEA71D5E4B25FB67088DD3B907250EE8529827CE4AB644FB6FEF780DA`，archive listing SHA-256 为
`ED4576D41DB716016BD327B502599A4E3580510BD990902B3F71C997C1A53621`，provenance verify 通过。
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
