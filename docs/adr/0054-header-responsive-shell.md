# ADR 0054：Header 品牌层与控制层响应式 shell

## 状态

已接受（UI-1.67，2026-08-10）。

## 背景

Header 同时承载品牌识别、信号场、连接状态、动效偏好和主题选择。UI-1.66 增加 38×38 品牌徽记后，如果继续把所有内容
放在一个 `QHBoxLayout`，在项目规定的 980px 最小窗口中会让多个信息簇争夺同一行宽度；在宽屏中又缺少明确的首屏层级。

## 决策

`workspace.py:build_app_header()` 使用一个外层 `QVBoxLayout` 和两个明确的内层 `QHBoxLayout`：

- `brand_row`：`BrandMarkSurface`、`SERIALFORGE` wordmark、副标题和 `SignalFieldWidget`；
- `controls_row`：连接状态 `statusCluster`、动效控制 `motionControls` 和主题选择 `themeControls`。

现有 widget 创建顺序、`window._*` facade 名称、signals、`install_tab_order()` 对象、ThemeSpec 刷新和 MotionController fan-out
全部保持。该切片只改变 layout ownership/geometry，不新增业务状态、resize timer、布局控制器或跨层依赖。

## 未采用的方案

- 在 `resizeEvent` 中隐藏主题/动效控件：会让用户在窄屏失去可操作入口，也会引入额外的响应式状态和事件生命周期。
- 依赖横向滚动条：Header 是首屏控制带，滚动会隐藏连接与主题入口，不符合工具站的即时操作要求。
- 把所有控件放入自适应 `QGridLayout`：网格可读性弱，且会把品牌和业务控制的变化理由重新耦合到一个隐式布局矩阵。

## 影响与验证

双层 shell 高度增加，但为 980px 最小窗口换取了可读、可操作的控制带；1180px 下品牌层与控制层形成稳定的视觉层级。
布局没有新增 timer 或资源，仍由 Qt 原生 focus/tab order 管理交互。

已通过 `UI167_HEADER_LAYOUT_PASS`：使用短时 Qt offscreen layout vector，在 952px 与 1152px 内容宽度下两行 geometry 均位于
Header 边界内，品牌行高度至少 38、控制行高度至少 30；Ruff、源码行数门禁、compileall 和 workspace/lifecycle 导入通过。
完整 GUI/HIDPI/读屏、真实视觉、启动和硬件验收未在本轮授权范围内运行。
