# UI/UX 观感评审报告 #044

> **评审人**: UI/UX 产品体验师 (角色 #7)
> **评审日期**: 2026-06-01
> **评审范围**: 三套 QSS 主题文件 + ToastWidget.h + SerialConfigPanel.cpp + NavIndicatorWidget.h
> **当前评分**: 43/1000

---

## 一、检查清单总览

| # | 检查项 | 结果 | 说明 |
|---|--------|------|------|
| 1 | 所有控件是否有 objectName | **部分通过** | 见问题 P01 |
| 2 | 三主题样式是否同步 | **部分通过** | 见问题 P02 |
| 3 | 按钮是否有 hover/pressed/disabled 三态 | **通过** | 见下文 |
| 4 | 输入框是否有 normal/focus/error/disabled 四态 | **通过** | 见下文 |
| 5 | 颜色是否使用语义色，无硬编码 | **部分通过** | 见问题 P03 |
| 6 | 间距是否符合 6.3 标准 | **通过** | 见下文 |
| 7 | 字体使用是否正确 | **通过** | 见下文 |
| 8 | 圆角是否统一 (6px) | **未通过** | 见问题 P04 |

---

## 二、通过项

### C01: 按钮三态覆盖 -- 通过

所有按钮类型均覆盖了 hover / pressed / disabled 三态。抽检清单:

- `QPushButton` 通用: hover(L147-149) / pressed(L151-153) / disabled(L155-159) -- dark_terminal.qss
- `QPushButton#sendButton`: hover / pressed / disabled -- 三个主题齐全
- `QPushButton#connectBtn`: hover / pressed / disabled + state 属性变体 -- 三个主题齐全
- `QToolButton`: hover / pressed / disabled / checked -- 三个主题齐全
- `QPushButton[quickCmdBtn="true"]`: hover / pressed / disabled -- 三个主题齐全
- OTA 按钮 (`#otaStartBtn`, `#otaCancelBtn`, `#otaClearHistory`): 三态齐全
- 图表按钮 (`#chartPauseBtn`, `#chartClearBtn`): 三态齐全
- 搜索栏按钮 (`#searchBarCloseBtn`): 三态齐全
- 背景设置按钮 (`#bgSelectImageBtn`, `#bgResetBtn`): 三态齐全

### C02: 输入框四态覆盖 -- 通过

`QLineEdit` 通用样式覆盖了四种状态:
- normal: background-color + border (L134-141)
- focus: border 颜色变化 (L143-145)
- error: `QLineEdit[hasError="true"]` 2px error 色边框 (L153-156)
- disabled: 降低对比度 (L147-151)

`QLineEdit#searchBarInput` 和 `QLineEdit#sendInput` 也有独立的 hasError 状态。
`QSpinBox` 同样覆盖了 hover / focus / disabled 状态。
`QComboBox` 覆盖了 hover / disabled 状态。

### C03: 间距符合 6.3 标准 -- 通过

SerialConfigPanel.cpp 布局验证:
- `mainLayout->setContentsMargins(12, 12, 12, 12)` -- 面板内边距 12px, 符合标准 (8-12px)
- `mainLayout->setSpacing(12)` -- 控件间距 12px, 符合标准 (6-8px 用于同行, 12-16px 用于分组)
- `m_connectBtn->setMinimumHeight(36)` -- 连接按钮 36px, 符合标准 (按钮最小 28px)

QSS padding 验证:
- QPushButton: `padding: 6px 16px` -- 合理
- QComboBox: `padding: 4px 8px` -- 合理
- QLineEdit: `padding: 6px` -- 合理
- QTreeView::item: `padding: 4px 8px` -- 合理

### C04: 字体使用正确 -- 通过

全局字体定义 (`*` 选择器):
- UI 字体: `"Microsoft YaHei UI", "Segoe UI", sans-serif` -- 符合 CLAUDE.md 6.4 标准

终端/代码区域字体:
- QLineEdit (发送输入): `"Consolas", "Courier New", monospace`
- QTextEdit#otaLogView: `Consolas, 'Courier New', monospace`
- QLabel#elapsedLabel: `monospace` (时间显示)

字号验证:
- 界面字号: 12-13px, 符合标准
- 状态栏: 12px, 符合标准
- 工具提示: 12px, 符合标准

### C05: ToastWidget 实现 -- 通过

ToastWidget.h 实现质量高:
- `setObjectName("toastWidget")` + `setProperty("type", ...)` -- QSS 选择器正确
- 颜色全部从 `ThemeManager::SemanticColor` 获取 (Success/Error/Accent), 无硬编码
- 弹出动画: 300ms OutBack -- 符合 CLAUDE.md 6.5
- 消失动画: 250ms InCubic -- 符合 CLAUDE.md 6.5
- 圆角 8px (`kRadius = 8`), 左侧彩色边框 4px -- 视觉精致
- 防抖机制 `showDebounced()` -- 避免通知轰炸, 实用
- 多条吐司自动堆叠 + 消失后重排动画 -- 体验流畅

### C06: NavIndicatorWidget 实现 -- 通过

NavIndicatorWidget.h 实现质量高:
- `setObjectName("navIndicator")` -- QSS 选择器正确
- 透明背景 + WA_TransparentForMouseEvents -- 不干扰导航树交互
- 颜色从 `ThemeManager::SemanticColor::Accent` 获取 -- 无硬编码
- 动画: 250ms OutCubic -- 完全符合 CLAUDE.md 6.5
- 指示线宽度 3px, 圆角 2px -- 精致的 Arc Browser 风格
- 监听 themeChanged 信号自动刷新颜色 -- 主题切换适配
- eventFilter 跟随 navTree resize -- 布局健壮

### C07: SerialConfigPanel 呼吸动画 -- 通过

连接中的呼吸灯效果实现规范:
- 使用 `QSequentialAnimationGroup` + `QPropertyAnimation`
- 持续时间 1500ms x 2 = 3000ms 一个完整周期
- 缓动曲线 `InOutSine` -- 呼吸感自然
- 无限循环 `setLoopCount(-1)`
- 连接成功/失败时正确停止动画并重置 opacity

### C08: AnimatedProgressBar 流动效果 -- 通过

shimmer 流动效果实现规范:
- QPropertyAnimation 驱动 shimmerOffset [0.0, 1.0]
- 2000ms 循环, Linear 缓动 -- 符合 CLAUDE.md 6.5
- 颜色从 ThemeManager::SemanticColor::Accent 获取
- 支持完成变色动画 setChunkColor() / resetChunkColor()

### C09: 语义色板定义完整 -- 通过

三套主题均定义了完整的语义色板 (在 CSS 注释块中):
- 17 个语义色变量, 涵盖 Bg/Text/Accent/Border/Status/Scrollbar/Term 全系列
- dark_terminal: Catppuccin Mocha 色系
- modern_dark: Tokyo Night 色系
- light: Tailwind CSS Gray 色系
- ThemeManager 通过正则从注释块中提取, 设计合理

---

## 三、问题项

### P01: OTA Widget 部分控件缺少 QSS 定义 (严重度: 高)

**问题**: OtaWidget.cpp 中设置了多个 objectName, 但在三套 QSS 主题中完全没有对应的样式定义:

| C++ objectName | QSS 中是否存在 |
|----------------|---------------|
| `otaFileGroup` | 否 |
| `otaFileLabel` | 否 |
| `otaBrowseBtn` | 否 |
| `otaFileInfo` | 否 |
| `otaConfigGroup` | 否 |
| `otaProtocolCombo` | 否 |
| `otaProgressGroup` | 否 |
| `otaProgressBar` | 否 (QSS 中是 `#otaProgress`, 不匹配) |
| `otaStatusLbl` | 否 |
| `otaSpeedLbl` | 否 |
| `otaEtaLbl` | 否 |
| `otaLogGroup` | 否 |
| `otaHistoryGroup` | 否 |

**影响**: 这些控件将回退到通用 QPushButton/QComboBox/QGroupBox 样式。虽然功能不受影响, 但:
- `otaBrowseBtn` 没有独立的 accent 色, 与普通按钮无视觉区分
- `otaProgressBar` 与 QSS `#otaProgress` 不匹配, 导致 QSS 完全失效, 进度条显示为系统默认样式
- `otaStatusLbl` / `otaSpeedLbl` / `otaEtaLbl` 无法定制颜色
- 各 GroupBox 无法独立调整边距和标题样式

**修复建议**:
1. 将 C++ 中 `m_progressBar->setObjectName("otaProgressBar")` 改为 `setObjectName("otaProgress")` 以匹配现有 QSS, 或反过来将 QSS 的 `#otaProgress` 改为 `#otaProgressBar`
2. 为 `#otaBrowseBtn` 添加独立 QSS (accent 色按钮)
3. 为 `#otaStatusLbl`, `#otaSpeedLbl`, `#otaEtaLbl` 添加 QSS (使用 text-secondary / text-muted 颜色)
4. 各 ota GroupBox 若无需独立样式可保持通用 QGroupBox 样式, 但需确认视觉一致

### P02: FrameVisualEditor 大量控件缺少 QSS 定义 (严重度: 高)

**问题**: FrameVisualEditor.cpp 中创建了大量带有 objectName 的控件, 但在三套 QSS 中完全没有对应样式:

| C++ objectName | 用途 |
|----------------|------|
| `frameHeaderEdit` | 帧头输入框 |
| `frameFooterEdit` | 帧尾输入框 |
| `frameLengthOffsetSpin` | 长度偏移 SpinBox |
| `frameLengthSizeCombo` | 长度大小 Combo |
| `frameLengthEndianCheck` | 字节序 CheckBox |
| `frameLengthAdjustSpin` | 长度调整 SpinBox |
| `frameChecksumTypeCombo` | 校验类型 Combo |
| `frameChecksumOffsetSpin` | 校验偏移 SpinBox |
| `frameChecksumStartSpin` | 校验起始 SpinBox |
| `framePreviewGroup` | 预览分组 |
| `framePreviewLabel` | 预览标签 |
| `frameMoveUpBtn` | 上移按钮 |
| `frameMoveDownBtn` | 下移按钮 |

**影响**: 这些控件全部回退到通用 QLineEdit/QComboBox/QSpinBox/QPushButton 样式。虽然没有视觉错误, 但:
- `frameMoveUpBtn`/`frameMoveDownBtn` 没有与 `frameAddFieldBtn`/`frameRemoveFieldBtn` 统一的按钮风格
- `framePreviewLabel` 无法设置等宽字体或特殊背景色来突出预览区域

**修复建议**: 为 `#frameMoveUpBtn`/`#frameMoveDownBtn` 添加与 `#frameAddFieldBtn` 一致的 QSS 样式。其他控件使用通用样式即可, 但需在三个主题中确认通用样式足够。

### P03: ChartColors.h 中硬编码颜色 (严重度: 中)

**文件**: `src/chart/ChartColors.h`

**问题**: 波形图通道颜色使用硬编码十六进制色值:
```cpp
// darkColors():
QColor("#89b4fa"), QColor("#a6e3a1"), QColor("#f9e2af"), ...

// lightColors():
QColor("#1e66f5"), QColor("#40a02b"), QColor("#df8e1d"), ...
```

这些颜色没有通过 ThemeManager 的语义色系统管理。

**影响**: 如果未来需要统一调整色调或添加新主题, 需要修改 C++ 代码而非 QSS 文件。

**修复建议**: 这个属于可接受的例外 -- 波形图通道颜色是数据可视化的调色板, 不是 UI 语义色。每个通道需要独立的可区分颜色, 不适合映射到语义色。但建议:
1. 在 ChartColors.h 中添加注释说明这是数据可视化调色板, 与 UI 语义色是两套独立体系
2. 考虑将颜色定义移到 JSON 配置文件中, 方便用户自定义通道颜色

### P04: 圆角值不统一, 违反 6.3 标准 (严重度: 中)

**问题**: CLAUDE.md 6.3 明确规定 "圆角统一值: 6px", 但三个主题中实际使用了 6 种不同的圆角值:

| 圆角值 | 使用次数 | 适用控件 |
|--------|---------|---------|
| 4px | 43 次 | 绝大多数按钮、输入框、下拉框 |
| 6px | 8 次 | QGroupBox、QMenu、面板容器、弹出面板、下拉列表 |
| 2px | 3 次 | 搜索栏 CheckBox indicator |
| 3px | 2 次 | 通用 CheckBox indicator |
| 7px | 1 次 | Slider handle |
| 8px | 1 次 | bgSettingsPopup |

**影响**: 4px 是最常用的圆角值, 但标准要求 6px。如果强制改为 6px, 按钮和输入框会显得稍圆, 需要权衡。

**修复建议** (非阻塞, 需产品经理确认):
1. **方案 A**: 将 6.3 标准修改为 "按钮/输入框 4px, 面板/菜单/GroupBox 6px, 小型 indicator 2-3px" -- 更符合实际设计语言
2. **方案 B**: 全部统一为 6px -- 需要确认视觉效果是否过于圆润
3. 建议采用方案 A, 将标准文本与实际实现保持一致

### P05: SerialConfigPanel ComboBox 控件缺少 QSS (严重度: 中)

**问题**: SerialConfigPanel 中的多个 ComboBox 设置了 objectName, 但在三套 QSS 中没有对应的选择器:

| C++ objectName | QSS 中是否存在 |
|----------------|---------------|
| `portCombo` | 否 |
| `baudCombo` | 否 |
| `dataBitsCombo` | 否 |
| `parityCombo` | 否 |
| `stopBitsCombo` | 否 |
| `flowControlCombo` | 否 |

**影响**: 这些控件使用通用 QComboBox 样式。功能正常, 但:
- `baudCombo` 是可编辑的 (setEditable), 内部 QLineEdit 的字体应使用等宽字体, 但通用 QComboBox 样式中没有特殊处理
- 无法为端口选择框设置独立的宽度或颜色

**修复建议**: 为 `#baudCombo` 添加 QSS, 设置内部 QLineEdit 使用等宽字体:
```css
QComboBox#baudCombo QLineEdit {
    font-family: "Consolas", "Courier New", monospace;
}
```
其他 ComboBox 可使用通用样式。

### P06: PanelManager 面板容器 objectName 缺少 QSS (严重度: 低)

**问题**: PanelManager.cpp 中为各面板设置了 objectName:
- `serialConfigPanel`, `dataStatsPanel`, `protocolViewPanel`, `frameEditorPanel`
- `chartWidgetPanel`, `otaWidgetPanel`, `terminalPanel`, `searchBarPanel`

这些 objectName 在三套 QSS 中没有对应样式。

**影响**: 这些是面板容器 QWidget, 如果不需要独立背景色或边框, 可以保持透明/继承父级样式。但如果未来需要为不同面板设置不同的背景, 则需要添加 QSS。

**修复建议**: 当前可接受。如果面板切换动画需要独立的面板背景, 再补充 QSS。

---

## 四、改进建议 (非阻塞优化项)

### S01: ToastWidget 图标字体平台兼容性

ToastWidget.h 使用 `QFont("Segoe UI Emoji", ...)` 绘制图标。Segoe UI Emoji 在 Windows 上可用, 但在 Linux/macOS 上不可用。

建议: 添加回退字体 `QFont("Segoe UI Emoji", kIconSize, QFont::Bold)` -> 检测平台, 或使用 SVG 图标代替 Unicode 字符。

### S02: 搜索栏 CheckBox indicator 尺寸偏小

`QCheckBox#searchBarRegexCheck::indicator` 和 `QCheckBox#searchBarHexCheck::indicator` 的尺寸为 14x14px, 比通用 CheckBox 的 16x16px 小。在触摸屏设备上可能难以点击。建议统一为 16x16px。

### S03: 缺少 QSS 变量引用机制

当前 QSS 文件中的颜色全部是硬编码的十六进制值 (如 `#1e1e2e`, `#89b4fa`), 虽然 CLAUDE.md 6.2 定义了语义色板, 但 QSS 选择器中直接使用色值而非变量引用。这意味着修改主背景色需要在 QSS 文件中查找替换多处。

Qt QSS 不支持 CSS 变量语法, 当前通过注释块 + ThemeManager 正则提取的方案是合理的变通。但如果未来主题复杂度增加, 可考虑:
1. 使用 ThemeManager 在运行时替换 QSS 模板中的占位符
2. 或使用预处理器 (如 Python 脚本) 从变量定义生成最终 QSS

### S04: Slider handle 圆角不一致

`QSlider#bgBlurSlider::handle:horizontal` 使用 `border-radius: 7px` (实际是圆形, 因为 handle 是 14x14px)。这与 6px 标准不同。由于 Slider handle 是圆形控件, 7px 圆角使其看起来是正圆, 这是合理的。建议在 6.3 标准中添加例外说明: "Slider handle 使用 50% 圆角以实现圆形效果"。

### S05: 缺少 focus-visible 状态

QSS 中 QLineEdit:focus 使用边框色变化表示焦点, 但没有明显的外发光效果。CLAUDE.md 6.6 要求焦点态 "2px border-focus 色边框 + 轻微外发光"。当前只有 1px 边框变化 (普通态是 1px, 焦点态也是 1px 但颜色不同)。

建议: 将 QLineEdit:focus 的边框改为 2px, 与 error 状态的 2px 保持一致的视觉权重。或通过 `outline` / `box-shadow` 效果增强焦点可见性 (但 Qt QSS 对这些属性支持有限)。

### S06: QSS 中 bgSettingsPopup 圆角与标准不一致

`QWidget#bgSettingsPopup` 使用 `border-radius: 8px`, 而 CLAUDE.md 6.3 要求统一 6px。作为弹出面板, 8px 是合理的设计选择 (比面板内容控件稍圆, 增加层次感), 但需要更新标准或统一为 6px。

---

## 五、总结

### 统计

| 类别 | 数量 |
|------|------|
| 通过项 | 9 |
| 问题项 | 6 |
| 改进建议 | 6 |

### 问题严重度分布

| 严重度 | 数量 | 问题编号 |
|--------|------|---------|
| 高 | 2 | P01, P02 |
| 中 | 2 | P03, P04 |
| 低 | 1 | P06 |
| 待定 | 1 | P05 |

### 优先修复建议

1. **P01 (紧急)**: 修复 `otaProgressBar` vs `otaProgress` 的 objectName 不匹配问题 -- 这直接导致进度条样式失效
2. **P02 (高)**: 为 FrameVisualEditor 的 MoveUp/MoveDown 按钮添加 QSS -- 这些是高频操作按钮, 需要与 Add/Remove 按钮视觉一致
3. **P04 (中)**: 将 CLAUDE.md 6.3 的圆角标准与实际实现同步 -- 避免未来开发者困惑
4. **P05 (中)**: 为 baudCombo 添加等宽字体 QSS -- 串口工具的专业感细节

### 整体评价

三套主题的同步性很好 (相同的 border-radius 分布, 相同的控件选择器覆盖), 语义色板体系设计完善, ToastWidget/NavIndicatorWidget/AnimatedProgressBar 的动画实现规范且符合 CLAUDE.md 6.5 标准。主要问题集中在 OTA Widget 和 FrameVisualEditor 的 objectName 与 QSS 不匹配, 属于遗漏而非设计缺陷。修复后观感质量将显著提升。
