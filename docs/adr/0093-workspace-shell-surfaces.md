# ADR-0093：工作区外壳与路线装饰条主题表面

- 状态：已接受
- 日期：2026-08-11
- 范围：`presentation/theme_stylesheet_controls.py`、`presentation/theme_variant_controls.py`

## 背景

工作区真实 `QTabWidget` 已有 Tab、pane 和 overflow 规则，但承载它的
`workspaceShell` 与下方 `workspaceRouteStrip` 没有独立主题边界。两者因此可能依赖父级透明背景或
Qt 原生默认表面，三套主题的层级不稳定，也容易重新出现用户反馈的白色横带。

## 决策

在 presentation theme owner 中为两个稳定 objectName 增加对称 selector：

- base stylesheet 提供稳定的 surface、border、圆角和 history→info 渐变；
- theme variant 只覆盖 `ThemeSpec` 的 `surface`、`border`、`neutral_border`、`history_surface` 与
  `info_surface` 等既有语义 token；
- `workspace.py` 继续只负责把真实 Tab 与 `WorkspaceRouteSurface` 放入 layout-owned shell，
  不接收样式策略；
- route beacon 的固定尺寸、鼠标透明、NoFocus、空 accessibility、Tab index 与共享动效生命周期不变。

## 后果

工作区外壳、Tab pane 与路线条形成稳定的 presentation surface，三套主题都不会回落透明或白色原生表面。
样式仍集中在 controls theme 模块，未新增业务状态、DTO、timer、资源、依赖或导航模型；未来增加工作区页时只需
复用既有 shell contract。

## 验证与边界

`scripts/check.ps1`、compileall、ruff 和真实组合根 Qt offscreen vector 均通过。vector 覆盖三套主题、4 个 Tab、
148×28 route beacon、两个 selector 与整窗 near-white pixel=0；未显示 GUI、未启动 EXE、未连接 UART/网络/BLE/RTT、
未做硬件或正式发行验收。离屏验证中出现的裸 Python 缺少 PySide6、Qt `render` 签名和无参 session close 清理调用问题
均属于验证脚本入口，修正后未修改产品代码。

本轮只有 Python/Qt presentation 代码，没有 MCU、嵌入式 C/C++、bootloader、Flash、真实 OTA 或 debug target 修改；
vendor public source applicability=N/A，不声明 MISRA、ISO 或硬件合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
