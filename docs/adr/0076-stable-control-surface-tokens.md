# ADR 0076：稳定控件表面统一使用语义主题 token

日期：2026-08-10

状态：已接受

## 背景

UI-1.85 已清理 stable QSS 中的近白文字和选中色，但 `theme_stylesheet_controls.py` 的表格、表头、状态栏、滚动条和 Tooltip 仍保留一组
局部 hex 颜色。它们虽然只影响默认模板，却会让主题切换后的层级来源不透明，也容易再次形成亮色横带或白色回退。

## 决策

- `theme_stylesheet_controls.py` 只引用 `theme_tokens.py` 已存在的 surface、border、history、interaction、selection 和 text 角色。
- 表头和 Tooltip 的渐变可以保留，但每个 stop 必须来自语义 token；滚动条轨道/corner 使用 `SURFACE_INPUT`，滑块使用 `NEUTRAL_BORDER`。
- `theme_variant_controls.py` 继续负责三套主题的控件覆盖。此切片不新建 palette registry、控件专属 token、状态源、动画时钟或 controller 依赖。
- 通过静态 token audit、compileall、ruff 和 Qt offscreen 三主题 surface vector 作为本轮证据；可见 GUI、硬件和正式发布验收另行授权。

## 结果

默认 stable 模板和主题 variant 之间的依赖方向保持不变，控件视觉层级仍可由 token 调整，后续新增主题只需要补充 `ThemeSpec`/variant 值，避免复制整份 QSS。

