# UIUX-046: QSS 样式一致性审查报告

> 审查人: UI/UX 产品体验师
> 审查范围: 迭代 #45 新增/修改的 QSS 样式
> 涉及文件:
>   - `resources/themes/dark_terminal.qss`
>   - `resources/themes/modern_dark.qss`
>   - `resources/themes/light.qss`
>   - `src/serial/BookmarkWidget.cpp` / `.h`

---

## 1. otaBrowseBtn 与其他浏览按钮的一致性

**结论: 通过**

项目中存在两个"浏览/选择文件"按钮: `otaBrowseBtn` 和 `bgSelectImageBtn`。
两者的样式在三个主题中完全对齐:

| 属性 | otaBrowseBtn | bgSelectImageBtn |
|------|-------------|-----------------|
| 结构 | 次背景 + accent 文字 + border 描边 | 次背景 + accent 文字 + border 描边 |
| hover | 背景提亮 + accent 边框 | 背景提亮 + accent 边框 |
| pressed | 更深背景 | 更深背景 |
| disabled | 次背景 + muted 文字 + muted 边框 | 次背景 + muted 文字 + muted 边框 |
| padding | 6px 12px | 6px 12px |
| font-size | 12~13px | 12px |

唯一的微小差异: `otaBrowseBtn` 在 dark_terminal 中 font-size 为 13px，而 `bgSelectImageBtn` 为 12px。
这属于合理差异(OTA 面板按钮稍大)，不构成一致性问题。

---

## 2. frameMoveUpBtn/frameMoveDownBtn 与 frameAddFieldBtn/frameRemoveFieldBtn 视觉一致性

**结论: 通过**

四组按钮共享统一的结构模板，仅在文字颜色和 hover 态边框色上有语义区分:

| 按钮 | 默认文字色 | hover 边框色 | 语义 |
|------|----------|------------|------|
| frameAddFieldBtn | accent | accent | 添加(正面操作) |
| frameRemoveFieldBtn | error | error | 删除(危险操作) |
| frameMoveUpBtn | text-primary | accent | 导航(中性操作) |
| frameMoveDownBtn | text-primary | accent | 导航(中性操作) |

视觉层次正确: 添加/删除通过颜色传达操作语义，上移/下移为中性操作使用默认文字色。
结构属性(border-radius, padding, font-size)在三个主题中完全一致。

**注意点**: modern_dark 中四组按钮的 `background-color` 使用的是 `#1a1b26`（最深背景），
而 dark_terminal 中使用 `#313244`（次背景），light 中使用 `#ffffff`（白色）。
这是各主题色板体系的正确映射，不是一致性问题。

---

## 3. baudCombo 可编辑 QLineEdit 等宽字体影响范围

**结论: 通过（无副作用）**

三个主题文件中均有以下规则:

```css
/* 可编辑ComboBox内部QLineEdit使用等宽字体(波特率等数字输入) */
QComboBox QLineEdit {
    font-family: Consolas, "Courier New", monospace;
}
```

此选择器 `QComboBox QLineEdit` 仅匹配作为 QComboBox 子控件的 QLineEdit 实例。
独立的 QLineEdit 不会受影响，因为 QSS 的后代选择器要求父子关系。

**当前项目中可编辑的 ComboBox**: 波特率(baudCombo)、数据位等配置项。
这些控件确实需要等宽字体以便对齐数字，设计意图正确。

**潜在风险**: 如果未来有非数字输入的可编辑 ComboBox（例如输入备注文字），
等宽字体可能不合适。建议在注释中注明此规则的适用范围，或改用
`QComboBox#baudCombo QLineEdit` 进行精确限定。

---

## 4. 面板容器透明背景

**结论: 通过**

三个主题文件中 PanelManager 面板容器列表完全一致:

```css
QWidget#serialConfigPanel,
QWidget#dataStatsPanel,
QWidget#protocolViewPanel,
QWidget#frameEditorPanel,
QWidget#chartWidgetPanel,
QWidget#otaWidgetPanel,
QWidget#terminalPanel,
QWidget#searchBarPanel,
QWidget#bookmarkWidgetPanel {
    background-color: transparent;
}
```

所有 9 个面板均在三个主题中同步声明为透明背景。
设计意图正确: 面板容器透明，让各面板内部控件自主控制背景色，
避免容器背景覆盖磨砂玻璃效果。

`bookmarkWidgetPanel` 已正确包含在列表中。

---

## 5. 三主题样式同步检查

**结论: 通过（无遗漏）**

逐项对比三个主题中的以下模块，确认选择器名称、伪状态数量、属性结构完全对齐:

| 模块 | dark_terminal | modern_dark | light | 同步状态 |
|------|:---:|:---:|:---:|:---:|
| BookmarkWidget 全套 | 有 | 有 | 有 | 同步 |
| PanelManager 容器列表(9项) | 有 | 有 | 有 | 同步 |
| FrameEditor 按钮组(4对) | 有 | 有 | 有 | 同步 |
| OTA Widget | 有 | 有 | 有 | 同步 |
| otaBrowseBtn | 有 | 有 | 有 | 同步 |
| QComboBox QLineEdit | 有 | 有 | 有 | 同步 |
| ToastWidget 后备样式 | 有 | 有 | 有 | 同步 |
| NavIndicatorWidget | 有 | 有 | 有 | 同步 |
| BookmarkWidget 对话框 | 有 | 有 | 有 | 同步 |

所有新增样式在三个主题中完全同步，无遗漏项。

---

## 6. bookmarkList 字体问题

**结论: 通过（等宽字体选择正确）**

```css
QListWidget#bookmarkList {
    font-family: "Consolas", "Courier New", monospace;
    font-size: 12px;
}
```

bookmarkList 使用等宽字体是正确的。列表项格式为 `"HH:mm:ss.zzz  标签文本"`，
时间戳部分的冒号和数字在等宽字体下能完美对齐，确保视觉上的时间列对齐。
这与 TerminalWidget、otaLogView、framePreviewLabel 等数据展示区域的设计语言一致。

非数据类列表（如 QTreeView 导航树）正确使用 UI 字体 "Microsoft YaHei UI"，
两者不冲突。

---

## 7. BookmarkWidget 按钮层次

**结论: 通过（三层次正确）**

BookmarkWidget 的三个按钮严格遵循 CLAUDE.md 6.6 节的按钮体系规范:

| 按钮 | 层次 | QSS 特征 | 语义 |
|------|------|---------|------|
| bookmarkAddBtn | 主要按钮 | 实心 accent 背景 + 深色文字 + 无边框 | "添加"是主要操作 |
| bookmarkRemoveBtn | 次要按钮 | 次背景 + 描边 + hover 时 error 色提示 | "删除"是次要操作，hover 提示危险 |
| bookmarkClearBtn | 幽灵/危险按钮 | 透明背景 + muted 文字 + 描边 + hover 变 error | "清空全部"是高危操作，低调显示 |

三个主题中按钮层次完全一致:
- bookmarkAddBtn: accent 实心填充
- bookmarkRemoveBtn: 次背景 + accent 描边，hover 变 error
- bookmarkClearBtn: 透明 + muted，hover 变 error

这与项目中其他危险操作按钮（otaCancelBtn、chartClearBtn、bgResetBtn）的
视觉语言完全一致: 透明/低调默认态 + hover 时 error 红色警示。

---

## 总结

| 检查项 | 状态 | 备注 |
|--------|------|------|
| otaBrowseBtn 浏览按钮一致性 | 通过 | 与 bgSelectImageBtn 结构对齐 |
| MoveUp/Down vs Add/Remove 视觉一致 | 通过 | 语义色彩区分正确 |
| baudCombo 等宽字体影响范围 | 通过 | QSS 后代选择器无副作用 |
| 面板容器透明背景 | 通过 | 9 个面板三主题同步 |
| 三主题样式同步 | 通过 | 所有新增模块无遗漏 |
| bookmarkList 字体 | 通过 | 等宽字体用于时间戳对齐 |
| 按钮三层次 | 通过 | 主/次/危险层次分明 |

**发现的问题: 0 个**
**建议改进: 1 个低优先级建议**

> 建议: `QComboBox QLineEdit` 的等宽字体规则是全局的，
> 未来如有非数字可编辑 ComboBox 可能受影响。可考虑改用
> `QComboBox#baudCombo QLineEdit` 等精确选择器，但当前无实际副作用。
