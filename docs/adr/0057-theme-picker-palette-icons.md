# ADR 0057：主题选择器显示目标配色预览

日期：2026-08-10  
状态：已接受  
增量：UI-1.70

## 背景

主题切换入口已有文字、tooltip 和 header 色盘，但下拉列表中的每个选项只显示名称。用户需要在切换前快速区分三套
二次元主题，同时保留 Qt 原生 `QComboBox` 的键盘、焦点、选择和无障碍语义。

## 决策

- 新增 `presentation/theme_picker_icons.py`，将每个 `ThemeSpec` 的 surface 和四个 accent token 绘制为 18×18
  resource-free `QIcon`。
- `workspace.py` 在原生 combo 条目完成后调用 `refresh_theme_picker_icons()`；图标通过 `UserRole` theme key 对齐，
  不重写条目文字、tooltip、currentIndexChanged 或主题偏好。
- 图标提供 Normal/Selected/Disabled mode，绘制逻辑不创建 timer、不读取 session/transport 状态，也不加入 lifecycle fan-out；
  每个图标表达目标主题的固定 palette，因此主题切换时无需重复生成。

## 被拒绝的方案

- 不使用 PNG/SVG/GIF/字体或网络资源，避免打包、授权和主题刷新分裂。
- 不把 palette 预览做成第二个可交互控件，避免扩大焦点树和 Tab 顺序。
- 不让 icon 根据当前连接状态变化，避免主题选择器与业务状态耦合。

## 验证

- `scripts/check.ps1`：通过，148 个源码文件均不超过 1000 行，三主题 token audit 通过。
- `python -m compileall -q src`：通过。
- Qt offscreen vector：三个 theme item 均有非空 16×16 actual icon，combo count、itemData key 保持。
- 未启动完整 GUI/EXE；HIDPI、读屏和正式发行验收需授权环境执行。
