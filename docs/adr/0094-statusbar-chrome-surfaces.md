# ADR-0094：QStatusBar 原生 chrome 边界主题化

- 状态：已接受
- 日期：2026-08-11
- 范围：`presentation/theme_stylesheet_controls.py`、`presentation/theme_variant_controls.py`

## 背景

SerialForge 已有主题化 `QStatusBar` 和 presentation-only `StatusFooterSurface`，但 Qt 原生 status item 与
内部状态文本没有显式的透明背景、无边框规则。Windows 原生 style 可能在 item 之间绘制默认分隔线或浅色底，
使底部状态栏成为整套二次元主题中最后一处白线回退风险。

## 决策

在稳定的 controls theme owner 中为 base/variant 对称增加：

- `QStatusBar::item` 的透明背景与零边框；
- `QStatusBar QLabel` 的透明背景、零边框、既有 muted token 和有限水平 padding；
- `StatusFooterSurface` 继续由现有 custom painter/lifecycle fan-out 管理，不改变状态事实或动画时钟。

不在 controller 中写 QSS，不新增状态源、DTO、timer、palette、依赖或状态栏替代组件。

## 后果

原生 status text、permanent footer rail、焦点/无障碍语义和关闭生命周期保持 Qt/application 原有 owner；三套主题的
底部 chrome 获得一致的非白色回退。以后新增底部 presentation widget 时必须复用 status bar 的 semantic surface，
不能重新引入 native item 的隐式边界。

## 验证与边界

静态门禁、compileall、ruff、真实组合根 Qt offscreen 三主题 vector、onefile provenance 均通过；vector 未显示 GUI、
未启动 EXE、未连接 UART/网络/BLE/RTT、未执行 OTA 或硬件验收。本轮只有 Python/Qt presentation 代码，没有嵌入式
C/C++、MCU、bootloader、Flash 或 vendor target 修改；vendor public source applicability=N/A，不声明 MISRA/ISO/硬件合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
