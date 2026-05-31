# PRD-014: QSS主题颜色迁移 -- 消除C++硬编码颜色

## 背景

CLAUDE.md 6.8 UI执行强制规则第1条明确规定:

> **禁止在C++代码中硬编码颜色值到setStyleSheet()** -- 所有颜色必须从主题QSS文件中获取。

当前代码库中存在 **36处 setStyleSheet() 调用** 和 **59处硬编码十六进制色值**，分布在10个源文件中。这些硬编码颜色违反了项目约束，同时导致以下具体问题:

1. **主题切换失效** -- `TerminalSearchBar` 在 `SearchBarColors` 命名空间中硬编码7个色值常量。切换到 light 主题后，搜索栏仍显示 dark_terminal 配色。
2. **状态色无法适配** -- `MainWindow::updateConnectionStatus` 用4个硬编码色值表示连接状态，在 light 主题下暗色调颜色可读性差。
3. **面板样式重复** -- `DataStatistics` 中3个QFrame用相同的 `#1e1e2e`/`#313244` 硬编码3次，与QSS已有语义色重复。
4. **组件样式分散** -- `OtaWidget`、`SerialConfigPanel`、`FrameVisualEditor` 中的按钮、进度条、文本框样式全部内联在C++中，无法通过QSS统一调整。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | 为所有需要QSS选择器的widget设置唯一objectName | P0 | 全部受影响文件 |
| R2 | MainWindow: 连接状态标签颜色迁移到QSS动态属性 | P0 | core/MainWindow |
| R3 | SerialConfigPanel: 连接/断开按钮状态色迁移到QSS动态属性 | P0 | serial/SerialConfigPanel |
| R4 | DataStatistics: RX/TX/Time面板样式迁移到QSS objectName选择器 | P0 | serial/DataStatistics |
| R5 | TerminalSearchBar: 消除SearchBarColors命名空间，迁移到QSS | P0 | terminal/TerminalSearchBar |
| R6 | OtaWidget: 全部6处setStyleSheet迁移到QSS | P0 | ota/OtaWidget |
| R7 | FrameVisualEditor: 应用按钮样式迁移到QSS | P0 | protocol/FrameVisualEditor |
| R8 | ProtocolView: 错误项前景色迁移到动态属性或委托 | P1 | protocol/ProtocolView |
| R9 | OtaHistoryModel: 成功/失败色迁移到动态属性或委托 | P1 | ota/OtaHistoryModel |
| R10 | TerminalWidget: QPainter颜色保留在C++中，从ThemeManager获取 | P1 | terminal/TerminalWidget |
| R11 | ChartWidget: 通道默认颜色表保留在C++中（数据可视化配色） | P2 | chart/ChartWidget |
| R12 | 三个QSS主题文件同步新增所有选择器 | P0 | resources/themes/ |

## 文件级影响分析

### 需要修改的C++文件

| 文件 | 色值数 | setStyleSheet数 | 迁移策略 |
|------|-------|----------------|---------|
| `src/terminal/TerminalSearchBar.cpp` | 7 | 10 | objectName + 动态属性替代SearchBarColors |
| `src/ota/OtaWidget.cpp` | 18 | 6 | objectName + QSS选择器 |
| `src/serial/DataStatistics.cpp` | 11 | 8 | objectName + QSS选择器 |
| `src/core/MainWindow.cpp` | 4 | 6 | 动态属性 `connectionState` 驱动QSS |
| `src/serial/SerialConfigPanel.cpp` | 6 | 3 | 动态属性 `connected` 驱动QSS |
| `src/protocol/FrameVisualEditor.cpp` | 2 | 1 | objectName + QSS选择器 |
| `src/protocol/ProtocolView.cpp` | 1 | 0 | 动态属性 `parseError` |
| `src/ota/OtaHistoryModel.cpp` | 1 | 0 | 动态属性 `otaResult` 或QStyledItemDelegate |

### 不修改的C++文件（允许例外）

| 文件 | 原因 |
|------|------|
| `src/terminal/TerminalWidget.cpp` | QPainter自绘引擎，CLAUDE.md 6.8 明确允许 |
| `src/chart/ChartWidget.cpp` | 通道颜色表用于QLineSeries，属数据可视化配色 |
| `src/core/ThemeManager.cpp` | `qApp->setStyleSheet(qss)` 是正确的主题加载用法 |

## 迁移方案

### 方案一: objectName + QSS选择器（静态样式）

适用于初始化后样式不变的控件。C++中仅设置 `setObjectName("uniqueName")`，删除 `setStyleSheet()` 调用，样式完全由QSS文件定义。

```cpp
// 修改前 (DataStatistics.cpp):
rxFrame->setStyleSheet("QFrame { background-color: #1e1e2e; border: 1px solid #313244; ... }");

// 修改后:
rxFrame->setObjectName("rxStatsFrame");  // 样式由QSS文件控制
```

```css
/* dark_terminal.qss */    QFrame#rxStatsFrame { background-color: #1e1e2e; border: 1px solid #313244; ... }
/* modern_dark.qss */      QFrame#rxStatsFrame { background-color: #1a1b26; border: 1px solid #292e42; ... }
/* light.qss */            QFrame#rxStatsFrame { background-color: #f3f4f6; border: 1px solid #e5e7eb; ... }
```

### 方案二: 动态属性 + QSS属性选择器（运行时状态变化）

适用于运行时样式需要动态切换的场景。通过 `setProperty()` 设置状态，QSS使用 `[prop="value"]` 选择器匹配，状态变化后调用 `unpolish` + `polish` 刷新。

```cpp
// MainWindow.cpp -- 连接状态标签
m_connStatusLbl->setObjectName("connStatusLabel");
m_connStatusLbl->setProperty("connectionState", "disconnected");

// 状态更新时:
m_connStatusLbl->setProperty("connectionState", "connected");
m_connStatusLbl->style()->unpolish(m_connStatusLbl);
m_connStatusLbl->style()->polish(m_connStatusLbl);
```

```css
/* dark_terminal.qss: */
QLabel#connStatusLabel[connectionState="connected"]    { color: #a6e3a1; }
QLabel#connStatusLabel[connectionState="disconnected"] { color: #f38ba8; }
QLabel#connStatusLabel[connectionState="connecting"]   { color: #f9e2af; }
QLabel#connStatusLabel[connectionState="error"]        { color: #f38ba8; }
```

同样的模式应用于:
- **SerialConfigPanel** -- `QPushButton#connectBtn[connected="false"]` 绿色 / `[connected="true"]` 红色
- **TerminalSearchBar** -- `QLineEdit#searchBarInput[inputState="normal"]` 默认边框 / `[inputState="error"]` 红色边框
- **MainWindow sendInput** -- `QLineEdit#sendInput[sendError="true"]` 红色边框替代 `"border: 1px solid red"`

### 方案三: QStyledItemDelegate（Model/View行级着色）

适用于 `QAbstractItemModel::data(Qt::ForegroundRole)` 返回硬编码色的场景（ProtocolView、OtaHistoryModel）。使用自定义委托从ThemeManager获取语义色，或使用动态属性配合QSS `::item` 子选择器。

## objectName注册表

| objectName | 控件类型 | 所在类 |
|-----------|---------|--------|
| `connStatusLabel` | QLabel | MainWindow |
| `sendInput` | QLineEdit | MainWindow |
| `connectBtn` | QPushButton | SerialConfigPanel |
| `rxStatsFrame` | QFrame | DataStatistics |
| `rxTotalLabel`, `rxRateLabel` | QLabel | DataStatistics |
| `txStatsFrame` | QFrame | DataStatistics |
| `txTotalLabel`, `txRateLabel` | QLabel | DataStatistics |
| `elapsedStatsFrame` | QFrame | DataStatistics |
| `elapsedLabel` | QLabel | DataStatistics |
| `terminalSearchBar` | QWidget | TerminalSearchBar |
| `searchBarInput` | QLineEdit | TerminalSearchBar |
| `searchResultLabel` | QLabel | TerminalSearchBar |
| `regexCheck`, `hexCheck` | QCheckBox | TerminalSearchBar |
| `searchCloseBtn` | QPushButton | TerminalSearchBar |
| `otaStartBtn`, `otaCancelBtn` | QPushButton | OtaWidget |
| `otaProgressBar` | QProgressBar | OtaWidget |
| `otaLogView` | QTextEdit | OtaWidget |
| `otaHistoryView` | QTreeView | OtaWidget |
| `otaClearHistoryBtn` | QPushButton | OtaWidget |
| `frameApplyBtn` | QPushButton | FrameVisualEditor |

## 动态属性清单

| 属性名 | 值域 | 控件 | 说明 |
|--------|------|------|------|
| `connectionState` | connected / disconnected / connecting / error | `connStatusLabel` | 连接状态指示 |
| `connected` | true / false | `connectBtn` | 连接按钮状态切换 |
| `inputState` | normal / error | `searchBarInput` | 搜索输入验证状态 |
| `sendError` | true / false | `sendInput` | 发送框HEX错误状态 |
| `otaResult` | success / failure | OtaHistoryModel行项 | OTA历史结果着色 |
| `parseError` | true / false | ProtocolView行项 | 协议解析错误着色 |

## 依赖的公共组件

| 组件 | 文件 | 复用方式 |
|------|------|---------|
| `ThemeManager` | `core/ThemeManager.h/cpp` | TerminalWidget/ChartWidget通过它获取QPainter绘制颜色 |
| `Constants` | `core/Constants.h` | ConnectionState枚举用于动态属性值映射 |

## 设计模式

QSS动态属性选择器是策略模式的声明式实现: 控件视觉表现由属性值决定，属性变化时QSS引擎自动切换样式策略。C++代码只修改属性值，不感知具体样式。

## 影响范围

| 文件 | 操作 | 说明 |
|------|------|------|
| `src/core/MainWindow.cpp` | 修改 | connStatusLabel/sendInput改为动态属性 |
| `src/serial/SerialConfigPanel.cpp` | 修改 | connectBtn改为动态属性驱动 |
| `src/serial/DataStatistics.cpp` | 修改 | 消除8处setStyleSheet，改用objectName |
| `src/terminal/TerminalSearchBar.cpp` | 修改 | 消除SearchBarColors和10处setStyleSheet |
| `src/ota/OtaWidget.cpp` | 修改 | 消除6处setStyleSheet |
| `src/protocol/FrameVisualEditor.cpp` | 修改 | 消除1处setStyleSheet |
| `src/protocol/ProtocolView.cpp` | 修改 | 错误项前景色改为委托 |
| `src/ota/OtaHistoryModel.cpp` | 修改 | 结果着色改为委托 |
| `resources/themes/dark_terminal.qss` | 修改 | 新增全部选择器 |
| `resources/themes/modern_dark.qss` | 修改 | 同上，modern_dark色板 |
| `resources/themes/light.qss` | 修改 | 同上，light色板 |

## 验收标准

### 功能验收

| # | 验收条件 | 验证方法 |
|---|---------|---------|
| AC1 | `grep -rn 'setStyleSheet' src/` 仅返回 ThemeManager.cpp 的 `qApp->setStyleSheet()` 和 MainWindow.cpp 中 sendInput 空字符串重置 | grep验证 |
| AC2 | `grep -rn '#[0-9a-fA-F]\{6\}' src/` 仅返回 TerminalWidget.cpp（QPainter）、ChartWidget.cpp（通道色表） | grep验证 |
| AC3 | 三个主题下启动应用，所有控件颜色正确 | 手动切换主题逐一检查 |
| AC4 | 连接/断开串口，状态标签颜色正确切换 | 手动测试各状态 |
| AC5 | 连接按钮连接态/断开态颜色和文字正确 | 手动测试 |
| AC6 | 搜索栏正常态和HEX非法输入态边框颜色正确切换 | 输入非法HEX验证 |
| AC7 | OTA面板、DataStatistics面板在三个主题下样式正确 | 切换主题检查 |
| AC8 | 主题切换后无残留的旧主题颜色 | 切换主题后检查所有面板 |
| AC9 | 消除SearchBarColors命名空间后搜索栏功能正常 | 编译 + Ctrl+F测试 |
| AC10 | sendInput HEX错误验证的红色边框在三个主题下均正确 | 切换主题后输入非法HEX |

### 代码质量验收

| # | 验收条件 |
|---|---------|
| AC11 | 所有新增objectName在三个QSS文件中均有对应选择器 |
| AC12 | 动态属性变化后正确调用 `unpolish` + `polish` 刷新样式 |
| AC13 | 不引入新的硬编码色值到C++ setStyleSheet |
| AC14 | objectName遵循camelCase，与现有 `quickCommandBar`/`sendButton` 风格一致 |
| AC15 | 零编译错误，零编译警告 |
| AC16 | EmbedDebug.bat启动验证通过 |
