# PRD-071 UI Industrial Workbench Refresh

## 1. 背景

当前界面已经具备 BasePanel、IconNavBar、CommandPalette、Toast 等基础设施，但整体观感仍偏“控件堆叠”和单一暗色面板，缺少嵌入式调试工具应有的工作台层级、状态辨识和操作密度。

本 PRD 只处理视觉和可读性，不新增业务功能。

## 2. 目标

把主界面刷新为工业调试工作台风格：

- 导航、终端、面板、状态栏的层级更清楚。
- 连接、发送、危险、禁用等操作状态更容易辨认。
- 输入框、下拉框、表格、树、Tab、GroupBox 的边界和焦点态更稳定。
- 三套主题保持同一类控件的尺寸、圆角、状态语义一致。

## 3. 非目标

- 不新增 QWidget 类。
- 不改业务逻辑、串口协议、数据收发、Controller 行为。
- 不重写 ThemeManager。
- 不引入第二套主题文件或平行资源目录。
- 不修改构建输出目录、可执行文件名或启动入口。

## 4. 约束

- 遵守 `docs/constraints/05-ui-standard.md`。
- 所有改动优先落在 `resources/themes/*.qss`。
- 不在 C++ 中新增 `setStyleSheet()` 硬编码颜色。
- 按钮必须具备 `default / hover / pressed / disabled` 状态。
- UI 改动完成后必须验证 `cmake --build build` 和 `EmbedDebug.bat`。

## 5. 实施范围

本轮只允许修改：

- `resources/themes/dark_terminal.qss`
- `resources/themes/modern_dark.qss`
- `resources/themes/light.qss`
- 本 PRD 文档

如构建暴露既有错误，可按构建线做最小修复，但不得借机扩大 UI 功能范围。

## 6. 验收标准

1. 三套主题都包含 PRD-071 覆盖段，覆盖主窗口、导航树、分割器、状态栏、输入框、按钮、表格、Tab、GroupBox。
2. `QPushButton`、`QLineEdit`、`QComboBox`、`QTreeView`、`QTabWidget` 至少具备清晰的 hover/focus/selected/disabled 样式。
3. `#navTree` 选中态有左侧 accent 指示和背景变化。
4. `#serialPanel`、`#rightWidget`、`#terminalContainer` 的层级清晰，不再全部贴同一块背景。
5. `#sendButton`、`#connectBtn` 保持主操作视觉权重，危险/断开按钮使用 error 语义色。
6. `cmake --build build --config Release` 成功。
7. `EmbedDebug.bat` 能启动 `build/EmbedDebug.exe`。

## 7. 失败条件

- 任一主题 QSS 解析失败导致整体样式丢失。
- UI 控件文字被裁切或按钮状态不可辨认。
- 主题之间控件尺寸明显不一致。
- 构建失败或 `EmbedDebug.bat` 无法启动。
- 修改超出本 PRD 范围。
