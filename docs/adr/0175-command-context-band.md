# ADR-0175：命令上下文带与独立操作行

## 状态

已接受（ARCH-124 / UI-1.197，2026-08-12）

## 背景

命令页原先把发送历史、批量命令和批量操作连续堆叠在顶部，窄窗口下选择控件与操作按钮争抢横向空间，视觉节奏容易挤成一条长横排。
本轮需要改善响应式密度，同时保留已有控件、行为和唯一共享 120Hz 动效时钟。

## 决策

- 新增 `presentation/command_context_band.py::CommandContextBand` 作为选择上下文的唯一 presentation geometry owner。
- owner 接收 builder 已创建的两个 selector cell，只使用 `QGridLayout` 在宽屏两列与窄屏单列之间切换；切换阈值由既有 cell 的
  `minimumSizeHint()/sizeHint()`、spacing 和 margins 推导，不使用窗口魔法像素。
- 使用 `QLayout.SizeConstraint.SetNoConstraint` 允许布局收缩进入 compact mode；历史 ComboBox 的 220px 最小宽度保持不变；
  wrapper 不增加 `role=surface`，避免嵌套卡片视觉与白色 fallback。
- 五个批量操作按钮仍由 builder 组成独立 action row；本 ADR 不引入操作行折叠，不改变 signals、bindings、业务执行、focus/tab order、
  accessibility、scroll owner、theme selector 或 MotionController。
- `minimumSizeHint()` 返回当前列模式需要的高度，`sizeHint()` 返回当前模式的自然偏好；FontChange、StyleChange、LayoutRequest 和 resize
  统一经过幂等 `_relayout()`。

## 边界

本 ADR 只覆盖 Python/PySide6 presentation。业务事实、批量定义、执行状态、transport、OTA/AES/RTT/J-Link contract-only/attach-only
继续由原 owner 管理；本轮无 embedded C/C++、固件或硬件改动，public-vendor-source applicability=N/A。

## 评审与验证

架构师 `019ff5a5-cb5d-75b3-9fa6-c41bc2a66802` APPROVE owner/boundary；Terra follow-up
`019ff5a8-f256-7002-b8fc-86de2dab2dd0` APPROVE Qt sizing 与视觉边界；`019ff5b5-7c29-7fa3-9c27-14b251e0e951`
APPROVE 当前列模式 height contract；`019ff5c0-2320-7ec1-99d1-2b7d90444be1` APPROVE docstring-only 修正。
独立 reviewer `019ff5bc-af7c-7f91-bc2d-891292e96db0` 无 Critical/Required，结论 `APPROVE WITH ADVISORIES`；简化 reviewer
`019ff5bc-b25e-7743-9a2b-f0ae71f185f8` 无必须简化项。

compact minimum width `160px`、wide switch threshold `330px`；320/329px 单列，330px 起双列。workspace 三主题两尺寸生命周期矩阵、
style relayout、对象 identity、accessible contract 与 `MOTION_10S=pass`（`1224` frames / `120.000Hz`）通过。compileall、Ruff、
`scripts/check.ps1`、source-limit 与 theme audit 通过。offscreen 证据不等同真实 Windows GUI/HIDPI、显示器合成、EXE startup、硬件或正式发行验收。
