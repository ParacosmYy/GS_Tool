# ADR 0077：稳定控件模板统一使用语义主题角色

日期：2026-08-10

状态：已接受

## 背景

UI-1.86 已收敛表格、状态栏、滚动条和 Tooltip 的稳定颜色，但输入、按钮、Tab、终端和空态仍保留大量默认 hex 字面量。虽然 variant stylesheet
通常会覆盖它们，稳定回退仍可能与主题选择脱节，并增加后续嵌入式调试站页面复制样式的成本。

## 决策

- `theme_stylesheet_controls.py` 的状态面、输入/禁用态、ComboBox popup、Menu、按钮、checkbox、workspace Tab、terminal 和空态只引用已有
  `theme_tokens.py` 语义角色。
- stable template 继续描述 selector、几何和默认结构；`theme_variant_controls.py` 继续拥有三套主题的实际值覆盖。
- 不新增 palette registry、控件专属 token、动态状态源、动画时钟、controller、设备库或密钥依赖。
- 用静态 token audit、compileall、ruff 和三主题真实 Qt offscreen 控件组合 vector 作为证据；可见 GUI、硬件和正式发行验收仍需单独授权。

## 结果

稳定回退和主题变体共享同一颜色语义，后续新增 OTA/debug presentation 页面可以复用现有角色，不需要再写一套局部色板；selector、业务 projection、键盘和 accessibility
契约不变。

