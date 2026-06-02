# PRD-035: Bug修复冲刺 -- 全模块审查与新特性提案

## 背景

commit #34 完成了 FrameParser 状态机拆分、DataExporter 导出增强（协议帧CSV / HEX dump / 时间过滤）和 UI 一致性收尾。当前评分 36 分。36 是 3 的倍数，按 CLAUDE.md 3.4.5 节规定，本次迭代必须执行全面的 Bug 审查和修复冲刺。

本次审查范围覆盖全部七大模块，采用逐文件代码审计方式排查问题。同时按照 CLAUDE.md 3.4.4 节规定，每 3 次迭代需进行新特性提案评估，本次一并输出。

**审查基准**: commit #34, score 36。

---

## 一、全功能Bug审查报告

### 1.1 终端功能模块

#### BUG-T01: 方向前缀着色使用了 darker(130) 导致 TX 前缀过暗 [P2]

**文件**: `src/terminal/TerminalWidget.cpp` 行 273

**现象**: 方向前缀 `[TX:]` 使用 `dataColor.darker(130)` 渲染，而非 PRD-026 R4 规定的独立配色（TX 绿色 #a6e3a1，RX 蓝色 #89b4fa）。当前 m_txColor 和 m_rxColor 本身已经是方向颜色，`darker(130)` 会让前缀变得比数据文字更暗，视觉上前缀反而不醒目。

**根因**: PRD-026 R4 设计了 `[TX:]` 使用绿色 #a6e3a1，`[RX:]` 使用蓝色 #89b4fa 的独立配色方案，但实际实现中使用 `dataColor.darker(130)` 只是将方向颜色变暗 30%，与设计意图不符。

**修复方案**: 使用硬编码的语义色替代 `darker(130)`:
- TX 前缀: `QColor(0xa6, 0xe3, 0xa1)` (Catppuccin green)
- RX 前缀: `QColor(0x89, 0xb4, 0xfa)` (Catppuccin blue)
- 后续可迁移到 ThemeManager 提供 `directionPrefixTxColor()` / `directionPrefixRxColor()`

**影响**: 仅视觉表现，不影响功能。

---

#### BUG-T02: 搜索栏 Ctrl+F 激活后搜索栏可能被面板切换遮挡 [P1]

**文件**: `src/core/MainWindow.cpp` 行 208-209

**现象**: Ctrl+F 快捷键连接到 `m_panelManager->searchBar()->activate()`，但如果当前活动面板不是终端面板（用户可能正在查看串口配置或 OTA 面板），搜索栏的激活没有意义，用户看不到搜索栏。同时 `handleConnectionState` 在连接成功后会自动切换到终端面板（行 256），但这种自动切换不应在搜索激活时发生。

**根因**: 搜索栏的 activate 调用没有检查当前是否在终端面板上，也没有自动切换到终端面板的逻辑。

**修复方案**: 在 Ctrl+F 激活搜索栏时，先检查当前面板是否为终端容器，若不是则先切换到终端面板再激活搜索栏:
```cpp
connect(searchShortcut, &QShortcut::activated, this, [this]() {
    // 确保终端面板可见后再激活搜索栏
    m_navController->switchToPanel(m_panelManager->terminal());
    m_panelManager->searchBar()->activate();
});
```

**影响**: 搜索功能在非终端面板时无法使用。

---

#### BUG-T03: 终端清屏后搜索状态未清除 [P1]

**文件**: `src/terminal/TerminalWidget.cpp` `clear()` 方法 (行 148-155)

**现象**: `TerminalWidget::clear()` 重置了缓存、方向过滤和选区，但没有清除搜索状态。清屏后如果之前有活跃搜索，搜索匹配列表仍然持有旧行号引用，`paintEvent` 中 `refreshSearchAfterCacheUpdate()` 可能访问已被清除的缓存行导致搜索匹配指向无效位置。

**根因**: `clear()` 缺少 `m_searchManager->clearSearchHighlight()` 调用。

**修复方案**: 在 `clear()` 方法中增加搜索管理器重置:
```cpp
void TerminalWidget::clear()
{
    m_cachedLines.clear();
    m_cachedLineCount = 0;
    m_directionFilter->reset();
    m_selectionManager->reset();
    m_searchManager->clearSearchHighlight();  // 新增: 清屏时同步清除搜索状态
    update();
}
```

**影响**: 清屏后残留搜索高亮可能指向不存在的行，导致视觉异常。

---

#### BUG-T04: 纯文本搜索大小写敏感，但 UX 未提供切换选项 [P2]

**文件**: `src/terminal/TerminalSearchManager.cpp` 行 88, 128

**现象**: 纯文本搜索使用 `text.indexOf(pattern, pos)` 默认大小写敏感。串口协议中 AT 指令通常区分大小写（如 `AT` 与 `at`），因此默认大小写敏感是合理的。但 TerminalSearchBar 没有"大小写敏感"切换按钮，用户无法选择忽略大小写搜索。

**修复方案**: 后续迭代在 TerminalSearchBar 增加大小写切换按钮，本迭代仅记录此体验缺口。当前行为（区分大小写）对嵌入式调试是合理的默认值。

**影响**: 用户体验受限，但不影响核心功能。

---

#### BUG-T05: 十进制(Decimal)显示模式下数据不含 ASCII 文本参考 [P2]

**文件**: `src/terminal/TerminalWidget.cpp` `formatToCache()` (行 472-476)

**现象**: Decimal 模式仅显示 `25 65 108 108 111` 这样的十进制数值，没有对应的 ASCII 文本参考行。用户需要手动心算十进制到 ASCII 的映射，降低了调试效率。

**修复方案**: 非紧急，记录为体验优化候选。可在后续迭代中为 Decimal 模式添加类似 Mixed 模式的双列显示（十进制 | ASCII）。

**影响**: 体验不佳，但 Decimal 模式本身使用频率较低。

---

### 1.2 串口功能模块

#### BUG-S01: 串口连接成功后连接按钮 "连接中..." 状态可能卡住 [P1]

**文件**: `src/serial/SerialConfigPanel.cpp` 行 155-171

**现象**: 点击连接按钮时设置 `m_connecting = true` 并禁用按钮（行 164-165）。但如果连接流程中 `ConnectionController::connectSerial()` 同步完成（即 `open()` 立即返回成功），`setConnected(true)` 在 `handleConnectionState` 中被调用（通过信号），此时 `m_connecting` 被清除。然而如果 `connectRequested` 信号的处理链条中某个环节抛出异常或信号丢失，按钮会永久停留在"连接中..."状态。

**根因**: 当前的 `m_connecting` 状态恢复完全依赖 `setConnected()` 被调用。虽然正常流程下不会卡住，但如果 `connectionStateChanged` 信号因为某种原因未发射（比如连接对象在信号发射前被意外销毁），就会出问题。

**修复方案**: 添加超时保护机制。在设置 `m_connecting = true` 的同时启动一个 1 秒单次定时器，如果超时后 `m_connecting` 仍为 true，强制恢复按钮状态:
```cpp
// 在 SerialConfigPanel 中新增 QTimer* m_connectTimeoutTimer
// 连接按钮点击时:
m_connectTimeoutTimer->start(1000);
// setConnected() 中:
m_connectTimeoutTimer->stop();
// 超时处理:
m_connecting = false;
m_connectBtn->setEnabled(true);
updateConnectButtonState();
```

**影响**: 极端情况下按钮状态卡住，用户无法重试连接，必须重启应用。

---

#### BUG-S02: 端口热插拔后 SerialConfigPanel 端口列表未自动刷新 [P1]

**文件**: `src/serial/PortWatcher.cpp`, `src/core/ConnectionController.cpp`

**现象**: `PortWatcher` 每 1 秒轮询端口变化，发射 `portAdded`/`portRemoved` 信号。`ConnectionController` 连接了这些信号并处理连接断开逻辑（行 437-479），但 `portAdded` 信号仅被转发（`emit portAdded(portName)` 行 479），没有连接到 `SerialConfigPanel::refreshPorts()`。用户插入新的 USB 转串口设备后，必须手动点击"刷新"按钮才能看到新端口。

**根因**: `MainWindowSignalConnect.cpp` 中没有将 `ConnectionController::portAdded` 信号连接到 `SerialConfigPanel::refreshSlots()`。

**修复方案**: 在 `connectSignals()` 中添加:
```cpp
// 端口热插拔自动刷新端口列表
connect(m_connController, &ConnectionController::portAdded,
        m_panelManager->serialConfig(), &SerialConfigPanel::refreshPorts);
connect(m_connController, &ConnectionController::portAdded,
        this, [this](const QString& portName) {
    statusBar()->showMessage(tr("检测到新端口: %1").arg(portName), 3000);
});
```

注意: 需要在 `ConnectionController.h` 中确认 `portAdded` 信号是否已声明。当前代码行 479 中有 `emit portAdded(portName)` 调用，说明信号已存在。

**影响**: 用户体验不佳。大多数现代串口工具（如 PuTTY、TeraTerm）在检测到新设备时会自动刷新端口列表。

---

#### BUG-S03: 波特率自定义值输入后可能被 QIntValidator 截断 [P2]

**文件**: `src/serial/SerialConfigPanel.cpp` 行 86

**现象**: 波特率输入框安装了 `QIntValidator(300, 10000000)`，但 QComboBox 的 `setEditable(true)` 配合 QIntValidator 时存在已知问题: 用户输入中间状态（如输入 "1" 准备输入 "1500000"）时，QIntValidator 的 Intermediate 状态可能阻止输入，或者在某些 Qt 版本下允许非法中间值通过。

**修复方案**: 将 QIntValidator 的下限设为 0（允许中间输入状态），在连接时验证实际值:
```cpp
m_baudCombo->lineEdit()->setValidator(new QIntValidator(0, 10000000, this));
// connectSerial 中已有 currentBaudRate() -> toInt()，值为 0 时会被 QSerialPort 拒绝并报错
```

**影响**: 极少数自定义波特率场景下输入体验不佳。

---

### 1.3 发送功能模块

#### BUG-F01: 定时发送器 UI 入口缺失 -- TimedSender 无法从界面配置 [P1]

**文件**: `src/serial/TimedSender.h/cpp`, `src/core/SendController.h`

**现象**: `TimedSender` 类实现了完整的定时发送功能（间隔设置、启停控制、数据发送），`SendController` 持有 `m_timedSender` 实例并暴露了 `timedSender()` 方法。但发送栏 UI（`createSendBar()`）中没有定时发送的配置入口（间隔输入框、启停按钮）。定时发送功能已经完全实现但用户无法使用。

**根因**: `createSendBar()` 创建了模式切换、换行符选择、输入框和发送按钮，但没有添加定时发送的 UI 控件。

**修复方案**: 在发送栏增加定时发送按钮和间隔配置:
- 新增 `m_timedSendBtn`（checkable 按钮，"定时发送"）
- 新增 `m_intervalSpin`（QSpinBox，单位 ms，范围 10~60000，默认 1000）
- 勾选定时发送后，按设定间隔周期性发送输入框内容
- 取消勾选或清空输入框时停止定时发送

预估工作量: 50 行新增代码。

**影响**: 已实现的功能无法被用户访问。

---

#### BUG-F02: 发送历史自动补全在 HEX 模式下显示的是原始 HEX 文本 [P2]

**文件**: `src/core/SendController.cpp` 行 71-75, 239

**现象**: 发送历史通过 `SendHistory::addEntry(text, isHex)` 记录原始输入文本。自动补全的 `m_sendCompleterModel` 使用 `m_sendHistory->recentTexts()` 获取历史。当用户在文本模式下输入时，自动补全会同时显示文本模式和 HEX 模式的历史记录，HEX 字符串（如 "AA 55 01 00 FE"）会出现在文本模式的补全列表中。

**修复方案**: 可以在两种模式间过滤补全列表:
- 文本模式下只显示文本模式的历史
- HEX 模式下只显示 HEX 模式的历史

或在 `recentTexts()` 方法中添加可选的 `isHex` 过滤参数。

**影响**: 体验不佳，但不影响发送功能正确性。

---

#### BUG-F03: 快捷指令编辑后不自动持久化 [P1]

**文件**: `src/serial/QuickCommandBar.cpp` 行 229-231

**现象**: `onEditRequested()` 中用户确认编辑后调用 `setCommands(newCmds)` 更新内存中的指令列表并重建按钮，但没有调用 `saveCommands()` 将编辑结果持久化到 QSettings。应用重启后编辑的指令会丢失。

**根因**: `setCommands()` 只更新内存状态，`saveCommands()` 需要显式调用。`onEditRequested()` 的编辑确认分支缺少 `saveCommands()` 调用。

**修复方案**: 在 `onEditRequested()` 确认分支中增加 `saveCommands()`:
```cpp
if (dlg.exec() == QDialog::Accepted) {
    // ... 现有的读取表格逻辑 ...
    setCommands(newCmds);
    saveCommands();  // 新增: 编辑确认后立即持久化
}
```

同样检查应用启动时是否有 `loadCommands()` 调用。当前 QuickCommandBar 构造函数中没有调用 `loadCommands()`，需要在 PanelManager 创建面板后或 MainWindow 初始化时调用。

**影响**: 用户编辑的快捷指令在重启后丢失。

---

### 1.4 协议解析模块

#### BUG-P01: FrameParser 缓冲区溢出时当前字节可能被丢弃 [P2]

**文件**: `src/protocol/FrameParser.cpp` processByte 相关逻辑

**现象**: 当缓冲区达到最大帧长度限制时，`failFrame()` 被调用报告错误并 reset 状态机。但 `processByte()` 注释中有"不 return: 当前字节可能是新帧起始"的逻辑。如果在 reset 后当前字节确实是新帧头，它会被正确处理。但如果 `failFrame` 和 `reset` 的实现中清空了缓冲区，当前字节需要被重新送入状态机才能参与帧头匹配。

**根因**: 需要确认 `failFrame()` -> `reset()` 后，`processByte()` 是否会在同一个调用中继续处理当前字节（即 `switch` 分支中的 `Idle`/`HeaderMatching` case 是否会被执行）。如果 `reset()` 将状态设为 `Idle`，而 `processByte()` 在 `failFrame()` 返回后继续执行 switch，则当前字节会被 Idle 分支处理并进入 HeaderMatching，逻辑正确。需要确认 reset 的实现是否如此。

**修复方案**: 如果确认当前字节被丢弃，在 `failFrame()` 后需要将当前字节重新传给 `handleHeaderMatching(byte)`。

**影响**: 极端边界条件，仅在最长帧溢出且下一帧紧跟的场景下出现。

---

#### BUG-P02: JustFloat/FireWater 协议模式切换后 ChartWidget 通道未自动重建 [P1]

**文件**: `src/protocol/ProtocolBridgeManager.cpp`, `src/chart/ChartWidget.cpp`

**现象**: `ProtocolBridgeManager::setProtocolMode()` 切换协议模式时发射 `protocolModeChanged` 信号，但在 `MainWindowSignalConnect.cpp` 中没有找到将此信号连接到 ChartWidget 通道重建的逻辑。FrameParser 模式下通过 `FrameVisualEditor::definitionChanged` 触发 `ChartWidget::configureFromFrameDefinition()`，但切换到 JustFloat 或 FireWater 模式时，ChartWidget 的通道配置不会被自动更新。

**根因**: 协议模式切换时没有对应的 ChartWidget 通道重新配置逻辑。JustFloat 模式需要知道通道数才能正确显示波形，FireWater 模式需要知道分隔符和字段名。

**修复方案**: 在 `connectSignals()` 中添加:
```cpp
connect(m_protocolBridgeMgr, &ProtocolBridgeManager::protocolModeChanged,
        this, [this](ProtocolBridgeManager::ChartProtocolMode mode) {
    if (mode == ProtocolBridgeManager::ChartProtocolMode::JustFloat) {
        // 从 JustFloatBridge 获取通道数，生成默认通道配置
        int chCount = m_protocolBridgeMgr->justFloatBridge()->channelCount();
        // ... 生成 ChannelConfigSet 并设置到 ChartModel
    }
    // FireWater 类似处理
});
```

**影响**: VOFA+ 协议模式下波形图无法正确显示数据。

---

### 1.5 波形图模块

#### BUG-C01: ChartWidget 图表区域背景未从 ThemeManager 获取配色 [P1]

**文件**: `src/chart/ChartWidget.cpp` setupUI() (行 62-81)

**现象**: PRD-034 R7 要求在 setupUI() 中从 ThemeManager 获取语义色并设置到 QChart 的背景、图例、坐标轴和网格线。当前代码中 QChart 使用默认配色，没有调用 `ThemeManager::instance().color()` 设置图表元素颜色。暗色主题下图表背景可能为白色（QChart 默认），与整体暗色主题不协调。

**根因**: PRD-034 R7 的 ThemeManager 配色代码可能未在本迭代中实现，或被遗漏。

**修复方案**: 在 `setupUI()` 末尾添加 ThemeManager 配色代码:
```cpp
auto& theme = ThemeManager::instance();
m_chart->setBackgroundBrush(QBrush(QColor(theme.color(ThemeManager::SemanticColor::BgPrimary))));
m_chart->legend()->setLabelColor(QColor(theme.color(ThemeManager::SemanticColor::TextSecondary)));
m_xAxis->setLabelsColor(QColor(theme.color(ThemeManager::SemanticColor::TextMuted)));
m_yAxis->setLabelsColor(QColor(theme.color(ThemeManager::SemanticColor::TextMuted)));
// 网格线和标题同理
```
并在 `ThemeManager::themeChanged` 信号中刷新这些颜色。

**影响**: 暗色主题下图表区域显示为白色，视觉上严重不协调。

---

#### BUG-C02: ChartWidget 首次显示时可能仍出现空白渲染 [P2]

**文件**: `src/chart/ChartWidget.cpp`

**现象**: PRD-026 R1 提到 ChartWidget 首次显示时空白的问题，并提出 showEvent + setMinimumSize 双保险修复方案。当前代码中没有看到 `showEvent` 重写或 `setMinimumSize` 设置。setupUI() 中 `m_chartView` 创建后直接添加到 layout（行 80-82），没有 `setMinimumSize(200, 150)` 保护。

**根因**: PRD-026 R1 的修复可能未实际应用到代码中。

**修复方案**: 添加 PRD-026 R1 提出的双保险方案:
1. `m_chartView->setMinimumSize(200, 150);`
2. 重写 `showEvent` 在首次显示时强制刷新轴范围

**影响**: 首次切换到波形图面板时可能看到空白区域，需要点击 Clear 后才正常。

---

#### BUG-C03: ChartWidget 暂停/清除按钮无 QSS 样式 [P1]

**文件**: `resources/themes/*.qss`

**现象**: PRD-034 R7 要求在三个主题 QSS 文件中添加 ChartWidget 控件样式（暂停/清除按钮、窗口大小下拉框、状态标签）。需要确认这些样式是否已实际添加。如果未添加，暗色主题下这些按钮会显示为系统默认样式（白色/灰色），与整体暗色风格不协调。

**根因**: PRD-034 的 QSS 更新可能未被包含在 commit 中。

**修复方案**: 在三个主题 QSS 文件中添加 PRD-034 R7.2 定义的 ChartWidget 样式规则。

**影响**: 波形图控件在暗色主题下视觉不协调。

---

### 1.6 OTA 模块

#### BUG-O01: OTA 传输启动时未检查连接状态 [P0]

**文件**: `src/ota/OtaWidget.cpp` 行 172-197

**现象**: `onStartTransfer()` 检查了文件路径是否为空，然后直接调用 `m_manager->startTransfer(filePath, protocol)`。没有检查当前是否有活跃连接。如果用户在未连接状态下点击"开始传输"，`OtaManager` 会尝试从空连接写入数据，可能导致崩溃或无意义的错误消息。

**根因**: `onStartTransfer()` 缺少连接状态前置检查。

**修复方案**: 在 `onStartTransfer()` 开头添加连接检查:
```cpp
void OtaWidget::onStartTransfer()
{
    // 前置检查: 是否有活跃连接
    if (!m_manager->connection()) {
        appendLog(tr("错误: 未建立连接，请先连接串口"));
        return;
    }

    QString filePath = m_filePathEdit->text().trimmed();
    if (filePath.isEmpty()) {
        appendLog(tr("错误: 未选择固件文件"));
        return;
    }
    // ... 现有逻辑 ...
}
```

需要确认 `OtaManager` 是否暴露了 `connection()` 方法。如果没有，需要添加或在 OtaWidget 中缓存连接状态。

**影响**: 未连接状态下点击传输可能导致崩溃。

---

#### BUG-O02: OTA 传输取消后进度条状态未完全重置 [P2]

**文件**: `src/ota/OtaWidget.cpp` 行 199-204

**现象**: `onCancelTransfer()` 调用 `m_manager->cancelTransfer()` 和 `setTransferring(false)`，但没有重置进度条值（`m_progressBar->setValue(0)` 在 `setTransferring(true)` 时才执行，见行 291）。取消后进度条可能停留在已传输的百分比位置，给用户造成困惑。

**根因**: `setTransferring(false)` 路径中没有重置进度条。

**修复方案**: 在 `onCancelTransfer()` 中添加进度条重置:
```cpp
void OtaWidget::onCancelTransfer()
{
    m_manager->cancelTransfer();
    appendLog(tr("用户已取消传输"));
    m_progressBar->setValue(0);      // 新增: 取消后重置进度条
    m_statusLbl->setText(tr("已取消"));
    setTransferring(false);
}
```

**影响**: 视觉状态不准确，不影响功能。

---

#### BUG-O03: OTA 文件选择组和配置组缺少 objectName [P2]

**文件**: `src/ota/OtaWidget.cpp` setupUI()

**现象**: `fileGroup`（QGroupBox "固件文件"）和 `configGroup`（QGroupBox "传输设置"）没有设置 objectName。同样，`m_filePathEdit`、`m_browseBtn`、`m_protocolCombo` 缺少 objectName。QSS 无法精确选择这些控件，导致暗色主题下 QGroupBox 标题和输入框可能显示为系统默认样式。

**修复方案**: 为所有 OTA 面板子控件添加 objectName:
```cpp
fileGroup->setObjectName("otaFileGroup");
m_filePathEdit->setObjectName("otaFilePath");
m_browseBtn->setObjectName("otaBrowseBtn");
configGroup->setObjectName("otaConfigGroup");
m_protocolCombo->setObjectName("otaProtocolCombo");
```

并在三个主题 QSS 文件中添加对应的样式规则。

**影响**: 暗色主题下 OTA 面板视觉不协调。

---

### 1.7 UI/UX 模块

#### BUG-U01: MainWindow.cpp 行数已降至 326 行，接近目标但仍有优化空间 [P2-INFO]

**文件**: `src/core/MainWindow.cpp`

**现状**: MainWindow.cpp 当前 326 行，MainWindowSignalConnect.cpp 182 行，合计 508 行。MainWindow.h 197 行。MainWindow.cpp 已大幅精简（从最初的 1092 行降至 326 行），接近 500 行上限目标。当前 MainWindow.cpp 只包含 setupUI/setupStatusBar/handleConnectionState/closeEvent，职责清晰。

**结论**: 无需进一步拆分，当前状态合格。

---

#### BUG-U02: OtaWidget.cpp 295 行已接近 500 行上限的 60%，无需拆分 [P2-INFO]

**文件**: `src/ota/OtaWidget.cpp`

**现状**: 295 行，职责清晰（UI 构建 + 传输控制 + 进度更新 + 日志）。在合理范围内。

---

#### BUG-U03: ConnectionController.cpp 525 行超出 500 行文件上限 [P0]

**文件**: `src/core/ConnectionController.cpp`

**现象**: 当前 525 行，超出 CLAUDE.md 4.6 节规定的 500 行 .cpp 文件上限。

**根因**: ConnectionController 包含完整的串口/网络连接生命周期管理、自动重连、热插拔处理、超时保护、下游清理等逻辑。虽然每个方法职责清晰，但总体代码量较大。

**修复方案**: 将热插拔相关逻辑（`onPortRemoved`/`onPortAdded`）提取到独立的 `HotPlugHandler` 类中，将自动重连逻辑（`onAutoReconnect`/`enableAutoReconnect`）提取到 `ReconnectManager` 类中:

```
ConnectionController.cpp (~350 行): 核心连接管理
HotPlugHandler.cpp (~80 行): 端口热插拔检测和自动断开
ReconnectManager.cpp (~70 行): 自动重连逻辑
```

**影响**: 违反 CLAUDE.md 文件体积约束铁律。

---

#### BUG-U04: SerialConfigPanel.cpp 396 行，在安全范围内 [P2-INFO]

**现状**: 396 行，接近但未超出 500 行上限。当前职责清晰（UI 构建 + 端口管理 + 状态控制），无需拆分。

---

### 1.8 Bug 审查汇总表

| ID | 模块 | 描述 | 优先级 | 预估行数 |
|----|------|------|--------|---------|
| BUG-T01 | 终端 | 方向前缀着色使用了 darker(130) 而非设计配色 | P2 | 10 |
| BUG-T02 | 终端 | Ctrl+F 搜索栏在非终端面板时无响应 | P1 | 5 |
| BUG-T03 | 终端 | 终端清屏后搜索状态未清除 | P1 | 1 |
| BUG-T04 | 终端 | 纯文本搜索缺少大小写切换按钮 | P2 | -- (记录) |
| BUG-T05 | 终端 | 十进制模式缺少 ASCII 参考 | P2 | -- (记录) |
| BUG-S01 | 串口 | 连接按钮"连接中..."状态可能卡住 | P1 | 15 |
| BUG-S02 | 串口 | 热插拔后端口列表未自动刷新 | P1 | 8 |
| BUG-S03 | 串口 | 自定义波特率 QIntValidator 限制过严 | P2 | 3 |
| BUG-F01 | 发送 | 定时发送器 UI 入口缺失 | P1 | 50 |
| BUG-F02 | 发送 | 发送历史自动补全未按模式过滤 | P2 | 15 |
| BUG-F03 | 发送 | 快捷指令编辑后不自动持久化 | P1 | 2 |
| BUG-P01 | 协议 | 帧缓冲区溢出后当前字节可能丢弃 | P2 | -- (需确认) |
| BUG-P02 | 协议 | VOFA+ 模式切换后 ChartWidget 通道未重建 | P1 | 25 |
| BUG-C01 | 波形图 | 图表背景未从 ThemeManager 获取配色 | P1 | 20 |
| BUG-C02 | 波形图 | 首次显示可能空白（showEvent 修复缺失） | P2 | 15 |
| BUG-C03 | 波形图 | 暂停/清除按钮缺少 QSS 样式 | P1 | 30 (QSS) |
| BUG-O01 | OTA | 传输启动未检查连接状态 | P0 | 5 |
| BUG-O02 | OTA | 取消传输后进度条未重置 | P2 | 2 |
| BUG-O03 | OTA | OTA 面板控件缺少 objectName | P2 | 10 |
| BUG-U03 | UI | ConnectionController.cpp 超 500 行上限 | P0 | 150 (拆分) |

### 1.9 优先级排序

**P0 Bug（必须修复，否则禁止新特性开发）**:
1. BUG-O01: OTA 传输未检查连接状态 -- 可能导致崩溃
2. BUG-U03: ConnectionController.cpp 超 500 行上限 -- 违反铁律

**P1 Bug（应该修复，功能受限或体验严重受损）**:
3. BUG-T02: Ctrl+F 非终端面板时无响应
4. BUG-T03: 清屏后搜索状态残留
5. BUG-S01: 连接按钮状态可能卡住
6. BUG-S02: 热插拔后端口列表未自动刷新
7. BUG-F01: 定时发送器 UI 入口缺失
8. BUG-F03: 快捷指令编辑后不持久化
9. BUG-P02: VOFA+ 模式切换后通道未重建
10. BUG-C01: 图表背景配色未使用 ThemeManager
11. BUG-C03: ChartWidget 控件缺少 QSS 样式

**P2 Bug（体验不佳，可延后）**:
12. BUG-T01: 方向前缀配色与设计不符
13. BUG-T04: 搜索缺少大小写切换
14. BUG-T05: 十进制模式缺少 ASCII 参考
15. BUG-S03: 波特率 Validator 限制过严
16. BUG-F02: 发送历史自动补全未按模式过滤
17. BUG-P01: 帧溢出后字节可能丢弃
18. BUG-C02: ChartWidget 首次显示可能空白
19. BUG-O02: OTA 取消后进度条未重置
20. BUG-O03: OTA 控件缺少 objectName

---

## 二、新特性提案

按 CLAUDE.md 3.4.4 节规定，每 3 次迭代进行一次新特性提案评估。以下为本次迭代提出的 3 个新特性候选。

---

### FEATURE-002: 定时发送器 UI 集成

**动机**: TimedSender 功能已完整实现（底层定时器、间隔控制、数据发送），但发送栏 UI 中没有对应的配置入口。用户无法使用这一已经开发完成的功能。将定时发送器暴露到 UI 是最小投入最大产出的改进。

**方案**:
- 在发送栏右侧添加 checkable 的"定时"按钮（"定时发送"）
- 按钮旁放置 QSpinBox（10ms~60000ms，默认 1000ms）
- 勾选后按间隔周期性发送输入框当前内容
- 输入框为空或取消勾选时自动停止

**工作量估算**: 约 80 行代码（SendController.cpp 50 行 + QSS 30 行），1 个迭代内完成。

**用户价值**: 嵌入式开发中大量场景需要周期性发送数据（心跳包、传感器查询、状态轮询），定时发送是串口调试工具的标配功能。

**优先级**: P1 -- 功能已实现只需 UI 暴露，ROI 极高。

---

### FEATURE-003: 搜索增强 -- 大小写切换与正则预设模板

**动机**: 当前搜索栏支持纯文本/正则/HEX 三种模式，但缺少两个高频需求: (1) 大小写敏感切换（嵌入式调试中常需忽略大小写搜索 AT 指令响应）; (2) 常用正则预设模板（如匹配 IP 地址、匹配数值、匹配 HEX 序列等），降低正则使用门槛。

**方案**:
- TerminalSearchBar 增加大小写切换按钮（Aa 图标）
- 增加正则预设下拉菜单（IP: `\d+\.\d+\.\d+\.\d+`，数值: `\d+(\.\d+)?`，HEX: `[0-9A-Fa-f]+`）
- 搜索逻辑中根据大小写标志选择 `Qt::CaseSensitive` 或 `Qt::CaseInsensitive`

**工作量估算**: 约 60 行代码，1 个迭代内完成。

**用户价值**: 降低搜索使用门槛，减少手动输入正则的出错率。

**优先级**: P2 -- 体验优化，非核心功能。

---

### FEATURE-004: 通知吐司系统 (Toast Notification)

**动机**: 当前所有状态反馈通过状态栏（`statusBar()->showMessage()`）或 QMessageBox 实现。状态栏消息容易被忽略（用户注意力在终端区域），QMessageBox 则打断工作流（需要点击确认）。CLAUDE.md 6.5 节明确要求实现通知吐司弹出/消失动画，但当前代码中没有 Toast 组件。

**方案**:
- 新增 `ToastWidget` 类（QWidget，无标题栏，固定在父窗口右下角）
- 支持 success/warning/error 三种级别（左侧彩色边框 + 图标 + 消息文字）
- 自动消失（3 秒），带向上飘出消失动画
- 提供 `ToastManager::show(message, level)` 静态接口
- 替换现有 `statusBar()->showMessage()` 为 Toast 通知（状态栏保留连接状态）

**工作量估算**: 约 200 行代码（ToastWidget 150 行 + ThemeManager 集成 30 行 + QSS 20 行），1-2 个迭代。

**用户价值**: 统一反馈机制，不打断工作流的同时确保用户注意到关键操作结果。

**优先级**: P1 -- CLAUDE.md 6.5 节明确要求的动画组件，且改善整体 UX。

---

## 三、需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | BUG-O01: OTA 传输启动前检查连接状态 | P0 | ota/OtaWidget.cpp, ota/OtaManager.h |
| R2 | BUG-U03: ConnectionController.cpp 拆分为 500 行以内 | P0 | core/ConnectionController.cpp, 新增 core/HotPlugHandler.h/cpp, core/ReconnectManager.h/cpp |
| R3 | BUG-T02: Ctrl+F 自动切换到终端面板 | P1 | core/MainWindow.cpp |
| R4 | BUG-T03: 终端清屏时清除搜索状态 | P1 | terminal/TerminalWidget.cpp |
| R5 | BUG-S01: 连接按钮超时保护 | P1 | serial/SerialConfigPanel.h/cpp |
| R6 | BUG-S02: 热插拔自动刷新端口列表 | P1 | core/MainWindowSignalConnect.cpp |
| R7 | BUG-F01: 定时发送器 UI 集成 | P1 | core/SendController.h/cpp |
| R8 | BUG-F03: 快捷指令编辑后自动持久化 | P1 | serial/QuickCommandBar.cpp |
| R9 | BUG-P02: VOFA+ 模式切换后 ChartWidget 通道重建 | P1 | core/MainWindowSignalConnect.cpp, chart/ChartWidget.h/cpp |
| R10 | BUG-C01: ChartWidget 使用 ThemeManager 配色 | P1 | chart/ChartWidget.cpp |
| R11 | BUG-C03: ChartWidget 控件 QSS 样式 | P1 | resources/themes/*.qss |
| R12 | BUG-T01: 方向前缀使用设计配色替代 darker(130) | P2 | terminal/TerminalWidget.cpp |
| R13 | BUG-O02: OTA 取消后重置进度条 | P2 | ota/OtaWidget.cpp |
| R14 | BUG-O03: OTA 面板控件添加 objectName | P2 | ota/OtaWidget.cpp |

---

## 四、需求详细说明

---

### R1: OTA 传输启动前检查连接状态 (P0)

#### 问题分析

`OtaWidget::onStartTransfer()` 在用户点击"开始传输"时直接调用 `m_manager->startTransfer()`，不检查连接是否存在。如果 `OtaManager` 内部的 `m_connection` 为 nullptr（未连接状态），`startTransfer()` 会尝试从空指针写入数据。

需要确认 `OtaManager::startTransfer()` 内部是否有空指针检查。如果有，则此 Bug 降级为 P1（用户体验问题，不会崩溃但错误信息不友好）。如果没有，保持 P0。

#### 修改方案

在 `onStartTransfer()` 开头添加前置检查:
```cpp
if (!m_manager->hasConnection()) {
    appendLog(tr("错误: 未建立连接，请先连接串口"));
    return;
}
```

需要在 `OtaManager` 中添加 `hasConnection()` 方法（或暴露 `connection()` getter）。

---

### R2: ConnectionController.cpp 拆分为 500 行以内 (P0)

#### 问题分析

当前 ConnectionController.cpp 525 行，超出 CLAUDE.md 4.6 节 500 行上限。文件包含:
- 构造/析构/依赖注入: ~80 行
- connectSerial: ~65 行
- disconnectCurrent: ~25 行
- connectNetwork: ~60 行
- setDtr/setRts/enableAutoReconnect: ~30 行
- onConnectionStateChanged: ~40 行
- onDataReceived: ~10 行
- onConnectionTimeout: ~25 行
- onAutoReconnect: ~15 行
- onPortRemoved/onPortAdded: ~45 行
- connectSignals/clearDownstream: ~30 行

#### 拆分方案

将热插拔和自动重连逻辑提取到独立类:

**HotPlugHandler** (~70 行):
- `onPortRemoved(portName)` -- 端口拔出自动断开
- `onPortAdded(portName)` -- 端口接入通知
- 持有 PortWatcher 实例

**ReconnectManager** (~50 行):
- `enableAutoReconnect(bool, int)` -- 启用/禁用
- `onAutoReconnect()` -- 定时器触发重连
- `shouldReconnect()` -- 判断是否需要重连
- 持有重连定时器和上次连接参数

拆分后 ConnectionController.cpp 约 350 行，HotPlugHandler.cpp 约 70 行，ReconnectManager.cpp 约 50 行，全部在 500 行上限内。

---

### R3: Ctrl+F 自动切换到终端面板 (P1)

修改 `MainWindow::setupUI()` 中 Ctrl+F 快捷键的连接:
```cpp
auto* searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
connect(searchShortcut, &QShortcut::activated, this, [this]() {
    m_navController->switchToPanel(m_panelManager->terminal());
    m_panelManager->searchBar()->activate();
});
```

---

### R4: 终端清屏时清除搜索状态 (P1)

修改 `TerminalWidget::clear()`:
```cpp
void TerminalWidget::clear()
{
    m_cachedLines.clear();
    m_cachedLineCount = 0;
    m_directionFilter->reset();
    m_selectionManager->reset();
    m_searchManager->clearSearchHighlight();  // 新增
    update();
}
```

---

### R5: 连接按钮超时保护 (P1)

在 `SerialConfigPanel` 中添加 `QTimer* m_connectTimeoutTimer`:
```cpp
// setupUI() 中:
m_connectTimeoutTimer = new QTimer(this);
m_connectTimeoutTimer->setSingleShot(true);
connect(m_connectTimeoutTimer, &QTimer::timeout, this, [this]() {
    m_connecting = false;
    m_connectBtn->setEnabled(true);
    updateConnectButtonState();
});

// 连接按钮点击时:
m_connectTimeoutTimer->start(2000);  // 2秒超时

// setConnected() 中:
m_connectTimeoutTimer->stop();
```

---

### R6: 热插拔自动刷新端口列表 (P1)

在 `MainWindowSignalConnect.cpp` connectSignals() 中添加:
```cpp
connect(m_connController, &ConnectionController::portAdded,
        m_panelManager->serialConfig(), &SerialConfigPanel::refreshPorts);
```

需确认 `ConnectionController::portAdded` 信号已在头文件中声明。

---

### R7: 定时发送器 UI 集成 (P1)

在 `SendController::createSendBar()` 中添加定时发送 UI:
- `m_timedSendBtn`: QToolButton（checkable，图标式，"定时"）
- `m_intervalSpin`: QSpinBox（10~60000，单位 ms，默认 1000）
- 勾选定时按钮时启动 `m_timedSender`，取消勾选时停止
- 定时发送内容为当前输入框文本

---

### R8: 快捷指令编辑后自动持久化 (P1)

修改 `QuickCommandBar::onEditRequested()`:
```cpp
if (dlg.exec() == QDialog::Accepted) {
    // ... 现有读取表格逻辑 ...
    setCommands(newCmds);
    saveCommands();  // 新增
}
```

同时在 `PanelManager::createPanels()` 或 `MainWindow` 构造中调用 `m_panelManager->quickCmdBar()->loadCommands()` 加载持久化的指令。

---

### R9: VOFA+ 模式切换后 ChartWidget 通道重建 (P1)

在 `connectSignals()` 中添加:
```cpp
connect(m_protocolBridgeMgr, &ProtocolBridgeManager::protocolModeChanged,
        this, [this](auto mode) {
    if (mode == ProtocolBridgeManager::ChartProtocolMode::JustFloat) {
        int ch = m_protocolBridgeMgr->justFloatBridge()->channelCount();
        m_panelManager->chartWidget()->configureFromFrameDefinition(
            FrameDefinition::justFloatDefaults(ch));
    }
});
```

需要在 `FrameDefinition` 中添加静态方法 `justFloatDefaults(int channelCount)` 生成默认配置。

---

### R10: ChartWidget 使用 ThemeManager 配色 (P1)

在 `ChartWidget::setupUI()` 末尾添加 ThemeManager 配色，并监听 `themeChanged` 信号动态更新。

---

### R11: ChartWidget 控件 QSS 样式 (P1)

在三个主题 QSS 文件中添加 PRD-034 R7.2 定义的样式规则（暂停/清除按钮、窗口大小下拉框、状态标签）。

---

## 五、依赖的公共组件

| 组件 | 用途 | 涉及需求 |
|------|------|---------|
| `ThemeManager` | ChartWidget 配色、方向前缀配色 | R10, R12 |
| `PortWatcher` | 热插拔检测 | R6 |
| `TimedSender` | 定时发送器底层 | R7 |
| `SendHistory` | 发送历史管理 | R8 |
| `FrameDefinition` | VOFA+ 默认通道配置生成 | R9 |
| `ConnectionController` | 拆分重构 | R2 |
| `OtaManager` | 连接状态检查 | R1 |

---

## 六、设计模式

| 模式 | 应用场景 | 涉及需求 |
|------|---------|---------|
| **策略模式 (Strategy)** | HotPlugHandler/ReconnectManager 从 ConnectionController 中提取为独立策略 | R2 |
| **观察者模式 (Observer)** | 热插拔信号 -> SerialConfigPanel 自动刷新 | R6 |
| **组合模式 (Composite)** | Toast 通知系统（如果 FEATURE-004 通过） | FEATURE-004 |

---

## 七、影响范围

### 文件变更矩阵

| 文件 | 变更类型 | R1 | R2 | R3 | R4 | R5 | R6 | R7 | R8 | R9 | R10 | R11 | R12 | R13 | R14 |
|------|---------|-----|-----|-----|-----|-----|-----|-----|-----|-----|------|------|------|------|------|
| `ota/OtaWidget.cpp` | 修改 | +5 | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | +2 | +10 |
| `ota/OtaManager.h` | 修改 | +3 | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| `core/ConnectionController.cpp` | 修改 | -- | -80 | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| `core/ConnectionController.h` | 修改 | -- | -10 | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| `core/HotPlugHandler.h` | **新增** | -- | +40 | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| `core/HotPlugHandler.cpp` | **新增** | -- | +70 | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| `core/ReconnectManager.h` | **新增** | -- | +30 | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| `core/ReconnectManager.cpp` | **新增** | -- | +50 | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| `core/MainWindow.cpp` | 修改 | -- | -- | +3 | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| `terminal/TerminalWidget.cpp` | 修改 | -- | -- | -- | +1 | -- | -- | -- | -- | -- | -- | -- | +5 | -- | -- |
| `serial/SerialConfigPanel.h` | 修改 | -- | -- | -- | -- | +2 | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| `serial/SerialConfigPanel.cpp` | 修改 | -- | -- | -- | -- | +15 | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| `core/MainWindowSignalConnect.cpp` | 修改 | -- | -- | -- | -- | -- | +5 | -- | -- | +15 | -- | -- | -- | -- | -- |
| `core/SendController.h` | 修改 | -- | -- | -- | -- | -- | -- | +5 | -- | -- | -- | -- | -- | -- | -- |
| `core/SendController.cpp` | 修改 | -- | -- | -- | -- | -- | -- | +50 | -- | -- | -- | -- | -- | -- | -- |
| `serial/QuickCommandBar.cpp` | 修改 | -- | -- | -- | -- | -- | -- | -- | +2 | -- | -- | -- | -- | -- | -- |
| `chart/ChartWidget.cpp` | 修改 | -- | -- | -- | -- | -- | -- | -- | -- | -- | +20 | -- | -- | -- | -- |
| `resources/themes/*.qss` (x3) | 修改 | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | +90 | -- | -- | -- |
| `protocol/FrameDefinition.h/cpp` | 修改 | -- | -- | -- | -- | -- | -- | -- | -- | +10 | -- | -- | -- | -- | -- |
| `CMakeLists.txt` | 修改 | -- | +4 | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |

### 预计变更量

| 类别 | 新增行数(估) | 修改行数(估) | 删除行数(估) |
|------|------------|------------|------------|
| P0 修复 (R1, R2) | 190 行 | 10 行 | 90 行 |
| P1 修复 (R3-R11) | 150 行 | 20 行 | 5 行 |
| P2 修复 (R12-R14) | 15 行 | 5 行 | 0 行 |
| **合计** | **约 355 行** | **约 35 行** | **约 95 行** |

---

## 八、验收标准

### P0 验收（必须全部通过才能 commit）

| 编号 | 验收项 | 通过条件 |
|------|--------|---------|
| AC-P0-01 | 编译零错误 | cmake --build build 无错误 |
| AC-P0-02 | EmbedDebug.bat 启动正常 | 双击 bat 文件应用窗口正常显示 |
| AC-P0-03 | OTA 未连接时传输不崩溃 | 未连接状态下点击"开始传输"显示错误日志而非崩溃 |
| AC-P0-04 | ConnectionController.cpp 行数 <= 500 | wc -l 确认 |
| AC-P0-05 | HotPlugHandler.cpp 行数 <= 100 | wc -l 确认 |
| AC-P0-06 | ReconnectManager.cpp 行数 <= 100 | wc -l 确认 |

### P1 验收（建议通过）

| 编号 | 验收项 | 通过条件 |
|------|--------|---------|
| AC-P1-01 | Ctrl+F 在非终端面板时切换到终端面板 | 查看串口配置面板时按 Ctrl+F，自动切换到终端并激活搜索栏 |
| AC-P1-02 | 清屏后搜索高亮消失 | 搜索后点击清屏，搜索高亮消失 |
| AC-P1-03 | 连接按钮 2 秒超时保护 | 模拟连接失败场景，按钮在 2 秒后自动恢复 |
| AC-P1-04 | 插入 USB 设备后端口列表自动刷新 | 插入 CH340 设备，端口列表无需手动刷新即显示新端口 |
| AC-P1-05 | 定时发送按钮可用 | 勾选定时按钮，数据按设定间隔周期性发送 |
| AC-P1-06 | 快捷指令编辑后重启不丢失 | 编辑快捷指令 -> 重启应用 -> 指令仍在 |
| AC-P1-07 | 暗色主题下图表背景为深色 | 图表区域背景色与整体暗色主题协调 |
| AC-P1-08 | ChartWidget 按钮有暗色样式 | 暂停/清除按钮在暗色主题下不是白色默认样式 |
| AC-P1-09 | VOFA+ 模式切换后波形图通道更新 | 切换到 JustFloat 模式后 ChartWidget 显示对应通道 |

### P2 验收（可选）

| 编号 | 验收项 | 通过条件 |
|------|--------|---------|
| AC-P2-01 | 方向前缀 [TX:] 显示为绿色 #a6e3a1 | 取色器验证 |
| AC-P2-02 | 方向前缀 [RX:] 显示为蓝色 #89b4fa | 取色器验证 |
| AC-P2-03 | OTA 取消后进度条归零 | 传输中取消，进度条回到 0% |
| AC-P2-04 | OTA 控件在暗色主题下可读 | 所有 OTA 面板控件样式正确 |

---

## 九、实施优先级

```
第一批（P0，阻塞所有其他工作）:
  R1 OTA 连接状态检查 ─────── 10 分钟 (最小修改)
  R2 ConnectionController 拆分 ─ 60 分钟 (新增 2 个文件)

第二批（P1，可并行开发）:
  R3 Ctrl+F 自动切换面板 ──── 5 分钟
  R4 清屏清除搜索状态 ────── 2 分钟
  R5 连接按钮超时保护 ────── 15 分钟
  R6 热插拔自动刷新端口 ──── 10 分钟
  R8 快捷指令持久化 ──────── 5 分钟
  R13 OTA 取消后进度条重置 ── 5 分钟
  R14 OTA 控件 objectName ─── 10 分钟

第三批（P1，需较多代码）:
  R7 定时发送器 UI 集成 ──── 40 分钟
  R9 VOFA+ 通道重建 ──────── 30 分钟
  R10 ChartWidget ThemeManager ─ 20 分钟
  R11 ChartWidget QSS 样式 ─── 30 分钟 (三主题文件)

第四批（P2，体验优化）:
  R12 方向前缀配色修正 ────── 10 分钟
```

**预估总工作量**: 约 4 小时

---

## 十、新特性提案评审结论

| 提案 | 用户价值 | 架构影响 | 开发成本 | 复用潜力 | 建议 |
|------|---------|---------|---------|---------|------|
| FEATURE-002 定时发送 UI | 高（已实现只差 UI） | 低（SendController 内部变更） | 80 行 | -- | **通过** |
| FEATURE-003 搜索增强 | 中（体验优化） | 低（TerminalSearchBar 扩展） | 60 行 | -- | 暂缓 |
| FEATURE-004 Toast 通知 | 高（CLAUDE.md 要求） | 中（新增组件） | 200 行 | 高（全应用复用） | 暂缓 |

**决策**: 本次仅通过 FEATURE-002（定时发送器 UI），因为它实际上是对 BUG-F01 的修复而非新特性。FEATURE-003 和 FEATURE-004 暂缓到下一轮新特性评估。

---

## 十一、本迭代不涉及的内容

| 排除项 | 原因 |
|--------|------|
| BUG-T04 搜索大小写切换 | P2 体验优化，延后到 FEATURE-003 统一实现 |
| BUG-T05 十进制模式 ASCII 参考 | P2 体验优化，使用频率低 |
| BUG-S03 波特率 Validator | P2 极端边界条件 |
| BUG-F02 发送历史模式过滤 | P2 体验优化 |
| BUG-P01 帧溢出字节丢弃 | P2 需要进一步确认是否真实存在 |
| BUG-C02 ChartWidget showEvent | P2 需要实际运行验证 |
| ZModem 协议实现 | 新特性，不在 Bug 修复冲刺范围内 |
