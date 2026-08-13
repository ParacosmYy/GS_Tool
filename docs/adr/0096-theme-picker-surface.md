# ADR-0096：主题选择器 semantic surface

- 状态：已接受
- 日期：2026-08-11
- 范围：`presentation/theme_stylesheet_base.py`、`presentation/theme_variant_shell.py`

## 背景

Header 的 `themeControls` 外壳与 `ThemePaletteSwatch` 已经主题化，但真正承载三套主题选择和主题切换 signal 的
`themePicker` 仍继承普通 ComboBox 外观。用户能切换颜色，却看不出它是全局主题入口，禁用/focus 状态也缺少专属层级。

## 决策

为 `QComboBox#themePicker` 增加 base/variant 对称 selector：默认使用 history→input 渐变和既有 history border，
hover 使用紫色 accent，focus 使用 focus token，disabled 使用 disabled semantic token；主题 icon、popup、itemData、
currentIndexChanged 和 `on_theme_changed()` 继续由现有 owner 管理。

## 后果

主题入口在 Header 中形成明确的可操作层级，与旁边的 palette swatch 和一次性 theme transition 视觉连续；不新增颜色
系统、动画时钟、状态源或业务依赖。后续新增主题只需复用同一 selector，不把主题策略分散到 workspace controller。

## 验证与边界

真实组合根 Qt offscreen vector 覆盖三套主题、三个 theme key、三个主题 icon、实际主题 signal、不可编辑、enabled/
disabled 和整窗 near-white pixel=0；静态门禁、compileall、ruff、provenance 与 onefile 打包通过。未显示 GUI、未启动
EXE、未连接真实设备或执行 OTA/J-Link；本轮只有 Python/Qt presentation 代码，vendor public source applicability=N/A，
不声明 MISRA/ISO/硬件合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
