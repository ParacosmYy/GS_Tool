# ADR-0148：扩展能力卡保留内容安全高度

日期：2026-08-12

状态：已采用（ARCH-97 / UI-1.170）

## 背景

扩展工具站能力卡在 Python builder 中已经声明了 `122px` 的最小高度，但主题由根窗口统一
应用后，后置通用 `QPushButton` 规则覆盖了这个 widget minimum，实际卡片约 `83px`。标题、
契约状态、摘要和边界说明因此处于被压缩风险，和“可审计工具站”定位不符。

## 决策

在 `theme_stylesheet_extension.py` 的专属 `QPushButton#extensionCapabilityCard` selector 中
显式声明 `min-height: 122px`。builder 继续负责内容和 accessibility，QSS 只负责视觉 contract；
纵向增长由既有外层 `QScrollArea` 承载。三主题 override 复用同一 selector 语义，不新增布局、
scroll、动画或业务状态。

## 验证与边界

三主题 × 980/1240 离屏验证中 7 张卡的实际最小高度为 `144px`，当前页横向滚动范围为 `0`；
卡片选择/详情 projection、只读 DTO、OTA XMODEM/YMODEM/TFTP、AES-GCM/CCM、RTT/J-Link
contract-only/attach-only、共享 MotionController `120Hz/8ms` 均保持。Ruff、compileall、
source-limit 与 theme token audit 通过。

本轮为 Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A；
架构师与独立 reviewer 调用超时关闭，父代理完成五轴 review 与简化评估。不作固件标准或认证声明。
