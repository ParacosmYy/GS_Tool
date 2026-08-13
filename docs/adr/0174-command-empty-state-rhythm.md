# ADR-0174：命令批处理空态与显式页面垂直节奏

## 状态

已接受（ARCH-123 / UI-1.196，2026-08-12）

## 背景

命令/批处理页在短内容场景会被共享短页居中策略推到 viewport 中部；空态同时占用 flexible stretch slot，造成顶部操作条拥挤与下方大块无层次空白并存。
三步卡片如果等待首次 `resizeEvent` 才重排，还会留下构造阶段同格占位的首帧风险。

## 决策

- `CommandBatchEmptyState` 改为 `Expanding/Preferred` 的自然高度卡片，主说明旁保留一个不抢焦点的三步引导轨：新建 → 添加步骤 → 执行查看。
- `CommandBatchStepRail` 只拥有已有三张 presentation card 的几何排布；初始化与 resize 共用幂等 `_relayout()`，宽度达到 720px 为三列，
  否则为单列；不新增业务状态、信号、焦点控件、timer 或独立动画循环。
- `ResponsiveScrollArea` 增加 `ShortPageVerticalRhythm` 薄契约，默认 `CENTER` 保持历史行为；命令页经 `scroll_page()` 显式传 `TOP`，不按页面
  名称特判，其他页面不改调用。
- 主题只为 `commandBatchStep*` 增加既有 semantic token 的基础样式与 variant override；不使用裸色、不改变共享 `MotionController`。

## 边界

本 ADR 只覆盖 Python/PySide6 presentation。`CommandBatchControlBindings`、ViewModel、CommandBatchService、执行链路、QScrollArea、
transport、OTA/AES/RTT/J-Link contract-only/attach-only 仍由原 owner 管理。embedded C/C++ public-vendor-source applicability=N/A。

## 评审与验证

架构师 `019ff58a-9d3d-75b0-9638-95529059138a` APPROVE；页面节奏与首帧 follow-up `019ff59a-996b-76e0-adbf-48a8633392a1` APPROVE。
独立 reviewer `019ff595-c839-7311-8c47-1a4006951912` 无 Critical/Required，结论 `APPROVE WITH ADVISORIES`；简化 reviewer
`019ff595-cb11-7370-b84c-6d088909da57` 无必须简化项。

`compileall`、Ruff、`scripts/check.ps1` 通过；首次显示与反复 resize 的 520/640/720/980/1180 矩阵无重叠；三主题无白色 palette fallback；
hide/show/close 与 accessible description 通过；10.2 秒共享动效样本为 `1224` frames / `120.000Hz`。onefile provenance verify 通过并覆盖根目录
两个 EXE。offscreen 证据不等同真实 Windows GUI/HIDPI、显示器 120fps、EXE startup、硬件或正式发行验收。
