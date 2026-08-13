# ADR-0170：连接控制带响应式布局 owner

## 状态

已接受（ARCH-119 / UI-1.192，2026-08-12）

## 背景

连接页控制带原先使用固定五列 `QGridLayout`。在 980px 窗口中，传输方式、快速配置和连接动作共享一条
过长横排，虽然没有越界，却形成视觉拥挤；连接摘要与链路 rail 也缺少明确的窄屏节奏。

## 决策

在 `controllers/connection_builder.py` 内增加无状态 `_ResponsiveConnectionBand`：

1. 只登记并重排已有 QWidget，不创建第二份控件或状态源；
2. 通过控件自身 `minimumSizeHint()/sizeHint()/minimumWidth()` 与现有 spacing/margin 推导 compact 断点；
3. 常规模式保留既有三行结构，紧凑模式把 transport/连接动作与 preset/保存删除拆成两行；
4. preset context 与 status rail 在紧凑模式跨满内容列；
5. 继续复用唯一 `MotionController`，不新增 timer、动画、滚动 owner 或业务 coupling；
6. `ConnectionShellBindings`、signal/callback、focus/accessibility、控件 identity 和 UART/OTA/debug 语义不变。

## 被否决的替代方案

- 在窗口或业务 controller 中写死 980/1180 像素断点：会把几何策略泄漏到错误 owner，并不能适应字体/主题变化；
- 仅通过 QSS 缩小字体和 padding：不能改变固定列的布局竞争，且会损害可读性；
- 为窄屏创建另一套控件：会复制 signal、accessibility 和对象状态，增加生命周期风险；
- 为布局变化增加 timer 或动画：布局重排不需要额外时钟，会扩大共享 120Hz 帧预算。

## 架构、审查与简化记录

架构裁决 `019ff507-084e-7941-9583-f60b8fbda2ca` 正式 `APPROVE`，要求断点使用控件 sizing contract；局部伸缩权
修正由 `019ff50d-87f5-7110-98c9-75f7d35b03fc` `APPROVE`。此前三次较宽架构审查线程超时关闭，未形成结论；
独立 reviewer `019ff50f-a11e-7042-b7e9-08f03167909e` 等待超时后关闭，未形成外部 findings。父代理完成
owner、行为保持、生命周期、可访问性、性能、security/readability 六轴审查与 simplification assessment。
本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。

## 验证与交付

三主题×六尺寸×正反向 resize 共 72 checks、生命周期矩阵 21 checks，均 0 failures；`scripts/check.ps1`、
compileall、Ruff 通过。`local-arch-119` onefile 已覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；
canonical、root、root-latest 均为 `48,036,144` bytes，SHA-256 为
`CBED1FB08241C4D938796CED3F2AECF84C1A8BBFEC793A9832D489AAFBE495DD`，archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。真实 GUI/HIDPI、EXE startup、硬件和
显示器 120fps 验收未运行。
