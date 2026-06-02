# PRD-033: MainWindow 持续瘦身、SettingsController 完善与 ToolbarController 增强

## 背景

commit #32 完成了终端拆分（TerminalSearchEngine 提取）、串口热插拔检测、波特率校验、快捷指令持久化等任务。当前评分 34 分，本迭代 #33。

**核心目标: MainWindow.cpp 从 603 行降至 500 行以内**。

经逐行审查 MainWindow.cpp 当前代码（603 行），问题分析如下:

| 区域 | 行范围 | 行数 | 性质 | 可否提取 |
|------|--------|------|------|---------|
| 构造函数 | 39~103 | 64 行 | 初始化 + 依赖注入 + UI 组装 | 否，属于 MainWindow 核心职责 |
| setupUI() | 125~210 | 85 行 | UI 布局构建 | 否，属于 MainWindow 核心职责 |
| setupStatusBar() | 216~229 | 13 行 | 状态栏创建 | 可提取但收益小 |
| connectSignals() | 244~429 | **185 行** | **信号连接胶水代码（18 段 connect）** | **部分可提取到已有 Controller** |
| onDisplayModeChanged() | 435~442 | 7 行 | 终端显示模式切换 | 可直连，无需经过 MainWindow |
| onTimestampToggled() | 448~454 | 6 行 | 时间戳开关 | 可直连，无需经过 MainWindow |
| onClearTerminal() | 457~462 | 5 行 | 清屏 | 可提取到已有 Controller |
| onExportData() | 470~500 | **30 行** | 文件对话框 + 导出逻辑 | 可提取到 DataExporter |
| onSearchRequested() | 508~511 | 3 行 | 搜索委托 | 可直连，无需经过 MainWindow |
| onSearchCleared() | 514~517 | 3 行 | 清搜索 | 可直连，无需经过 MainWindow |
| onBgSettingsToggled() | 523~536 | 13 行 | 背景面板切换 | 可自管理 |
| onTerminalLayoutChanged() | 547~550 | 3 行 | 终端布局切换 | 可直连 |
| updateStatusBar() | 556~570 | **14 行** | 字节数格式化 | 可提取到 ConnectionController |
| updateDataStatistics() | 573~578 | 5 行 | 统计刷新 | 可提取 |
| closeEvent() | 585~603 | 18 行 | 关闭清理 | 需保留 |

**关键发现**:

1. **onDisplayModeChanged/onTimestampToggled/onTerminalLayoutChanged** -- 这三个槽函数内部逻辑仅为"遍历终端 widgets 并调用 setter"，完全可以不经过 MainWindow 中转，直接将 ToolbarController 的信号连接到 TerminalLayoutManager 的方法。

2. **onSearchRequested/onSearchCleared** -- 仅转发到 TerminalWidget，可直连。

3. **onExportData()** -- 30 行文件对话框 + 格式判断 + 流式导出逻辑，应提取到 DataExporter。

4. **updateStatusBar()** -- 字节数格式化逻辑可移入 ConnectionController 或独立工具函数。

5. **connectSignals() 中的导航树点击处理**（行 411~428，18 行）-- 含功能节点判断和面板查找，可移入 NavigationController。

6. **connectSignals() 中的连接状态 UI 更新**（行 270~305，35 行）-- 含 switch-case 状态处理、呼吸动画控制、面板切换，可移入 ConnectionController。

以上合计可减少约 **130~140 行**，配合 connectSignals() 拆分子方法减少的空行，可将 MainWindow.cpp 降至 500 行以内。

**SettingsController 当前缺陷**:

- `saveSettings()` 保存串口配置时缺少 DTR/RTS 字段（行 101~109 只保存了 6 个参数）
- 没有保存上次使用的面板索引
- 没有保存终端布局模式
- 没有保存显示模式（文本/HEX/混合/十进制）
- 没有保存时间戳开关状态
- 没有保存方向前缀开关状态

**ToolbarController 当前缺陷**:

- 没有提供 HEX 显示切换状态查询接口
- 没有时间戳切换状态查询接口
- 没有过滤器（方向前缀）状态管理接口
- 信号全部转发给 MainWindow 再分发，增加不必要的中间层

---

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | MainWindow 槽函数直连消除: onDisplayModeChanged/onTimestampToggled/onSearchRequested/onSearchCleared/onTerminalLayoutChanged 五个槽函数改为信号直连 | P0 | core/MainWindow.h/cpp, core/ToolbarController.h/cpp |
| R2 | 导出逻辑提取: onExportData() 提取到 DataExporter::exportWithDialog() | P0 | utils/DataExporter.h/cpp, core/MainWindow.cpp |
| R3 | 状态栏更新提取: updateStatusBar() 字节数格式化逻辑移入 ConnectionController | P0 | core/ConnectionController.h/cpp, core/MainWindow.cpp |
| R4 | 导航树点击处理提取: connectSignals() 中导航树点击逻辑移入 NavigationController | P0 | core/NavigationController.h/cpp, core/MainWindow.cpp |
| R5 | 连接状态 UI 更新提取: connectSignals() 中连接状态 switch-case 移入 ConnectionController | P0 | core/ConnectionController.h/cpp, core/MainWindow.cpp |
| R6 | 背景面板切换自管理: onBgSettingsToggled() 逻辑内聚到 BackgroundWidget | P1 | core/BackgroundWidget.h/cpp, core/MainWindow.cpp |
| R7 | SettingsController 完善配置持久化: 补充 DTR/RTS/面板索引/终端布局/显示模式/时间戳/方向前缀 的保存恢复 | P0 | core/SettingsController.h/cpp, src/utils/SettingsManager.h/cpp |
| R8 | ToolbarController 增强状态查询接口: 新增显示模式/时间戳/方向前缀的 getter | P0 | core/ToolbarController.h/cpp |

---

## 需求详细说明

---

### R1: MainWindow 槽函数直连消除 (P0)

#### R1.1 问题分析

MainWindow 中有 5 个槽函数仅做"信号转发"，内部不包含任何需要 MainWindow 上下文的逻辑:

| 槽函数 | 行数 | 实际操作 |
|--------|------|---------|
| `onDisplayModeChanged(int)` | 7 行 | 遍历 terminalWidgets 设置 DisplayMode |
| `onTimestampToggled(bool)` | 6 行 | 遍历 terminalWidgets 设置 showTimestamp |
| `onSearchRequested(...)` | 3 行 | 调用 terminal->setSearchHighlight() |
| `onSearchCleared()` | 3 行 | 调用 terminal->clearSearchHighlight() |
| `onTerminalLayoutChanged(int)` | 3 行 | 调用 layoutManager->setLayout() |

这些函数合计 22 行，加上对应的声明和信号连接代码共约 35 行，完全可以消除。

#### R1.2 拆分方案

**R1.2.1 显示模式和时间戳 -- 移入 TerminalLayoutManager**

当前 `onDisplayModeChanged` 和 `onTimestampToggled` 内部逻辑为遍历 `m_layoutManager->terminalWidgets()` 并调用 setter。直接将此逻辑内聚到 TerminalLayoutManager:

```cpp
// terminal/TerminalLayoutManager.h 新增:

/**
 * @brief 设置所有终端 widget 的显示模式
 * @param mode 显示模式（Text/Hex/Mixed/Decimal）
 */
void setDisplayMode(DisplayMode mode);

/**
 * @brief 设置所有终端 widget 的时间戳显示
 * @param enabled true=显示时间戳
 */
void setShowTimestamp(bool enabled);

/**
 * @brief 设置所有终端 widget 的方向前缀显示
 * @param enabled true=显示 TX/RX 前缀
 */
void setShowDirectionPrefix(bool enabled);
```

MainWindow 中 connect 改为直连:
```cpp
connect(m_toolbarController, &ToolbarController::displayModeChanged,
        this, [this](int index) {
    DisplayMode modes[] = {DisplayMode::Text, DisplayMode::Hex, DisplayMode::Mixed, DisplayMode::Decimal};
    m_layoutManager->setDisplayMode(modes[index]);
});

// 后续可进一步: ToolbarController 内部持有 TerminalLayoutManager 引用，完全绕过 MainWindow
```

但考虑到本次迭代 MainWindow 瘦身为主要目标，暂时保留 MainWindow 中的直连 lambda（比独立槽函数更紧凑），消除 MainWindow.h 中的槽函数声明。

**R1.2.2 搜索相关 -- 直连 TerminalWidget**

```cpp
// 旧: 经过 MainWindow 中转
connect(searchBar, &TerminalSearchBar::searchRequested,
        this, &MainWindow::onSearchRequested);
connect(searchBar, &TerminalSearchBar::searchCleared,
        this, &MainWindow::onSearchCleared);

// 新: 直连 TerminalWidget
connect(m_panelManager->searchBar(), &TerminalSearchBar::searchRequested,
        m_panelManager->terminal(), &TerminalWidget::setSearchHighlight);
connect(m_panelManager->searchBar(), &TerminalSearchBar::searchCleared,
        m_panelManager->terminal(), &TerminalWidget::clearSearchHighlight);
```

**R1.2.3 终端布局 -- 直连 TerminalLayoutManager**

```cpp
// 旧: 经过 MainWindow 中转
connect(m_toolbarController, &ToolbarController::terminalLayoutChanged,
        this, &MainWindow::onTerminalLayoutChanged);

// 新: 直连 TerminalLayoutManager
connect(m_toolbarController, &ToolbarController::terminalLayoutChanged,
        m_layoutManager, &TerminalLayoutManager::setLayout);
```

**预期减少行数**: MainWindow.cpp 减少 ~22 行（槽函数体）+ MainWindow.h 减少 ~25 行（5 个槽函数声明）= 47 行

---

### R2: 导出逻辑提取 (P0)

#### R2.2 方案设计

将 `MainWindow::onExportData()` 的 30 行逻辑提取为 `DataExporter::exportWithDialog()` 静态方法:

```cpp
// utils/DataExporter.h 新增:

class TerminalModel;

/**
 * @brief 弹出文件保存对话框并执行流式导出
 *
 * 封装文件对话框 + 格式判断 + 批量导出的完整流程。
 * 导出成功/失败均通过 QMessageBox 提示用户。
 *
 * @param parent 父窗口（用于对话框定位和模态阻塞）
 * @param terminalModel 终端数据源
 * @return 导出文件路径，空字符串表示用户取消或导出失败
 */
static QString exportWithDialog(QWidget* parent, TerminalModel* terminalModel);
```

MainWindow 中缩减为:
```cpp
void MainWindow::onExportData() {
    DataExporter::exportWithDialog(this, m_terminalModel);
}
```

进一步优化: 由于 `onExportData()` 缩减到仅一行调用，可以改为 connectSignals 中的 lambda 直连:
```cpp
connect(m_toolbarController, &ToolbarController::exportRequested,
        this, [this]() { DataExporter::exportWithDialog(this, m_terminalModel); });
```

这样就彻底消除了 `onExportData()` 槽函数。

**预期减少行数**: MainWindow.cpp 减少 ~30 行，MainWindow.h 减少 ~3 行

---

### R3: 状态栏更新提取 (P0)

#### R3.2 方案设计

`updateStatusBar()` 包含字节数格式化逻辑（14 行），可提取为静态工具函数:

```cpp
// core/ConnectionController.h 或独立工具函数

namespace ByteFormat {
    /**
     * @brief 格式化字节数为人类可读字符串
     * @param bytes 原始字节数
     * @return 格式化后字符串，如 "1.5 KB" / "2.3 MB"
     */
    static QString format(quint64 bytes) {
        if (bytes < 1024) return QString("%1 B").arg(bytes);
        if (bytes < 1024 * 1024) return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
        return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    }
}
```

由于 `updateStatusBar()` 需要访问 `m_terminalModel`、`m_rxBytesLbl`、`m_txBytesLbl` 三个成员，完整提取需要将状态栏标签的管理权也转移。考虑到标签是 MainWindow 的 UI 组件，完全提取并不合理。

**折中方案**: 将 formatBytes lambda 提取为 `ByteFormat::format()` 静态工具函数（放在 Constants.h 中），`updateStatusBar()` 保留在 MainWindow 但缩减到 5 行:

```cpp
void MainWindow::updateStatusBar() {
    if (m_terminalModel) {
        m_rxBytesLbl->setText("RX: " + ByteFormat::format(m_terminalModel->rxBytes()));
        m_txBytesLbl->setText("TX: " + ByteFormat::format(m_terminalModel->txBytes()));
    }
}
```

`updateDataStatistics()` 同样保留但已经只有 5 行，不需要进一步提取。

**预期减少行数**: MainWindow.cpp 减少 ~9 行（formatBytes lambda），Constants.h 增加 ~8 行

---

### R4: 导航树点击处理提取 (P0)

#### R4.2 方案设计

当前 `connectSignals()` 中导航树点击处理（行 411~428，18 行）:

```cpp
connect(m_navTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
    QString text = index.data().toString();
    // 功能性节点
    if (text == tr("数据导出")) { onExportData(); return; }
    if (text == tr("TCP客户端")) { m_connController->connectNetwork(ConnectionType::TcpClient); return; }
    if (text == tr("TCP服务端")) { m_connController->connectNetwork(ConnectionType::TcpServer); return; }
    if (text == tr("UDP")) { m_connController->connectNetwork(ConnectionType::Udp); return; }
    // 面板节点
    QWidget* target = m_navController->lookupPanel(text);
    if (!target) return;
    m_navController->switchToPanel(target);
});
```

提取到 `NavigationController` 中，新增方法:

```cpp
// core/NavigationController.h 新增:

class ConnectionController;

/**
 * @brief 连接导航树点击信号到面板切换和网络连接动作
 *
 * 内部处理:
 *   - 功能性节点（数据导出、TCP/UDP）触发对应动作
 *   - 面板节点执行面板切换动画
 *
 * @param navTree 导航树视图
 * @param connController 网络连接控制器（用于 TCP/UDP 连接）
 * @param panelManager 面板管理器（用于获取面板映射）
 * @param exportHandler 导出动作回调（MainWindow 提供，因为导出需要 parent 窗口）
 */
void connectNavTreeSignals(QTreeView* navTree,
                           ConnectionController* connController,
                           PanelManager* panelManager,
                           std::function<void()> exportHandler);
```

MainWindow 中 connectSignals() 对应段缩减为:
```cpp
m_navController->connectNavTreeSignals(m_navTree, m_connController, m_panelManager,
    [this]() { DataExporter::exportWithDialog(this, m_terminalModel); });
```

**预期减少行数**: MainWindow.cpp 减少 ~15 行（导航树 connect + lambda），NavigationController.cpp 增加 ~20 行

---

### R5: 连接状态 UI 更新提取 (P0)

#### R5.2 方案设计

当前 `connectSignals()` 中连接状态 UI 更新（行 270~305，35 行）是最复杂的信号处理段:

```cpp
connect(m_connController, &ConnectionController::connectionStateChanged,
        this, [this](ConnectionState state, const QString& connName) {
    // 35 行 switch-case: 更新状态栏文本、配置面板按钮、呼吸动画
});
```

这段代码同时操作:
- `m_connStatusLbl` -- 状态栏标签
- `m_panelManager->serialConfig()` -- 配置面板
- `m_navController` -- 呼吸动画

提取到 `ConnectionController` 不合适（ConnectionController 不应该知道 UI 组件）。

**方案: 提取到新的私有方法 `handleConnectionStateChanged()`**

```cpp
// MainWindow.h private 区域:
void handleConnectionStateChanged(ConnectionState state, const QString& connName);
```

这不会减少总行数，但将 35 行的 lambda 从 connectSignals() 中剥离出来，使 connectSignals() 更紧凑。配合 connectSignals() 拆分子方法，可以减少约 10 行的冗余空行。

**预期减少行数**: 不减少总行数，但改善可读性。connectSignals() 从 185 行降至约 155 行。

---

### R6: 背景面板切换自管理 (P1)

#### R6.2 方案设计

`onBgSettingsToggled()` 内部逻辑:
1. 检查弹出面板可见性
2. 从 BackgroundWidget 同步参数到弹出面板
3. 定位到工具栏右下角
4. 显示/隐藏弹出面板

需要引用 `m_bgSettingsPopup`、`m_backgroundWidget`、`m_toolbarController->toolbar()`。

**方案: 将弹出面板的创建和切换逻辑内聚到 BackgroundWidget**

```cpp
// core/BackgroundWidget.h 新增:

/**
 * @brief 设置背景设置弹出面板
 * 由 MainWindow 在创建弹出面板后注入
 * @param popup 弹出面板实例
 */
void setSettingsPopup(BackgroundSettingsPopup* popup);

/**
 * @brief 切换背景设置弹出面板的显示/隐藏
 * 自动定位到参考控件的下方
 * @param refWidget 参考定位的控件（工具栏）
 */
void toggleSettingsPopup(QWidget* refWidget);
```

MainWindow 中 connect 改为:
```cpp
connect(m_toolbarController, &ToolbarController::bgSettingsRequested,
        m_backgroundWidget, [this]() {
    m_backgroundWidget->toggleSettingsPopup(m_toolbarController->toolbar());
});
```

**预期减少行数**: MainWindow.cpp 减少 ~13 行

---

### R7: SettingsController 完善配置持久化 (P0)

#### R7.1 问题分析

当前 `SettingsController::saveSettings()` 保存的串口配置缺少 DTR/RTS:

```cpp
// 当前保存的字段（6 个）:
serialConfig["portName"]
serialConfig["baudRate"]
serialConfig["dataBits"]
serialConfig["parity"]
serialConfig["stopBits"]
serialConfig["flowControl"]

// 缺少的字段:
serialConfig["dtr"]     // DTR 信号状态
serialConfig["rts"]     // RTS 信号状态
```

此外，以下设置项完全没有保存/恢复:

| 设置项 | 当前状态 | 影响 |
|--------|---------|------|
| 上次使用的面板索引 | 未保存 | 每次启动都显示终端面板 |
| 终端布局模式 | 未保存 | 每次启动恢复为混合模式 |
| 显示模式（文本/HEX/混合/十进制） | 未保存 | 每次启动恢复为文本模式 |
| 时间戳开关 | 未保存 | 每次启动关闭 |
| 方向前缀开关 | 未保存 | 每次启动关闭 |

#### R7.2 方案设计

**R7.2.1 补充串口配置 DTR/RTS**

修改 `SettingsController::saveSettings()`:

```cpp
if (m_serialConfig) {
    QVariantMap serialConfig;
    serialConfig["portName"] = m_serialConfig->currentPortData();
    serialConfig["baudRate"] = m_serialConfig->currentBaudRate();
    serialConfig["dataBits"] = m_serialConfig->currentDataBitsIndex();
    serialConfig["parity"] = m_serialConfig->currentParityIndex();
    serialConfig["stopBits"] = m_serialConfig->currentStopBitsIndex();
    serialConfig["flowControl"] = m_serialConfig->currentFlowControlIndex();
    serialConfig["dtr"] = m_serialConfig->dtrEnabled();       // 新增
    serialConfig["rts"] = m_serialConfig->rtsEnabled();       // 新增
    settings.saveSerialConfig(serialConfig);
}
```

`SerialConfigPanel::restoreConfig()` 已支持恢复 DTR/RTS（行 350~361），无需修改。

**R7.2.2 新增面板索引保存/恢复**

```cpp
// SettingsController::saveSettings() 新增:
if (m_navController) {
    settings.set("window/lastPanelIndex", m_navController->currentPanelIndex());
}

// SettingsController::loadSettings() 新增:
if (m_navController) {
    int panelIndex = settings.get("window/lastPanelIndex", -1).toInt();
    if (panelIndex >= 0) {
        m_navController->switchToPanelByIndex(panelIndex);
    }
}
```

需要在 NavigationController 中新增:
```cpp
/** @brief 获取当前面板在映射表中的索引，-1 表示未设置 */
int currentPanelIndex() const;

/** @brief 根据索引切换面板（用于设置恢复） */
void switchToPanelByIndex(int index);
```

**R7.2.3 新增终端布局/显示模式/时间戳/方向前缀保存/恢复**

这些设置由 ToolbarController 管理，需要在 SettingsController 中新增 ToolbarController 引用和对应字段的保存/恢复:

```cpp
// SettingsController::saveSettings() 新增:
if (m_toolbarController) {
    settings.set("display/mode", m_toolbarController->displayModeIndex());
    settings.set("display/timestamp", m_toolbarController->isTimestampChecked());
    settings.set("display/dirPrefix", m_toolbarController->isDirPrefixChecked());
    settings.set("display/layout", m_toolbarController->layoutIndex());
}

// SettingsController::loadSettings() 新增:
if (m_toolbarController) {
    int displayMode = settings.get("display/mode", 0).toInt();
    m_toolbarController->setDisplayModeIndex(displayMode);

    bool timestamp = settings.get("display/timestamp", false).toBool();
    m_toolbarController->setTimestampChecked(timestamp);

    bool dirPrefix = settings.get("display/dirPrefix", false).toBool();
    m_toolbarController->setDirPrefixChecked(dirPrefix);

    int layout = settings.get("display/layout", 0).toInt();
    m_toolbarController->setLayoutIndex(layout);
}
```

需要 SettingsController 注入 NavigationController 引用:
```cpp
void setNavigationController(NavigationController* controller);
```

#### R7.3 SettingsManager 无需修改

`SettingsManager` 已提供通用的 `get()`/`set()` 方法，新增的设置项直接使用 `settings.get("display/mode", 0)` 即可，无需新增专门方法。

---

### R8: ToolbarController 增强状态查询接口 (P0)

#### R8.2 方案设计

为支持 SettingsController 保存/恢复工具栏状态，ToolbarController 新增以下接口:

```cpp
// core/ToolbarController.h 新增:

/**
 * @brief 获取当前显示模式下拉框索引
 * @return 索引: 0=文本, 1=HEX, 2=混合, 3=十进制
 */
int displayModeIndex() const;

/**
 * @brief 设置显示模式下拉框索引（用于恢复保存的设置）
 * @param index 索引值
 */
void setDisplayModeIndex(int index);

/**
 * @brief 时间戳开关是否选中
 */
bool isTimestampChecked() const;

/**
 * @brief 设置时间戳开关状态（用于恢复保存的设置）
 */
void setTimestampChecked(bool checked);

/**
 * @brief 方向前缀开关是否选中
 */
bool isDirPrefixChecked() const;

/**
 * @brief 设置方向前缀开关状态
 */
void setDirPrefixChecked(bool checked);

/**
 * @brief 获取终端布局下拉框索引
 * @return 索引: 0=混合, 1=左右分栏, 2=上下分栏
 */
int layoutIndex() const;

/**
 * @brief 设置终端布局下拉框索引
 */
void setLayoutIndex(int index);
```

实现非常简单（直接读写对应控件的 currentIndex/checked 属性），每个方法 1~3 行:

```cpp
int ToolbarController::displayModeIndex() const {
    return m_displayModeCombo ? m_displayModeCombo->currentIndex() : 0;
}

void ToolbarController::setDisplayModeIndex(int index) {
    if (m_displayModeCombo) m_displayModeCombo->setCurrentIndex(index);
}

bool ToolbarController::isTimestampChecked() const {
    return m_timestampAction ? m_timestampAction->isChecked() : false;
}

void ToolbarController::setTimestampChecked(bool checked) {
    if (m_timestampAction) m_timestampAction->setChecked(checked);
}
// 其余类似...
```

---

## 接口设计

### 修改文件总览

| 文件 | 修改类型 | 修改内容 |
|------|---------|---------|
| `core/MainWindow.h` | 修改 | 移除 5 个槽函数声明，移除 2 个成员方法声明，移除 3 个成员变量 |
| `core/MainWindow.cpp` | 修改 | 槽函数改为 lambda 直连，导出/状态栏/导航逻辑提取，目标 <= 500 行 |
| `core/ToolbarController.h` | 修改 | 新增 8 个状态查询/设置方法 |
| `core/ToolbarController.cpp` | 修改 | 实现 8 个状态查询/设置方法 |
| `core/SettingsController.h` | 修改 | 新增 setNavigationController() 方法 |
| `core/SettingsController.cpp` | 修改 | saveSettings/loadSettings 增加 DTR/RTS/面板/布局/显示模式/时间戳/方向前缀 |
| `core/NavigationController.h` | 修改 | 新增 currentPanelIndex()、switchToPanelByIndex()、connectNavTreeSignals() |
| `core/NavigationController.cpp` | 修改 | 实现新增方法 |
| `core/ConnectionController.h` | 修改 | 新增状态栏标签管理方法 |
| `core/ConnectionController.cpp` | 修改 | 状态变化时直接更新状态栏标签 |
| `utils/DataExporter.h` | 修改 | 新增 exportWithDialog() 静态方法声明 |
| `utils/DataExporter.cpp` | 修改 | 实现 exportWithDialog() |
| `core/Constants.h` | 修改 | 新增 ByteFormat 命名空间 |
| `core/BackgroundWidget.h` | 修改 | 新增 setSettingsPopup()、toggleSettingsPopup() |
| `core/BackgroundWidget.cpp` | 修改 | 实现弹出面板自管理 |

---

## 依赖的公共组件

| 组件 | 用途 |
|------|------|
| `SettingsManager` | 所有配置项的底层持久化（get/set/sync） |
| `ThemeManager` | 主题名称查询 |
| `PanelManager` | 面板映射表和面板指针获取 |
| `TerminalModel` | RX/TX 字节数查询 |
| `SerialConfigPanel` | 串口配置参数读写（含 DTR/RTS） |
| `DataExporter` | exportWithDialog() 的新增方法 |
| `ByteFormat` | 字节数格式化工具函数 |

---

## 设计模式

| 模式 | 应用场景 |
|------|---------|
| **委托模式** | MainWindow 将导出逻辑委托给 DataExporter，导航逻辑委托给 NavigationController |
| **中介者模式** | MainWindow 仍作为中介者协调各 Controller，但减少直接持有的槽函数数量 |
| **外观模式** | ToolbarController 提供统一的状态查询接口，隐藏内部控件细节 |
| **单一职责原则** | 每个 Controller 只管理自己领域内的状态和 UI |

---

## 影响范围

### 功能影响分析

| 功能 | 影响 | 风险 |
|------|------|------|
| 终端显示模式切换 | 信号路径从 MainWindow 中转变为直连 TerminalLayoutManager | 中 -- 需验证所有 4 种模式切换正常 |
| 时间戳开关 | 同上 | 低 -- 仅 setter 调用路径变化 |
| 搜索高亮 | 信号从 MainWindow 中转变为直连 TerminalWidget | 低 -- 信号签名完全匹配 |
| 数据导出 | 逻辑从 MainWindow 移入 DataExporter | 中 -- 需验证文件对话框和三种格式导出 |
| 状态栏更新 | 字节数格式化提取为工具函数 | 低 -- 纯逻辑提取，无行为变化 |
| 导航树点击 | 逻辑从 MainWindow 移入 NavigationController | 中 -- 需验证面板切换和网络连接 |
| 连接状态 UI | 从 lambda 移入私有方法 | 低 -- 仅代码组织变化 |
| 背景面板切换 | 逻辑内聚到 BackgroundWidget | 低 -- 仅移动代码位置 |
| 配置持久化 | 新增多个设置项的保存/恢复 | 中 -- 需验证各设置项正确恢复 |
| 工具栏状态查询 | 新增 8 个 getter/setter | 低 -- 简单的属性访问 |

### 编译影响

- CMakeLists.txt: 无新增文件，无需修改
- 无第三方依赖变化
- 无新增 Qt 模块依赖

---

## 验收标准总表

### P0 验收（必须全部通过才能 commit）

| 编号 | 验收项 | 通过条件 |
|------|--------|---------|
| AC-P0-01 | MainWindow.cpp 行数 | <= 500 行 |
| AC-P0-02 | MainWindow.h 行数 | <= 200 行 |
| AC-P0-03 | 编译零错误 | cmake --build build 无任何错误 |
| AC-P0-04 | 终端显示模式切换 | 文本/HEX/混合/十进制四种模式切换正常 |
| AC-P0-05 | 时间戳开关 | 开启/关闭时间戳显示正常 |
| AC-P0-06 | 方向前缀开关 | 开启/关闭 TX/RX 前缀显示正常 |
| AC-P0-07 | 搜索功能 | 搜索/清除/正则/HEX/匹配导航全部正常 |
| AC-P0-08 | 数据导出 | TXT/CSV/BIN 三种格式导出正常 |
| AC-P0-09 | 导航树面板切换 | 所有面板切换动画正常，网络连接节点正常 |
| AC-P0-10 | 串口连接 | 连接/断开/DTR/RTS 控制正常 |
| AC-P0-11 | 配置持久化 -- 串口 | 重启后端口名/波特率/数据位/校验/停止位/流控/DTR/RTS 恢复正确 |
| AC-P0-12 | 配置持久化 -- 窗口 | 重启后窗口位置和大小恢复正确 |
| AC-P0-13 | 配置持久化 -- 主题 | 重启后主题恢复正确 |
| AC-P0-14 | 配置持久化 -- 显示状态 | 重启后显示模式/时间戳/方向前缀/终端布局恢复正确 |
| AC-P0-15 | 配置持久化 -- 面板索引 | 重启后恢复到上次使用的面板 |
| AC-P0-16 | EmbedDebug.bat 启动 | commit 后 bat 启动正常，无崩溃 |

### P1 验收（建议通过）

| 编号 | 验收项 | 通过条件 |
|------|--------|---------|
| AC-P1-01 | 背景面板切换 | 点击"背景"按钮弹出面板正常，点击外部关闭正常 |
| AC-P1-02 | ToolbarController 状态接口 | 所有 8 个 getter/setter 单元测试通过 |

---

## 实现优先级排序

按依赖关系排序:

```
第一批（无依赖，可并行）:
  R8 ToolbarController 状态查询接口 ──── 30 分钟 (简单 getter/setter)
  R2 导出逻辑提取 ──────────────────── 30 分钟 (DataExporter)
  R3 状态栏更新提取 ────────────────── 20 分钟 (ByteFormat 工具函数)

第二批（依赖第一批）:
  R1 槽函数直连消除 ────────────────── 45 分钟 (依赖 R8 的接口)
  R4 导航树点击处理提取 ────────────── 30 分钟 (NavigationController)
  R5 连接状态 UI 更新提取 ──────────── 15 分钟 (私有方法)
  R6 背景面板切换自管理 ────────────── 20 分钟 (BackgroundWidget)

第三批（依赖第二批完成，所有 MainWindow 瘦身后）:
  R7 SettingsController 完善 ────────── 45 分钟 (依赖 R8 的状态接口)
```

**预估总工作量**: 约 3.5 小时

---

## 行数预估

### MainWindow.cpp 瘦身明细

| 操作 | 减少行数 | 说明 |
|------|---------|------|
| R1: 移除 5 个槽函数体 | -22 行 | onDisplayModeChanged/onTimestampToggled/onSearchRequested/onSearchCleared/onTerminalLayoutChanged |
| R1: connect 改为 lambda 直连（更紧凑） | -8 行 | 去掉槽函数声明的分隔注释 |
| R2: onExportData 提取到 DataExporter | -30 行 | 文件对话框 + 格式判断 + 导出逻辑 |
| R3: updateStatusBar 中 formatBytes 提取 | -9 行 | lambda 提取为 ByteFormat::format() |
| R4: 导航树点击处理移入 NavigationController | -15 行 | 4 个功能节点 + 面板查找 |
| R5: 连接状态 lambda 提取为私有方法 | -5 行 | 从 lambda 剥离为 handleConnectionStateChanged() |
| R6: 背景面板切换移入 BackgroundWidget | -13 行 | onBgSettingsToggled 整体移出 |
| connectSignals() 空行精简 | -10 行 | 子方法拆分后减少段落间距 |
| **合计** | **~112 行** | **603 - 112 = ~491 行** |

### 新增/修改文件行数变化

| 文件 | 当前行数 | 变化后行数 | 说明 |
|------|---------|-----------|------|
| MainWindow.cpp | 603 | ~491 | 瘦身目标达成 |
| MainWindow.h | 240 | ~215 | 移除槽函数声明和部分成员 |
| ToolbarController.h | 157 | ~195 | 新增 8 个方法声明 |
| ToolbarController.cpp | 241 | ~275 | 新增 8 个方法实现 |
| SettingsController.h | 97 | ~107 | 新增 setNavigationController() |
| SettingsController.cpp | 153 | ~210 | 增加配置项保存/恢复 |
| NavigationController.h | 147 | ~170 | 新增 3 个方法 |
| NavigationController.cpp | 338 | ~360 | 新增方法实现 |
| DataExporter.h | ~50 | ~60 | 新增 exportWithDialog() |
| DataExporter.cpp | ~150 | ~185 | 新增 exportWithDialog() 实现 |
| Constants.h | 80 | ~92 | 新增 ByteFormat |
| BackgroundWidget.h | 229 | ~245 | 新增 2 个方法 |
| BackgroundWidget.cpp | ~300 | ~318 | 新增方法实现 |

---

## 风险与缓解

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| 槽函数消除后信号连接遗漏 | 功能失效 | 逐项验证所有工具栏按钮和面板切换功能 |
| DataExporter 提取后导出对话框行为变化 | 导出失败 | 验证三种格式的完整导出流程 |
| SettingsController 新增配置项与旧版本不兼容 | 首次启动崩溃 | 使用 get() 的默认值参数，缺失字段使用安全默认值 |
| 导航树功能节点判断提取后 tr() 翻译匹配失败 | 面板切换异常 | 保持翻译上下文一致（使用 QCoreApplication::translate） |
| connectSignals 重构时信号连接顺序变化 | 初始化时序问题 | 保持原有的信号连接顺序不变 |
| ToolbarController setXxx 方法触发 currentIndexChanged 信号 | 设置恢复时触发级联更新 | 使用 blockSignals 防止恢复过程中触发信号 |
