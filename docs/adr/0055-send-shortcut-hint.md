# ADR 0055：发送快捷键提示保持展示边界

日期：2026-08-10  
状态：已接受  
增量：UI-1.68

## 背景

发送区已经支持 `Ctrl+Enter`，但用户只能从既有键盘路径或代码行为中发现它。需要在发送控制带内补充可见提示，
同时避免为了提示重复实现快捷键、改变焦点顺序，或重新拥有发送状态。

## 决策

- 在 `presentation/controllers/terminal.py:build_send_bar()` 中装配一个 `QLabel#sendShortcutHint`。
- 提示固定为 `Ctrl+Enter 发送`，提供一致的 AccessibleName、AccessibleDescription 和 tooltip。
- 控件为 `NoFocus`、无点击动作、不暴露为 `window._*` facade；既有快捷键注册、send action、send gate 和 ViewModel
  仍由原 owner 负责。
- `theme_stylesheet_base.py` 与 `theme_variant_shell.py` 均覆盖 keycap 的文字、背景、边框和圆角；不使用系统 palette
  的默认白色 surface。尺寸约束为 176–184 px，兼顾字体内边距与发送栏响应式空间。

## 被拒绝的方案

- 不在提示控件上重新绑定 `Ctrl+Enter`，避免双重 action、发送 gate 分叉和焦点语义漂移。
- 不把提示写入 `MainWindow` facade 或 ViewModel，避免 presentation 文案反向成为业务状态。
- 不新增动画 timer；快捷键说明是静态可发现性信息，发送状态与已有共享动效边界保持独立。

## 验证

- `scripts/check.ps1`：通过，147 个源码文件均不超过 1000 行，主题 token audit 通过。
- `python -m compileall -q src`：通过。
- Qt offscreen 短时布局 vector：760/952/1152 内容宽度均通过，文字区宽度不小于字体度量，`NoFocus` 与 `role=subtle` 保持。
- 未启动主窗口或 EXE，未执行完整 GUI/HIDPI/读屏/真实硬件验收；这些项目需在授权环境执行。
