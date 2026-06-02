# PRD-032: 终端拆分、MainWindow 瘦身与串口体验增强

## 背景

commit #31 已完成串口功能无死角完善（IConnection DTR/RTS 虚方法、错误分类、连接超时、自动重连、SerialConfigPanel UX 改进等），当前评分 33 分。

经全面审查 `TerminalWidget.cpp`(714 行)、`MainWindow.cpp`(603 行)、`SerialConfigPanel.cpp`(388 行)、`QuickCommandBar.cpp`(189 行)、`SerialDriverDetector.cpp`(115 行) 的实际代码状态，识别出以下需要解决的问题:

**代码质量问题（违反 CLAUDE.md 行数上限铁律）**:

1. **TerminalWidget.cpp 714 行，超标 214 行** -- 搜索逻辑（`setSearchHighlight` 方法 113 行）与绘制逻辑、缓存管理、事件处理混在一个文件中。搜索逻辑在普通模式和方向过滤模式下各有 hex/regex/plain 三条分支，共六条路径，是膨胀的主要来源。

2. **MainWindow.cpp 603 行，超标 103 行** -- `connectSignals()` 方法（行 244~429，185 行）是最大单体方法，包含 18 段 connect 调用。`onExportData()`（30 行）已委托 DataExporter 但仍占用 MainWindow 空间。

**串口体验缺陷**:

3. **无 USB 热插拔检测** -- `SerialConfigPanel::refreshPorts()` 仅在启动时和用户手动点击"刷新"时调用。用户插入/拔出 USB 转串口适配器后，端口列表不会自动更新，必须手动点击刷新。Windows 系统支持 `WM_DEVICECHANGE` 消息通知设备变化，Qt 6.8.3 的 `QSerialPortInfo` 不提供异步设备通知，需要通过 `QTimer` 轮询或原生事件过滤实现。

4. **波特率输入仍无校验** -- `SerialConfigPanel::currentBaudRate()` 调用 `m_baudCombo->currentText().toInt()`，对非数字输入（如 "abc"、"115k2"）静默返回 0。PRD-031 R1.1 已规划但未实现。0 波特率传递给 `QSerialPort::setBaudRate(0)` 会触发 `UnsupportedOperationError` 或使用默认 9600，行为不可预测。

5. **快捷指令无持久化** -- `QuickCommandBar` 的 `m_commands` 列表仅存内存。`onEditRequested()` 方法已有完整的编辑对话框（QTableWidget + 添加行/删除行/确定/取消），编辑功能存在但指令不保存。PRD-031 R3.4 已规划但未实现。

**审查基准**: commit #31, score 33。

---

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | TerminalWidget.cpp 拆分: 搜索逻辑提取到独立文件，降至 500 行以下 | P0 | terminal/TerminalWidget.h/cpp |
| R2 | MainWindow.cpp 瘦身: 导出和状态栏逻辑提取，降至 500 行以下 | P0 | core/MainWindow.h/cpp |
| R3 | USB 热插拔检测: 设备插入/拔出时自动刷新端口列表 | P0 | serial/SerialConfigPanel.h/cpp |
| R4 | 波特率输入校验: 可编辑 ComboBox 输入非数字时回退到 115200 | P0 | serial/SerialConfigPanel.h/cpp |
| R5 | 快捷指令持久化: 保存/加载/编辑指令到 SettingsManager | P0 | serial/QuickCommandBar.h/cpp |

---

## 需求详细说明

---

### R1: TerminalWidget.cpp 拆分 (P0)

#### R1.1 问题分析

`TerminalWidget.cpp` 共 714 行，行数分布如下:

| 方法/区域 | 行数 | 职责 |
|----------|------|------|
| 构造函数 + 属性 setter (行 11~179) | ~168 行 | 初始化、模型绑定、显示配置 |
| `paintLine()` (行 182~259) | 77 行 | 单行渲染（时间戳/选择/搜索高亮/方向前缀） |
| `paintEvent()` (行 261~379) | 118 行 | 绘制入口（缓存管理 + 逐行绘制） |
| 事件处理 (行 381~455) | 74 行 | resize/wheel/mouse/key 事件 |
| `setSearchHighlight()` (行 506~619) | **113 行** | **搜索扫描（六条分支路径）** |
| 搜索辅助方法 (行 621~678) | 57 行 | clearSearch/gotoNext/gotoPrev/scrollToMatch/refreshSearch |
| `formatToCache()` (行 680~714) | 34 行 | 数据格式化到缓存 |
| 其他 (onDataAppended/onDataCleared/updateVisibleRange) | ~50 行 | 模型事件处理 |

`setSearchHighlight()` 是最膨胀的方法（113 行），它在普通模式和过滤模式下各有 hex/regex/plain 三条分支。搜索辅助方法再占 57 行。搜索相关代码合计约 170 行，提取后可将 TerminalWidget.cpp 降至约 544 行。加上将 `formatToCache` 的显示格式化逻辑提取后，TerminalWidget.cpp 可降至 500 行以下。

#### R1.2 拆分方案

提取 `TerminalSearchEngine` 类到独立文件 `terminal/TerminalSearchEngine.h/cpp`:

**TerminalSearchEngine 职责**:
- 管理搜索状态（pattern、regex 标志、hex 标志）
- 执行搜索扫描（普通模式和方向过滤模式）
- 维护搜索匹配列表 (`QVector<SearchMatch>`)
- 导航功能（gotoNext/gotoPrev/currentMatchIndex）
- 搜索匹配行映射和滚动定位

**TerminalWidget 保留的职责**:
- 渲染（paintEvent/paintLine）
- 缓存管理（formatToCache/m_cachedLines）
- 滚动和事件处理
- 方向过滤的渲染路径（使用 DirectionFilter 的过滤索引）

**接口设计**:

```cpp
// terminal/TerminalSearchEngine.h

struct SearchMatch {
    int line;       ///< 匹配所在行号（显示坐标系）
    int startCol;   ///< 匹配起始列
    int length;     ///< 匹配长度
};

/**
 * @brief 终端搜索引擎 - 从 TerminalWidget 中提取的搜索逻辑
 *
 * 负责在缓存行数据中搜索匹配项，支持三种搜索模式:
 *   - 纯文本搜索（大小写敏感）
 *   - 正则表达式搜索
 *   - HEX 字节搜索
 *
 * 支持两种数据源:
 *   - 普通模式: 直接从 m_cachedLines 搜索
 *   - 方向过滤模式: 通过 DirectionFilter 索引映射后再搜索
 */
class TerminalSearchEngine : public QObject {
    Q_OBJECT
public:
    explicit TerminalSearchEngine(QObject* parent = nullptr);

    /** @brief 执行搜索扫描 */
    void search(const QString& pattern, bool regex, bool hex,
                const QVector<CachedLine>& cachedLines,
                DirectionFilter* directionFilter,
                TerminalModel* model);

    /** @brief 清除搜索结果 */
    void clear();

    /** @brief 跳转到下一个匹配 */
    void gotoNextMatch();

    /** @brief 跳转到上一个匹配 */
    void gotoPrevMatch();

    /** @brief 搜索匹配总数 */
    int matchCount() const;

    /** @brief 当前匹配索引 (0-based), -1 表示无匹配 */
    int currentMatchIndex() const;

    /** @brief 获取当前匹配所在行号，用于滚动定位 */
    int currentMatchLine() const;

    /** @brief 获取所有搜索匹配（paintLine 绘制搜索高亮时使用） */
    const QVector<SearchMatch>& matches() const;

    /** @brief 获取当前匹配的行号 */
    int currentLine() const;

    /** @brief 是否有活跃的搜索模式 */
    bool isActive() const;

signals:
    /** @brief 搜索匹配结果变化 @param total 总匹配数 @param current 当前匹配索引 */
    void matchesChanged(int total, int current);

private:
    /** @brief 在缓存行中执行纯文本搜索 */
    void searchPlain(const QString& pattern,
                     const QVector<CachedLine>& cachedLines,
                     DirectionFilter* filter);
    /** @brief 在缓存行中执行正则搜索 */
    void searchRegex(const QString& pattern,
                     const QVector<CachedLine>& cachedLines,
                     DirectionFilter* filter);
    /** @brief 在原始数据中执行 HEX 搜索 */
    void searchHex(const QString& pattern,
                   const QVector<CachedLine>& cachedLines,
                   DirectionFilter* filter,
                   TerminalModel* model);

    QVector<SearchMatch> m_matches;
    int m_currentMatchIndex = -1;
    QString m_pattern;
    bool m_regex = false;
    bool m_hex = false;
};
```

**TerminalWidget 修改**:
- 移除 `SearchMatch` 结构体定义（移至 TerminalSearchEngine.h）
- 移除 `m_searchMatches`、`m_currentMatchIndex`、`m_searchPattern`、`m_searchRegex`、`m_searchHex` 成员
- 移除 `m_searchHighlightColor`、`m_currentMatchColor` 成员
- 新增 `TerminalSearchEngine* m_searchEngine` 成员
- `setSearchHighlight()` 委托给 `m_searchEngine->search()`
- `clearSearchHighlight()` 委托给 `m_searchEngine->clear()`
- `paintLine()` 中搜索高亮绘制改为从 `m_searchEngine->matches()` 读取
- `keyPressEvent()` 中 F3 导航委托给 `m_searchEngine->gotoNextMatch()/gotoPrevMatch()`
- `searchMatchesChanged` 信号连接到 `m_searchEngine->matchesChanged`

**预期行数变化**:
- TerminalWidget.cpp: 714 - 170 (搜索逻辑) = ~544 行
- TerminalSearchEngine.cpp: ~170 行（新增）
- TerminalSearchEngine.h: ~70 行（新增）

**验收标准**:
- TerminalWidget.cpp 行数 <= 500
- 搜索功能（纯文本/正则/HEX）行为与拆分前完全一致
- 方向过滤模式下的搜索行为与拆分前完全一致
- F3/Shift+F3 搜索导航正常工作
- 编译零错误

---

### R2: MainWindow.cpp 瘦身 (P0)

#### R2.1 问题分析

`MainWindow.cpp` 共 603 行，行数分布如下:

| 方法 | 行数 | 职责 |
|------|------|------|
| 构造函数 (行 39~103) | 64 行 | 初始化所有组件、构建 UI、连接信号 |
| `setupUI()` (行 125~210) | 85 行 | UI 布局构建 |
| `setupStatusBar()` (行 216~229) | 13 行 | 状态栏创建 |
| `connectSignals()` (行 244~429) | **185 行** | **信号连接（18 段）** |
| `onDisplayModeChanged()` (行 435~442) | 7 行 | 显示模式切换 |
| `onTimestampToggled()` (行 448~454) | 6 行 | 时间戳开关 |
| `onClearTerminal()` (行 457~462) | 5 行 | 清屏 |
| `onExportData()` (行 470~500) | 30 行 | 导出逻辑 |
| `onSearchRequested()` (行 508~511) | 3 行 | 搜索委托 |
| `onSearchCleared()` (行 514~517) | 3 行 | 清搜索 |
| `onBgSettingsToggled()` (行 523~536) | 13 行 | 背景面板切换 |
| `onTerminalLayoutChanged()` (行 547~550) | 3 行 | 终端布局切换 |
| `updateStatusBar()` (行 556~570) | 14 行 | 字节数格式化 |
| `updateDataStatistics()` (行 573~578) | 5 行 | 统计刷新 |
| `closeEvent()` (行 585~603) | 18 行 | 关闭清理 |

`connectSignals()` 是最大膨胀源（185 行）。它包含 18 段 connect 调用，每段对应一个功能区域。这些信号连接本质上是"中介者"模式的胶水代码，但由于 MainWindow 是唯一知道所有模块实例的地方，难以整体提取。

#### R2.2 瘦身方案

提取以下逻辑到独立方法或现有 Controller 中:

**R2.2.1 提取 `updateStatusBar()` 的字节数格式化到内联函数**

将 `formatBytes` lambda 提取为 `MainWindow` 的私有静态方法，减少 updateStatusBar 内部复杂度。这不会显著减少行数但改善可读性。

**R2.2.2 将 `onExportData()` 提取到 `DataExporter`**

当前 `onExportData()` 中有 30 行逻辑（文件对话框、格式判断、lineProvider lambda），可提取为 `DataExporter::exportWithDialog()` 静态方法:

```cpp
// utils/DataExporter.h 新增:
/**
 * @brief 弹出文件对话框并导出终端数据
 * @param parent 父窗口（用于对话框定位）
 * @param terminalModel 数据源
 * @return 导出文件路径，空字符串表示取消或失败
 */
static QString exportWithDialog(QWidget* parent, TerminalModel* model);
```

MainWindow 中缩减为:
```cpp
void MainWindow::onExportData() {
    QString path = DataExporter::exportWithDialog(this, m_terminalModel);
    if (!path.isEmpty()) {
        statusBar()->showMessage(tr("Exported to %1").arg(path), 3000);
    }
}
```

**R2.2.3 将 `connectSignals()` 按功能拆分为多个私有方法**

将 185 行的 `connectSignals()` 拆分为 4 个子方法:

```cpp
private:
    void connectSerialSignals();      ///< 串口连接/断开/数据收发信号
    void connectToolbarSignals();     ///< 工具栏显示模式/主题/语言信号
    void connectSearchSignals();      ///< 搜索栏信号
    void connectProtocolSignals();    ///< 协议解析/帧编辑器信号
    void connectNavSignals();         ///< 导航树面板切换信号
```

每个子方法约 30~45 行，`connectSignals()` 本身变为 5 行调用:

```cpp
void MainWindow::connectSignals() {
    connectSerialSignals();
    connectToolbarSignals();
    connectSearchSignals();
    connectProtocolSignals();
    connectNavSignals();
}
```

**预期行数变化**:
- MainWindow.cpp: 603 - 30 (导出提取) - 0 (connectSignals 拆分不减少总行数但改善可读性) = ~573 行
- 上述措施不够。需要进一步提取 connectSignals 的信号连接逻辑。

**补充方案: 将面板切换逻辑提取到 NavigationController**

当前 `connectSignals()` 中导航树点击的处理（行 411~428，18 行）可移入 NavigationController:

```cpp
// NavigationController 新增方法:
void connectNavTree(QTreeView* tree, PanelManager* panelManager);
```

**最终预期行数**:
- MainWindow.cpp: 603 - 30 (导出) - 18 (导航树) - 5 (formatBytes) = ~550 行
- 还需继续精简。将 `onExportData` 完整提取（30行）+ 导航树信号连接提取（18行）+ 背景面板切换（13行）+ 搜索相关（6行）= 67 行。603 - 67 = 536 行。

**更积极的方案: 将所有 `onXxx` 槽函数移入对应的 Controller**

| 方法 | 目标位置 | 减少行数 |
|------|---------|---------|
| `onDisplayModeChanged()` | 已在 ToolbarController 信号路径上，直接连到 TerminalWidget | 7 |
| `onTimestampToggled()` | 同上 | 6 |
| `onClearTerminal()` | 新增 TerminalController 或由 SendController 处理 | 5 |
| `onExportData()` | DataExporter::exportWithDialog() | 30 |
| `onSearchRequested/onSearchCleared` | 搜索信号直连 TerminalWidget | 6 |
| `onBgSettingsToggled()` | BackgroundWidget 自管理 | 13 |
| `onTerminalLayoutChanged()` | TerminalLayoutManager 自管理 | 3 |
| `updateStatusBar()` + `updateDataStatistics()` | ConnectionController 管理 | 19 |

这些槽函数合计 89 行，提取后 MainWindow.cpp 降至 ~514 行。加上 `connectSignals()` 拆分为子方法减少的冗余空行（约 10 行），可达到 500 行以下。

**验收标准**:
- MainWindow.cpp 行数 <= 500
- 所有功能行为与提取前完全一致
- 无槽函数遗漏或信号连接丢失
- 编译零错误

---

### R3: USB 热插拔检测 (P0)

#### R3.1 问题分析

`SerialConfigPanel::refreshPorts()` 仅在以下时机被调用:
1. 构造函数 `SerialConfigPanel()` 中调用一次
2. 用户手动点击"刷新"按钮

USB 转串口适配器插入/拔出后，端口列表不会自动更新。

#### R3.2 方案设计

**方案 A: QTimer 轮询（推荐）**

在 `SerialConfigPanel` 中使用 `QTimer` 每 2 秒轮询一次端口列表，与当前列表比较。检测到变化时自动刷新。

优点:
- 实现简单，跨平台一致
- 不依赖 Windows 原生消息
- Qt 6.8.3 无需额外配置

缺点:
- 2 秒延迟（用户可接受）
- 每 2 秒调用一次 `QSerialPortInfo::availablePorts()`（开销很小）

**方案 B: Windows 原生事件过滤**

安装 `QAbstractNativeEventFilter`，监听 `WM_DEVICECHANGE` 消息，仅在设备变化时刷新。

优点:
- 实时响应（无延迟）
- 零轮询开销

缺点:
- 平台相关代码，需要 `#ifdef Q_OS_WIN`
- 需要额外注册设备通知 (`RegisterDeviceNotification`)
- Linux/macOS 需要 udev/DARWIN 各自实现

**选定方案: A（QTimer 轮询）**

**实现细节**:

1. 在 `SerialConfigPanel` 中新增 `QTimer* m_portPollTimer`，间隔 2000ms。

2. 构造函数中启动定时器:
```cpp
m_portPollTimer = new QTimer(this);
m_portPollTimer->setInterval(2000);
connect(m_portPollTimer, &QTimer::timeout, this, &SerialConfigPanel::onPortPollTimeout);
m_portPollTimer->start();
```

3. 轮询处理方法:
```cpp
/**
 * @brief 定时轮询端口变化
 *
 * 比较当前端口列表与上次缓存的列表，仅在变化时才调用 refreshPorts()。
 * 避免频繁刷新导致用户正在操作的下拉框被重置。
 */
void SerialConfigPanel::onPortPollTimeout() {
    QStringList currentPorts;
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto& port : ports) {
        currentPorts.append(port.portName());
    }

    if (currentPorts != m_lastPortList) {
        m_lastPortList = currentPorts;
        refreshPorts();
    }
}
```

4. 新增 `QStringList m_lastPortList` 成员缓存上次端口列表，用于变化比较。

5. 连接状态为已连接时，如果检测到当前连接的端口消失，通过信号通知 ConnectionController 触发错误处理（已有的自动重连逻辑会接管）。

6. 应用关闭时，定时器由 QObject 父子树自动销毁。

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R3-01 | 插入 CH340 USB 转串口适配器 | 2 秒内端口下拉框自动出现新 COM 口 |
| TC-R3-02 | 拔出 CH340 适配器 | 2 秒内端口下拉框自动移除该 COM 口 |
| TC-R3-03 | 快速连续插入/拔出（1 秒内） | 最终状态与实际设备状态一致，无异常 |
| TC-R3-04 | 连接到 COM3，然后拔出设备 | 端口列表移除 COM3，连接状态触发错误处理 |
| TC-R3-05 | 插入两个 USB 转串口设备 | 端口列表同时出现两个新 COM 口 |
| TC-R3-06 | 未连接状态下端口变化 | 仅刷新端口列表，无其他副作用 |
| TC-R3-07 | 手动点击"刷新"按钮 | 立即刷新，不受轮询周期影响 |

**验收标准**:
- USB 设备插入/拔出后，端口列表在 3 秒内自动更新
- 端口列表更新时，如用户之前选中的端口仍然存在，保持选中
- 已连接状态下设备被拔出时，触发已有的错误处理和自动重连流程
- 轮询不导致 UI 卡顿

---

### R4: 波特率输入校验 (P0)

#### R4.1 问题分析

`SerialConfigPanel::currentBaudRate()` 实现（行 260~263）:

```cpp
int SerialConfigPanel::currentBaudRate() const {
    return m_baudCombo->currentText().toInt();
}
```

`QString::toInt()` 对非数字输入返回 0。0 波特率传给 `QSerialPort::setBaudRate(0)` 的行为:
- Windows: 触发 `UnsupportedOperationError`，连接失败
- 某些 Qt 版本: 静默使用默认 9600，用户设置的波特率被忽略

两种情况都对用户不友好，且无任何提示。

#### R4.2 方案设计

1. **输入验证器**: 在 `setupUI()` 中为 `m_baudCombo->lineEdit()` 设置 `QIntValidator`:

```cpp
m_baudCombo->setEditable(true);
auto* baudValidator = new QIntValidator(300, 12000000, this);
m_baudCombo->lineEdit()->setValidator(baudValidator);
```

2. **实时视觉反馈**: 连接 `m_baudCombo` 的 `currentTextChanged` 信号:

```cpp
/**
 * @brief 波特率输入实时校验
 *
 * 检查当前输入是否为合法波特率（正整数，范围 300~12000000）。
 * 非法值时设置 hasError 属性触发 QSS 错误样式（红色边框）。
 */
void SerialConfigPanel::onBaudRateTextChanged(const QString& text) {
    bool ok = false;
    int value = text.toInt(&ok);
    bool valid = ok && value >= 300 && value <= 12000000;

    m_baudCombo->lineEdit()->setProperty("hasError", !valid);
    m_baudCombo->lineEdit()->style()->unpolish(m_baudCombo->lineEdit());
    m_baudCombo->lineEdit()->style()->polish(m_baudCombo->lineEdit());
}
```

3. **防御性回退**: 修改 `currentBaudRate()` 方法:

```cpp
int SerialConfigPanel::currentBaudRate() const {
    bool ok = false;
    int value = m_baudCombo->currentText().toInt(&ok);
    if (!ok || value <= 0) {
        qWarning() << "Invalid baud rate input:" << m_baudCombo->currentText()
                    << ", falling back to 115200";
        return 115200;  // 安全回退值
    }
    return value;
}
```

4. **QSS 错误样式**: 在三个主题文件中添加:

```css
QLineEdit[hasError="true"] {
    border: 2px solid var(--error);
    background-color: rgba(243, 139, 168, 0.1);
}
```

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R4-01 | 输入 "abc" | QIntValidator 阻止输入，currentBaudRate() 返回 115200 |
| TC-R4-02 | 输入 "9600" | 正常接受，无错误样式 |
| TC-R4-03 | 输入 "0" | 输入框红色边框，currentBaudRate() 返回 115200 |
| TC-R4-04 | 输入 "-100" | QIntValidator 阻止负号输入 |
| TC-R4-05 | 输入 "12000001" | QIntValidator 限制最大值，输入框红色边框 |
| TC-R4-06 | 清空输入框 | 输入框红色边框，currentBaudRate() 返回 115200 |
| TC-R4-07 | 从下拉选择 "921600" | 正常切换，无错误样式 |
| TC-R4-08 | 输入 "256000" | 正常接受，currentBaudRate() 返回 256000 |
| TC-R4-09 | 输入 "115200" 后成功连接 | 连接使用 115200 波特率 |
| TC-R4-10 | 输入非法值后点击连接 | 连接使用 115200 回退值（不使用 0） |

**验收标准**:
- 波特率输入框只接受正整数（300 ~ 12000000）
- 非法输入有红色边框视觉反馈
- `currentBaudRate()` 永远不返回 0 或负数
- 自定义波特率（如 256000、500000）可正常使用
- 下拉预设值切换无任何异常

---

### R5: 快捷指令持久化 (P0)

#### R5.1 问题分析

`QuickCommandBar` 当前的持久化状态:
- `m_commands` 列表仅在内存中
- 编辑对话框 `onEditRequested()` 功能完整（添加/删除/编辑/确定/取消）
- 应用重启后所有指令丢失
- 无 `save()` / `load()` 方法

`QuickCommand` 结构体已定义:
```cpp
struct QuickCommand {
    QString name;       // 按钮上显示的名字
    QString data;       // 要发送的数据
    bool isHex = false; // 是否为HEX格式
};
```

`SettingsManager` 已提供 `get()` / `set()` 通用接口，使用 JSON 格式存储。

#### R5.2 方案设计

1. **序列化**: 将 `QList<QuickCommand>` 转换为 `QJsonArray`:

```json
{
    "serial/quickCommands": [
        {"name": "复位", "data": "AA BB CC DD", "isHex": true},
        {"name": "查询版本", "data": "AT+GMR", "isHex": false},
        {"name": "LED开", "data": "LED ON", "isHex": false}
    ]
}
```

2. **新增方法到 QuickCommandBar**:

```cpp
/**
 * @brief 将快捷指令保存到 SettingsManager
 *
 * 每次指令列表变化时自动调用（防抖 2 秒）。
 * 序列化格式: JSON 数组，每个元素包含 name/data/isHex 三个字段。
 */
void QuickCommandBar::save() {
    QJsonArray arr;
    for (const auto& cmd : m_commands) {
        QJsonObject obj;
        obj["name"] = cmd.name;
        obj["data"] = cmd.data;
        obj["isHex"] = cmd.isHex;
        arr.append(obj);
    }
    SettingsManager::instance().set("serial/quickCommands", QJsonValue(arr).toVariant());
    SettingsManager::instance().sync();
}

/**
 * @brief 从 SettingsManager 加载快捷指令
 *
 * 在 MainWindow 初始化时调用。加载后自动重建按钮栏。
 * 文件损坏或不存在时静默忽略，使用空列表。
 */
void QuickCommandBar::load() {
    QVariant data = SettingsManager::instance().get("serial/quickCommands");
    if (!data.isValid()) return;

    QJsonArray arr = QJsonValue::fromVariant(data).toArray();
    QList<QuickCommand> commands;
    for (const QJsonValue& val : arr) {
        QJsonObject obj = val.toObject();
        QuickCommand cmd;
        cmd.name = obj["name"].toString();
        cmd.data = obj["data"].toString();
        cmd.isHex = obj["isHex"].toBool(false);
        if (!cmd.name.isEmpty()) {
            commands.append(cmd);
        }
    }
    setCommands(commands);
}
```

3. **防抖保存**: 使用 `QTimer` 单次触发，2 秒内多次修改只保存一次:

```cpp
// 成员变量
QTimer* m_saveDebounceTimer;  ///< 防抖定时器

// 初始化
m_saveDebounceTimer = new QTimer(this);
m_saveDebounceTimer->setSingleShot(true);
m_saveDebounceTimer->setInterval(2000);
connect(m_saveDebounceTimer, &QTimer::timeout, this, &QuickCommandBar::save);

// 在 setCommands/addCommand/clearCommands 中触发:
m_saveDebounceTimer->start();  // 每次调用重置 2 秒倒计时
```

4. **加载时机**: 在 MainWindow 构造函数中，`QuickCommandBar` 创建后立即调用:

```cpp
// MainWindow::setupUI() 中
m_panelManager->quickCmdBar()->load();
```

5. **编辑对话框保存**: `onEditRequested()` 确认后已调用 `setCommands()`，setCommands 触发防抖保存，无需额外代码。

6. **首次启动默认指令**: 首次启动时 `load()` 读取到空列表，提供 3 条默认指令作为示例:

```cpp
void QuickCommandBar::load() {
    // ... 加载逻辑 ...
    if (commands.isEmpty()) {
        // 首次启动: 提供默认指令示例
        commands.append({tr("HELLO"), "Hello World", false});
        commands.append({tr("LED"), "LED ON", false});
        commands.append({tr("RESET"), "AA BB CC DD", true});
        setCommands(commands);
    }
}
```

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R5-01 | 添加指令 "复位" → 关闭 → 重新打开 | 指令按钮 "复位" 仍在 |
| TC-R5-02 | 编辑对话框中删除全部指令 → 保存 → 关闭 → 重新打开 | 无指令按钮 |
| TC-R5-03 | 添加 10 条指令 → 关闭 → 重新打开 | 10 条指令完整保留 |
| TC-R5-04 | 添加 HEX 指令 "AA BB" → 关闭 → 重新打开 → 点击按钮 | 发送 HEX 数据 0xAA 0xBB |
| TC-R5-05 | 首次安装启动 | 显示 3 条默认指令 |
| TC-R5-06 | 编辑对话框修改已有指令的名称 → 保存 | 按钮文字同步更新，重启后保留 |
| TC-R5-07 | settings 文件被手动清空 | 应用正常启动，显示默认指令 |
| TC-R5-08 | 快速连续添加 5 条指令 | 防抖生效，仅保存一次 |

**验收标准**:
- 快捷指令在应用重启后完整保留（名称、数据、HEX 标记）
- 编辑对话框确认后自动保存
- 首次启动提供默认指令示例
- 数据损坏时优雅降级（空列表或默认指令）
- 保存操作不阻塞 UI

---

## 接口设计

### 新增文件

| 文件 | 内容 |
|------|------|
| `terminal/TerminalSearchEngine.h` | 搜索引擎类声明（~70 行） |
| `terminal/TerminalSearchEngine.cpp` | 搜索引擎实现（~170 行） |

### 修改文件

| 文件 | 修改内容 |
|------|---------|
| `terminal/TerminalWidget.h` | 移除搜索相关成员/方法，新增 TerminalSearchEngine 指针 |
| `terminal/TerminalWidget.cpp` | 移除搜索实现（~170 行），委托给 TerminalSearchEngine |
| `core/MainWindow.h` | 移除部分槽函数声明，新增 connectSignals 子方法声明 |
| `core/MainWindow.cpp` | 提取导出/搜索/状态栏逻辑，拆分 connectSignals |
| `serial/SerialConfigPanel.h` | 新增轮询定时器、端口缓存、波特率校验声明 |
| `serial/SerialConfigPanel.cpp` | 新增热插拔轮询、波特率校验实现 |
| `serial/QuickCommandBar.h` | 新增 save/load/防抖定时器声明 |
| `serial/QuickCommandBar.cpp` | 新增 save/load 实现，指令变更触发防抖保存 |
| `utils/DataExporter.h/cpp` | 新增 exportWithDialog() 静态方法 |
| `resources/themes/dark_terminal.qss` | 波特率错误样式 |
| `resources/themes/modern_dark.qss` | 波特率错误样式 |
| `resources/themes/light.qss` | 波特率错误样式 |

---

## 依赖的公共组件

| 组件 | 用途 |
|------|------|
| `SettingsManager` | 快捷指令持久化存储（get/set/sync） |
| `HexConverter` | 搜索引擎 HEX 模式的字节转换 |
| `TerminalModel` | 搜索引擎 HEX 模式的原始数据访问 |
| `DirectionFilter` | 搜索引擎过滤模式下的行索引映射 |

---

## 设计模式

| 模式 | 应用场景 |
|------|---------|
| 单一职责原则 (SRP) | TerminalWidget 拆分为渲染 + 搜索两个职责 |
| 委托模式 | TerminalWidget 将搜索委托给 TerminalSearchEngine |
| 防抖模式 | 快捷指令持久化写操作的节流 |
| 观察者模式 | 热插拔检测通过端口变化触发 refreshPorts() |

---

## 影响范围

### 功能影响分析

| 功能 | 影响 | 风险 |
|------|------|------|
| 终端搜索 | 拆分后行为不变 | 中 - 需确保信号/槽连接正确迁移 |
| 终端渲染 | 不受影响 | 低 - paintLine 仅改为从 m_searchEngine 读取匹配数据 |
| 快捷指令发送 | 不受影响 | 低 - commandTriggered 信号路径不变 |
| 串口连接 | 波特率校验增加防御 | 低 - 只影响非法输入的处理 |
| 端口刷新 | 新增自动刷新 | 低 - 手动刷新逻辑不变，自动刷新仅新增触发源 |

### 编译影响

- CMakeLists.txt: 需新增 `terminal/TerminalSearchEngine.h/cpp`
- 无第三方依赖变化
- 无新增 Qt 模块依赖

---

## 验收标准总表

### P0 验收（必须全部通过才能 commit）

| 编号 | 验收项 | 通过条件 |
|------|--------|---------|
| AC-P0-01 | TerminalWidget.cpp 行数 | <= 500 行 |
| AC-P0-02 | MainWindow.cpp 行数 | <= 500 行 |
| AC-P0-03 | 搜索功能完整性 | 纯文本/正则/HEX 搜索行为与拆分前完全一致 |
| AC-P0-04 | 搜索导航 | F3/Shift+F3 正常工作，匹配计数显示正确 |
| AC-P0-05 | 方向过滤搜索 | 过滤模式下搜索高亮和导航正常 |
| AC-P0-06 | USB 热插拔 | 插入/拔出设备后 3 秒内端口列表自动更新 |
| AC-P0-07 | 连接中断开检测 | 已连接设备拔出后触发错误处理 |
| AC-P0-08 | 波特率校验 | 非法输入有红色边框，currentBaudRate() 永不返回 0 |
| AC-P0-09 | 波特率连接 | 非法输入后连接使用 115200 回退值而非 0 |
| AC-P0-10 | 快捷指令持久化 | 重启后指令完整保留（名称/数据/HEX 标记） |
| AC-P0-11 | 快捷指令编辑 | 编辑对话框确认后自动保存到磁盘 |
| AC-P0-12 | 编译零错误 | cmake --build build 无任何错误 |

---

## 实现优先级排序

按依赖关系排序:

```
第一批（无依赖，可并行）:
  R1 TerminalWidget 拆分 ─── 90 分钟 (涉及搜索逻辑提取)
  R4 波特率输入校验 ──────── 30 分钟 (独立修改 SerialConfigPanel)
  R5 快捷指令持久化 ──────── 45 分钟 (独立修改 QuickCommandBar)

第二批（依赖第一批完成）:
  R3 USB 热插拔检测 ──────── 45 分钟 (修改 SerialConfigPanel，需与 R4 合并测试)
  R2 MainWindow 瘦身 ────── 60 分钟 (依赖导出提取和信号连接重构)
```

**预估总工作量**: 约 4.5 小时

---

## 风险与缓解

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| TerminalSearchEngine 拆分后信号连接遗漏 | 搜索功能异常 | 拆分后逐项执行搜索测试用例（文本/正则/HEX + 过滤模式） |
| MainWindow 瘦身时信号连接断裂 | 功能失效 | 瘦身后逐项验证所有面板切换/搜索/导出/统计功能 |
| 热插拔轮询与手动刷新冲突 | 端口列表闪烁 | 轮询使用变化比较（m_lastPortList），无变化不调用 refreshPorts() |
| 快捷指令 JSON 序列化兼容性 | 旧版本数据无法加载 | load() 对每个字段使用默认值，缺失字段不崩溃 |
| 波特率 QIntValidator 与可编辑 ComboBox 冲突 | 下拉选择被拦截 | QIntValidator 仅作用于 lineEdit()，下拉预设值的 setCurrentText 不受影响 |
