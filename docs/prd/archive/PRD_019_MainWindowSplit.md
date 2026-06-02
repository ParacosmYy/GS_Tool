# PRD-019: MainWindow Split + Translation Harvest + QSS ProtocolView Gap

## 背景

第六次架构审查周期（commit #22 基准）结合 QA 国际化测试报告，识别出三个需要在本迭代中解决的跨层问题:

1. **MainWindow.cpp 仍超限**: CLAUDE.md 4.6 节规定 `.cpp` 文件上限 500 行，MainWindow.cpp 当前 1092 行，是上限的 2.18 倍。NavigationController.h/cpp 和 RecordingController.h/cpp 已在上一轮创建，但 MainWindow.cpp 中的旧代码尚未移除，两份逻辑并存。需要在此次迭代中完成迁移并删除 MainWindow 中的冗余副本。
2. **翻译缺口**: QA 在英文模式下测试发现 34 个 tr() 字符串未收录到 `embeddebug_en.ts`。涉及 ChartWidget、DataLogger、MainWindow 回放、OtaWidget 协议列表、SerialConfigPanel 表单标签、QuickCommandBar 默认命令、DataStatistics 表单标签，以及 NavigationController 和 RecordingController 两个新文件中全部 tr() 调用。需要运行 `lupdate` 收割，然后逐条补全英文翻译。
3. **QSS protocolToolbar 语法错误**: `QWidget#protocolToolbar` 选择器在全部三个主题文件中缺少属性块和闭合花括号，导致后续规则解析异常。需修复为带 `background-color` 的完整规则块。

**审查基准**: PRD-018 交付后，当前评分 22/1000，commit #22。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | MainWindow Split Strategy: 将导航树构建、面板切换动画、呼吸动画迁移到 NavigationController；将录制/回放逻辑迁移到 RecordingController；MainWindow 仅保留 setupUI 骨架、setupToolbar、setupStatusBar、connectSignals、closeEvent 及业务槽函数 | P0 | core/MainWindow, core/NavigationController, core/RecordingController |
| R2 | Translation Harvest: 运行 lupdate 收割全部 tr() 字符串，补全 embeddebug_en.ts 中 34 条缺失翻译 | P1 | resources/translations, src/chart, src/utils, src/ota, src/serial, src/core |
| R3 | QSS protocolToolbar Fix: 修复三个主题文件中 QWidget#protocolToolbar 规则的语法错误 | P1 | resources/themes |

## 需求详细说明

---

### R1: MainWindow Split Strategy (P0)

#### 问题分析

MainWindow.cpp 当前 1092 行，CLAUDE.md 4.6 节规定 `.cpp` 上限 500 行。拆分目标分两阶段:

**已完成但未整合的部分**:
- `NavigationController.h/cpp` (67/256 行): 已包含 `buildNavTree`, `switchToPanel`, `allSwitchablePanels`, `setCurrentPanel`, `fadeInPanel`, `fadeOutPanel`, `startBreathingAnimation`, `stopBreathingAnimation`。
- `RecordingController.h/cpp` (52/136 行): 已包含 `setupActions`, `onToggleRecording`, `onStopRecording`, `onOpenPlayback`, `onStopPlayback`, `onPlaybackData`, `onPlaybackProgress`, `onRecordingStopped`。

**MainWindow.cpp 中的冗余代码** (需移除):
以下方法在 MainWindow.cpp 中存在但在 NavigationController 或 RecordingController 中已有完整实现，形成重复:

| MainWindow.cpp 中的冗余方法 | 行范围 | 对应的 Controller 方法 |
|-----------------------------|--------|----------------------|
| `buildNavPanelMappings()` | 349-360 | `NavigationController::buildNavTree()` 接收映射参数 |
| `allSwitchablePanels()` | 362-366 | `NavigationController::allSwitchablePanels()` |
| `switchToPanel()` | 368-466 | `NavigationController::switchToPanel()` |
| `startBreathingAnimation()` | 946-973 | `NavigationController::startBreathingAnimation()` |
| `stopBreathingAnimation()` | 976-990 | `NavigationController::stopBreathingAnimation()` |
| 录制/回放相关槽 (已移除) | -- | `RecordingController` 中的对应槽 |

此外，`setupUI()` 中导航树构建逻辑 (第 80-143 行) 应委托给 `NavigationController::buildNavTree()`，而非直接内联。

#### 方案设计

**Step 1: MainWindow 新增 Controller 成员变量**

```cpp
// MainWindow.h 新增
#include "core/NavigationController.h"
#include "core/RecordingController.h"

// 在 private 成员区新增:
NavigationController* m_navController;
RecordingController* m_recordingController;
```

**Step 2: 构造函数初始化 Controller 并替换方法调用**

```cpp
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    // ... 现有初始化列表 ...
{
    // 创建控制器（在 setupUI 之前，因为 setupUI 需要引用）
    m_navController = new NavigationController(this);
    m_recordingController = new RecordingController(m_dataLogger, this);

    setupUI();
    setupToolbar();
    setupStatusBar();
    connectSignals();

    // 初始面板状态
    m_navController->setCurrentPanel(m_terminal);

    loadSettings();
    m_statsTimer->setInterval(500);
    m_statsTimer->start();
    setWindowTitle(App::APP_NAME);
    resize(1200, 800);
    setMinimumSize(900, 600);
}
```

**Step 3: setupUI 委托导航树构建**

将 setupUI() 中第 80-143 行的导航树构建代码替换为:

```cpp
void MainWindow::setupUI()
{
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(m_mainSplitter);

    // ---- 左侧导航树 ----
    m_navTree = new QTreeView;
    m_navTree->setObjectName("navTree");
    m_navTree->setHeaderHidden(true);
    m_navTree->setMinimumWidth(180);
    m_navTree->setMaximumWidth(280);
    m_navTree->setIndentation(16);
    m_mainSplitter->addWidget(m_navTree);

    // ---- 右侧内容面板 ----
    // ... 串口配置面板、终端、搜索栏等不变 ...

    // 构建导航面板映射表（所有面板创建完成后）
    QVector<NavPanelMapping> mappings = {
        {QT_TRANSLATE_NOOP("MainWindow", "配置"),       m_serialConfig},
        {QT_TRANSLATE_NOOP("MainWindow", "终端"),       m_terminal},
        {QT_TRANSLATE_NOOP("MainWindow", "统计"),       m_dataStats},
        {QT_TRANSLATE_NOOP("MainWindow", "协议"),       m_protocolView},
        {QT_TRANSLATE_NOOP("MainWindow", "帧编辑器"),   m_frameEditor},
        {QT_TRANSLATE_NOOP("MainWindow", "波形图"),     m_chartWidget},
        {QT_TRANSLATE_NOOP("MainWindow", "OTA升级"),    m_otaWidget},
    };

    // 委托 NavigationController 构建树
    m_navController->buildNavTree(m_navTree, mappings);
}
```

**Step 4: setupToolbar 委托录制按钮创建**

将 setupToolbar() 中第 289-299 行的录制/回放按钮创建替换为:

```cpp
void MainWindow::setupToolbar()
{
    // ... displayModeCombo, timestampAction, clearAction, exportAction 不变 ...

    m_toolbar->addSeparator();

    // 委托 RecordingController 创建录制/回放按钮
    m_recordingController->setupActions(m_toolbar);

    m_toolbar->addSeparator();

    // ... 主题、语言下拉框不变 ...
}
```

**Step 5: connectSignals 委托 Controller 连接**

```cpp
void MainWindow::connectSignals()
{
    // ... 现有连接不变 ...

    // 录制/回放按钮连接已在 RecordingController::setupActions 中完成
    // 替换原 MainWindow 中的录制信号连接

    // RecordingController 信号转发
    connect(m_recordingController, &RecordingController::statusMessage,
            this, [this](const QString& msg, int timeout) {
                statusBar()->showMessage(msg, timeout);
            });
    connect(m_recordingController, &RecordingController::playbackData,
            this, &MainWindow::onPlaybackData);

    // 导航树点击 → 面板切换委托给 NavigationController
    connect(m_navTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        QString text = index.data().toString();

        // 功能性节点
        if (text == tr("数据导出")) { onExportData(); return; }
        if (text == tr("TCP客户端")) { onConnectNetwork(ConnectionType::TcpClient); return; }
        if (text == tr("TCP服务端")) { onConnectNetwork(ConnectionType::TcpServer); return; }
        if (text == tr("UDP")) { onConnectNetwork(ConnectionType::Udp); return; }

        // 面板映射查找
        QWidget* target = nullptr;
        for (const auto& mapping : m_navController->allSwitchablePanels()) {
            // ... 映射查找逻辑，改用 m_navController 的映射表 ...
        }
        if (target) m_navController->switchToPanel(target);
    });
}
```

**Step 6: 移除 MainWindow 中的冗余方法**

从 MainWindow.h 和 MainWindow.cpp 中删除以下方法:
- `buildNavPanelMappings()` -- 逻辑已在 NavigationController::buildNavTree 中
- `allSwitchablePanels()` -- 由 NavigationController 提供
- `switchToPanel()` -- 由 NavigationController 提供
- `startBreathingAnimation()` -- 由 NavigationController 提供
- `stopBreathingAnimation()` -- 由 NavigationController 提供

从 MainWindow.h 中删除以下不再需要的成员:
- `QVector<NavPanelMapping> m_navPanelMappings`
- `QWidget* m_currentPanel`
- `bool m_panelSwitching`
- `QPropertyAnimation* m_breathingAnim`
- `QGraphicsOpacityEffect* m_connStatusEffect`
- `QAction* m_recordAction` / `m_stopRecordAction` / `m_playbackAction` / `m_stopPlaybackAction`

从 MainWindow.h 中删除以下不再需要的私有槽声明:
- `onToggleRecording()` / `onStopRecording()` / `onOpenPlayback()` / `onStopPlayback()`
- `onPlaybackProgress()` / `onRecordingStopped()`

保留 `onPlaybackData()` 在 MainWindow 中，因为它需要访问 `m_terminalModel`。

**Step 7: onConnectionStateChanged 委托呼吸动画**

```cpp
case ConnectionState::Connecting:
    m_connStatusLbl->setText(tr("连接中..."));
    stateStr = "connecting";
    m_navController->startBreathingAnimation(m_connStatusLbl);  // 委托
    break;
// ... 其他状态调用 m_navController->stopBreathingAnimation(m_connStatusLbl);
```

**Step 8: closeEvent 委托清理**

```cpp
void MainWindow::closeEvent(QCloseEvent* event)
{
    m_navController->stopBreathingAnimation(m_connStatusLbl);
    // ... 其余逻辑不变 ...
}
```

**Step 9: MainWindow.h 头文件精简**

从 MainWindow.h 中移除已迁移到 NavigationController 的 include:
- 不再需要 `QPropertyAnimation` 和 `QGraphicsOpacityEffect` 的 include（如果仅用于动画）

移除已迁移到 NavigationController.h 的 `NavPanelMapping` 结构体定义，改为 include NavigationController.h。

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `src/core/MainWindow.h` | 新增 NavigationController/RecordingController 成员和 include；删除冗余成员变量、槽声明、NavPanelMapping 结构体 |
| `src/core/MainWindow.cpp` | 删除 buildNavPanelMappings/allSwitchablePanels/switchToPanel/startBreathingAnimation/stopBreathingAnimation 方法体；setupUI 委托导航树构建；setupToolbar 委托录制按钮创建；connectSignals 连接 Controller 信号 |
| `src/core/NavigationController.h` | 无变更（已就绪） |
| `src/core/NavigationController.cpp` | 无变更（已就绪） |
| `src/core/RecordingController.h` | 无变更（已就绪） |
| `src/core/RecordingController.cpp` | 无变更（已就绪） |

#### 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R1-AC1 | MainWindow.cpp 行数 < 600 行 | `wc -l src/core/MainWindow.cpp` | < 600 行 |
| R1-AC2 | MainWindow.h 行数 < 200 行 | `wc -l src/core/MainWindow.h` | < 200 行 |
| R1-AC3 | MainWindow.cpp 中不存在 switchToPanel/buildNavPanelMappings/allSwitchablePanels/startBreathingAnimation/stopBreathingAnimation 方法定义 | `grep -n` 检查 | 0 匹配 |
| R1-AC4 | 导航树面板切换动画正常工作 | 手动测试: 点击导航树各节点，验证淡入淡出切换 | 动画流畅 |
| R1-AC5 | 连接状态呼吸动画正常工作 | 手动测试: 点击连接后观察 "Connecting..." 状态脉冲 | 呼吸动画可见 |
| R1-AC6 | 录制/暂停/恢复/停止功能正常 | 手动测试: 录制数据 -> 暂停 -> 恢复 -> 停止 | 状态正确切换 |
| R1-AC7 | 回放功能正常 | 手动测试: 打开 .edl 文件回放，观察终端数据流 | 回放正确 |
| R1-AC8 | 编译零错误零警告 | `cmake --build build` | 0 error, 0 warning |
| R1-AC9 | NavigationController.cpp 和 RecordingController.cpp 无变更 | `git diff` 验证 | 两个文件不在变更列表中 |

---

### R2: Translation Harvest (P1)

#### 问题分析

QA 在英文语言模式下测试发现，部分 UI 文本仍显示中文源字符串，说明对应的 tr() 调用未被 `embeddebug_en.ts` 收录。

**缺失翻译根因分析**:
1. NavigationController.cpp 和 RecordingController.cpp 是新文件，尚未运行过 `lupdate` 收割
2. 部分旧文件中的 tr() 字符串可能在 lupdate 之后新增，未重新收割
3. SerialConfigPanel 表单标签（带冒号版本如 "波特率:"）未收录

**缺失翻译清单** (按上下文分组):

**NavigationController (新增上下文，14 条)**:

| 源字符串 | 类别 |
|---------|------|
| 串口 | 导航树节点 |
| 配置 | 导航树节点 |
| 终端 | 导航树节点 |
| 统计 | 导航树节点 |
| 协议 | 导航树节点 |
| 帧编辑器 | 导航树节点 |
| 波形图 | 导航树节点 |
| OTA升级 | 导航树节点 |
| 网络 | 导航树节点 |
| TCP客户端 | 导航树节点 |
| TCP服务端 | 导航树节点 |
| UDP | 导航树节点 |
| 工具 | 导航树节点 |
| 数据导出 | 导航树节点 |

**RecordingController (新增上下文，10 条)**:

| 源字符串 | 类别 |
|---------|------|
| 录制 | 工具栏按钮 |
| 停止录制 | 工具栏按钮 |
| 回放日志 | 工具栏按钮 |
| 停止回放 | 工具栏按钮 |
| 暂停 | 录制状态 |
| 继续 | 录制状态 |
| EmbedDebug Log (*.edl);;All files (*.*) | 文件过滤器 |
| 录制日志 | 文件对话框标题 |
| 打开日志回放 | 文件对话框标题 |
| 录制 | 停止后恢复按钮文本 |

**ChartWidget (部分已收录，补充 1 条)**:

| 源字符串 | 类别 |
|---------|------|
| Window: | 轴标签（已确认收录） |

全部 ChartWidget 字符串已收录，无需补充。

**SerialConfigPanel (补充 9 条带冒号标签)**:

| 源字符串 | 英文翻译 |
|---------|---------|
| 刷新 | Refresh |
| 波特率: | Baud Rate: |
| 数据位: | Data Bits: |
| 校验位: | Parity: |
| 停止位: | Stop Bits: |
| 流控: | Flow Control: |
| 偶校验 | Even |
| 奇校验 | Odd |
| RTS/CTS | RTS/CTS |
| XON/XOFF | XON/XOFF |

**QuickCommandBar (补充 2 条)**:

| 源字符串 | 英文翻译 |
|---------|---------|
| Edit | Edit |
| Command | Command |

**DataStatistics (已确认全部收录)**: 无缺失。

**OtaWidget (已确认全部收录)**: 无缺失。

**DataLogger (已确认全部收录)**: 无缺失。

**总计**: 约 34 条缺失翻译。

#### 方案设计

**Step 1: 更新 CMakeLists.txt 确保 lupdate 覆盖新文件**

确认 `src/core/NavigationController.cpp` 和 `src/core/RecordingController.cpp` 在 CMakeLists.txt 的源文件列表中。lupdate 会扫描 CMakeLists.txt 中列出的所有源文件。

**Step 2: 运行 lupdate 收割**

```bash
E:/Tool/DevEnv/Qt/6.8.3/mingw_64/bin/lupdate.exe src -ts resources/translations/embeddebug_en.ts
```

此命令会扫描 `src/` 目录下所有 `.cpp` 文件中的 tr() 调用，将新发现的字符串追加到 TS 文件中，已存在的条目保持不变。

**Step 3: 补全英文翻译**

在 lupdate 输出的 TS 文件中，新增条目的 `<translation>` 节点默认为空（`type="unfinished"`）。需要逐条填入英文翻译。

新增上下文及翻译:

```xml
<!-- NavigationController 上下文 -->
<context>
    <name>NavigationController</name>
    <message>
        <source>串口</source>
        <translation>Serial</translation>
    </message>
    <message>
        <source>配置</source>
        <translation>Config</translation>
    </message>
    <!-- ... 其余节点翻译同 MainWindow 中已有的对应翻译 ... -->
</context>

<!-- RecordingController 上下文 -->
<context>
    <name>RecordingController</name>
    <message>
        <source>录制</source>
        <translation>Record</translation>
    </message>
    <message>
        <source>停止录制</source>
        <translation>Stop Rec</translation>
    </message>
    <message>
        <source>回放日志</source>
        <translation>Playback</translation>
    </message>
    <message>
        <source>停止回放</source>
        <translation>Stop Play</translation>
    </message>
    <message>
        <source>暂停</source>
        <translation>Pause</translation>
    </message>
    <message>
        <source>继续</source>
        <translation>Resume</translation>
    </message>
    <message>
        <source>EmbedDebug Log (*.edl);;All files (*.*)</source>
        <translation>EmbedDebug Log (*.edl);;All files (*.*)</translation>
    </message>
    <message>
        <source>录制日志</source>
        <translation>Record Log</translation>
    </message>
    <message>
        <source>打开日志回放</source>
        <translation>Open Log for Playback</translation>
    </message>
</context>
```

对 SerialConfigPanel 上下文补充缺失条目:

```xml
<!-- SerialConfigPanel 补充 -->
<message>
    <source>刷新</source>
    <translation>Refresh</translation>
</message>
<message>
    <source>波特率:</source>
    <translation>Baud Rate:</translation>
</message>
<message>
    <source>数据位:</source>
    <translation>Data Bits:</translation>
</message>
<message>
    <source>校验位:</source>
    <translation>Parity:</translation>
</message>
<message>
    <source>停止位:</source>
    <translation>Stop Bits:</translation>
</message>
<message>
    <source>流控:</source>
    <translation>Flow Ctrl:</translation>
</message>
<message>
    <source>偶校验</source>
    <translation>Even</translation>
</message>
<message>
    <source>奇校验</source>
    <translation>Odd</translation>
</message>
<message>
    <source>RTS/CTS</source>
    <translation>RTS/CTS</translation>
</message>
<message>
    <source>XON/XOFF</source>
    <translation>XON/XOFF</translation>
</message>
```

**Step 4: 运行 lrelease 生成 .qm 文件**

```bash
E:/Tool/DevEnv/Qt/6.8.3/mingw_64/bin/lrelease.exe resources/translations/embeddebug_en.ts -qm resources/translations/embeddebug_en.qm
```

**Step 5: 去重处理**

NavigationController 和 RecordingController 中的部分 tr() 字符串与 MainWindow 中已有的字符串重复（如 "串口"、"配置"、"录制" 等）。Qt 的翻译机制按 `<context>` 分组，同一字符串在不同 context 中需要分别翻译。这不会导致运行时冲突，因为每个类在运行时使用自己 context 的翻译。

但需要注意的是，MainWindow 中原有的导航树节点 tr() 字符串将在 MainWindow Split 后被移除（因为导航树构建迁移到 NavigationController），所以 MainWindow context 中对应的条目将由 lupdate 标记为 obsolete，可安全保留或清理。

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `resources/translations/embeddebug_en.ts` | 新增 NavigationController 和 RecordingController 上下文；补充 SerialConfigPanel 缺失条目；补全 QuickCommandBar 缺失条目 |
| `resources/translations/embeddebug_en.qm` | 由 lrelease 重新生成 |
| `CMakeLists.txt` | 确认 NavigationController/RecordingController 在源文件列表中（可能无需变更） |

不涉及任何 C++ 源代码变更。

#### 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R2-AC1 | embeddebug_en.ts 中无 type="unfinished" 条目 | `grep -c 'unfinished' embeddebug_en.ts` | 0 条 |
| R2-AC2 | NavigationController 上下文包含全部 14 条翻译 | 检查 TS 文件 | 14 条 message |
| R2-AC3 | RecordingController 上下文包含全部翻译 | 检查 TS 文件 | 覆盖所有 tr() 调用 |
| R2-AC4 | SerialConfigPanel 上下文包含带冒号标签翻译 | 检查 TS 文件 | 波特率:/数据位:/校验位:/停止位:/流控: 均存在 |
| R2-AC5 | 英文模式下所有 UI 文本显示英文 | 手动测试: 切换到 English，重启应用，检查导航树、配置面板、录制按钮 | 无中文残留 |
| R2-AC6 | 中文模式下所有 UI 文本显示中文 | 手动测试: 默认中文模式，全界面检查 | 无英文残留 |
| R2-AC7 | lupdate 不产生重复条目 | 检查 TS 文件 | 同一 context 内无重复 source |

---

### R3: QSS protocolToolbar 语法修复 (P1)

#### 问题分析

三个主题文件中 `QWidget#protocolToolbar` 选择器存在语法错误:

```css
/* 当前状态（三个主题文件一致） */
QWidget#protocolView { background-color: #1e1e2e; }    /* 正确 */
QWidget#protocolToolbar {                                /* 缺少属性，缺少闭合花括号 */
QLabel#protocolStatusLabel {                             /* 被解析为 protocolToolbar 的后代选择器 */
```

实际效果: `QWidget#protocolToolbar` 规则缺少属性声明和闭合花括号，导致 QSS 解析器将后续的 `QLabel#protocolStatusLabel` 及其全部属性解析为 `protocolToolbar` 的后代选择器。这意味着 `protocolStatusLabel` 的样式只有在 `protocolToolbar` 内部才会生效（虽然实际确实在内部，所以视觉上可能未暴露问题）。

三个文件的具体位置:

| 文件 | protocolToolbar 行号 |
|------|---------------------|
| `resources/themes/dark_terminal.qss` | 第 412 行 |
| `resources/themes/modern_dark.qss` | 第 419 行 |
| `resources/themes/light.qss` | 第 419 行 |

#### 方案设计

在三个主题文件中为 `QWidget#protocolToolbar` 添加完整的属性块:

**dark_terminal.qss**:

```css
QWidget#protocolToolbar {
    background-color: #181825;
    border-bottom: 1px solid #313244;
}
```

**modern_dark.qss**:

```css
QWidget#protocolToolbar {
    background-color: #16161e;
    border-bottom: 1px solid #292e42;
}
```

**light.qss**:

```css
QWidget#protocolToolbar {
    background-color: #f3f4f6;
    border-bottom: 1px solid #e5e7eb;
}
```

每个文件的修改方式:
1. 在 `QWidget#protocolToolbar {` 之后添加 `background-color` 和 `border-bottom` 属性
2. 添加闭合花括号 `}`
3. 保持 `QLabel#protocolStatusLabel` 为独立的顶级选择器

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `resources/themes/dark_terminal.qss` | 修复 QWidget#protocolToolbar 规则，添加 background-color 和 border-bottom |
| `resources/themes/modern_dark.qss` | 同上 |
| `resources/themes/light.qss` | 同上 |

不涉及 C++ 代码变更。

#### 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R3-AC1 | 三个主题文件中 QWidget#protocolToolbar 有完整的属性块和闭合花括号 | 代码审查: 检查每个文件中 protocolToolbar 规则的语法 | 格式正确 |
| R3-AC2 | protocolToolbar 背景色与周围区域协调 | 手动测试: 三个主题下检查协议面板工具栏背景 | 无视觉异常 |
| R3-AC3 | QSS 解析无错误 | 运行应用，检查 debug 输出无 "Could not parse stylesheet" 警告 | 0 警告 |
| R3-AC4 | protocolStatusLabel 样式正确应用 | 手动测试: 检查协议面板状态文字颜色和字号 | 与设计一致 |

---

## 接口设计

### 新增接口

本 PRD 不新增公共接口。NavigationController 和 RecordingController 的接口已在上一轮迭代中定义并实现。

### 变更接口

| 接口 | 变更类型 | 影响分析 |
|------|---------|---------|
| MainWindow 构造函数 | 内部变更: 新增 NavigationController/RecordingController 初始化 | 无外部影响 |
| MainWindow::setupUI() | 内部变更: 委托 NavigationController 构建导航树 | 无外部影响 |
| MainWindow::setupToolbar() | 内部变更: 委托 RecordingController 创建录制按钮 | 无外部影响 |
| MainWindow::connectSignals() | 内部变更: 连接 Controller 信号 | 无外部影响 |

### 移除接口

| 接口 | 移除原因 |
|------|---------|
| MainWindow::buildNavPanelMappings() | 迁移到 NavigationController::buildNavTree() |
| MainWindow::allSwitchablePanels() | 迁移到 NavigationController |
| MainWindow::switchToPanel() | 迁移到 NavigationController |
| MainWindow::startBreathingAnimation() | 迁移到 NavigationController |
| MainWindow::stopBreathingAnimation() | 迁移到 NavigationController |
| NavPanelMapping 结构体（MainWindow.h 中定义） | 迁移到 NavigationController.h |

---

## 依赖的公共组件

| 组件 | 文件 | 复用方式 | 涉及需求 |
|------|------|---------|---------|
| NavigationController | `core/NavigationController.h/cpp` | MainWindow 委托导航/面板逻辑 | R1 |
| RecordingController | `core/RecordingController.h/cpp` | MainWindow 委托录制/回放逻辑 | R1 |
| DataLogger | `utils/DataLogger.h/cpp` | RecordingController 内部使用 | R1 |
| ThemeManager | `core/ThemeManager.h/cpp` | QSS 加载，修复后自动生效 | R3 |

---

## 设计模式

| 模式 | 应用场景 | 涉及需求 | 说明 |
|------|---------|---------|------|
| **委托模式 (Delegation)** | MainWindow 将导航和录制逻辑委托给专门的 Controller | R1 | 每个Controller遵循单一职责，MainWindow 仅做组装和信号路由 |
| **中介者模式 (Mediator)** | MainWindow 作为信号中介，转发 Controller 信号到 UI 组件 | R1 | RecordingController::playbackData -> MainWindow::onPlaybackData -> TerminalModel |

---

## 影响范围

### 文件变更矩阵

| 文件 | R1 | R2 | R3 |
|------|----|----|-----|
| `src/core/MainWindow.h` | 大幅精简: 删除冗余成员和槽声明 | - | - |
| `src/core/MainWindow.cpp` | 大幅精简: 删除迁移方法，委托 Controller | - | - |
| `src/core/NavigationController.h` | 无变更 | - | - |
| `src/core/NavigationController.cpp` | 无变更 | - | - |
| `src/core/RecordingController.h` | 无变更 | - | - |
| `src/core/RecordingController.cpp` | 无变更 | - | - |
| `resources/translations/embeddebug_en.ts` | - | 新增 NavigationController/RecordingController 上下文，补充缺失翻译 | - |
| `resources/translations/embeddebug_en.qm` | - | 由 lrelease 重新生成 | - |
| `resources/themes/dark_terminal.qss` | - | - | 修复 protocolToolbar 语法 |
| `resources/themes/modern_dark.qss` | - | - | 修复 protocolToolbar 语法 |
| `resources/themes/light.qss` | - | - | 修复 protocolToolbar 语法 |

### 预计变更量

| 需求 | 新增行数(估) | 修改行数(估) | 删除行数(估) |
|------|------------|------------|------------|
| R1 | 约 30 行（构造函数和信号连接重构） | 约 60 行（setupUI/setupToolbar/connectSignals 委托调用） | 约 450 行（删除冗余方法体和导航树内联构建） |
| R2 | 约 80 行（TS 文件新增条目） | 约 0 行 | 约 0 行 |
| R3 | 约 6 行（每个主题 +2 行属性） | 约 3 行（每个主题修改 1 行） | 约 0 行 |
| **合计** | **约 116 行** | **约 63 行** | **约 450 行** |

净减少约 **270 行**，符合 MainWindow 精简目标。

### 跨模块影响评估

- **R1 独立**: MainWindow 精简不涉及其他模块接口变更。NavigationController 和 RecordingController 的公共接口保持不变。
- **R2 依赖 R1**: lupdate 应在 R1 完成后执行，因为 R1 会移除 MainWindow 中已迁移的 tr() 调用（如导航树节点的 tr() 从 MainWindow 迁移到 NavigationController context）。如果在 R1 之前执行 lupdate，会先收割 MainWindow context 的条目，R1 后再执行一次 lupdate 会将这些条目标记为 obsolete 并在 NavigationController context 中重新创建。
- **R3 独立**: QSS 修复与代码变更无依赖关系，可随时执行。

---

## 验收标准总表

| 需求 | 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|------|---------|---------|---------|
| R1 | R1-AC1 | MainWindow.cpp < 600 行 | wc -l | < 600 |
| R1 | R1-AC2 | MainWindow.h < 200 行 | wc -l | < 200 |
| R1 | R1-AC3 | 无冗余方法残留 | grep 检查 | 0 匹配 |
| R1 | R1-AC4 | 面板切换动画正常 | 手动测试 | 动画流畅 |
| R1 | R1-AC5 | 呼吸动画正常 | 手动测试 | 脉冲可见 |
| R1 | R1-AC6 | 录制/暂停/恢复/停止正常 | 手动测试 | 状态正确 |
| R1 | R1-AC7 | 回放功能正常 | 手动测试 | 数据正确 |
| R1 | R1-AC8 | 编译零错误 | cmake --build | 0 error |
| R1 | R1-AC9 | Controller 文件无变更 | git diff | 不在变更列表 |
| R2 | R2-AC1 | 0 条 unfinished 翻译 | grep -c | 0 |
| R2 | R2-AC2 | NavigationController 上下文完整 | TS 文件检查 | 覆盖所有 tr() |
| R2 | R2-AC3 | RecordingController 上下文完整 | TS 文件检查 | 覆盖所有 tr() |
| R2 | R2-AC4 | SerialConfigPanel 标签翻译完整 | TS 文件检查 | 含冒号版本均存在 |
| R2 | R2-AC5 | 英文模式无中文残留 | 手动测试 | 全英文 |
| R2 | R2-AC6 | 中文模式无英文残留 | 手动测试 | 全中文 |
| R2 | R2-AC7 | 无重复条目 | TS 文件检查 | 同 context 无重复 source |
| R3 | R3-AC1 | protocolToolbar 语法正确 | 代码审查 | 格式正确 |
| R3 | R3-AC2 | 工具栏背景色协调 | 手动测试 | 无视觉异常 |
| R3 | R3-AC3 | QSS 解析无警告 | 运行时检查 | 0 警告 |
| R3 | R3-AC4 | 状态标签样式正确 | 手动测试 | 颜色字号正确 |

---

## 实施优先级

| 顺序 | 需求 | 优先级 | 理由 |
|------|------|--------|------|
| 1 | **R1 (P0)** | MainWindow Split | 违反 CLAUDE.md 4.6 文件体积铁律，必须立即修复；且 R2 依赖 R1 完成后再收割翻译 |
| 2 | **R3 (P1)** | QSS 修复 | 独立于 R1，变更极小（3 行），可在 R1 之后立即完成 |
| 3 | **R2 (P1)** | Translation Harvest | 需在 R1 完成后执行 lupdate，确保收割结果反映最终代码状态 |

**并行策略**:
- R1 和 R3 无依赖关系，可同时执行
- R2 必须在 R1 完成后执行，否则会收割到 MainWindow 中即将被移除的 tr() 字符串

---

## 验证度量指标

### 代码度量

| 度量项 | 度量方法 | 当前基线 | 目标值 |
|--------|---------|---------|--------|
| MainWindow.cpp 行数 | wc -l | 1092 行 | < 600 行 |
| MainWindow.h 行数 | wc -l | 218 行 | < 200 行 |
| 未翻译字符串数 | grep -c 'unfinished' | 34 条 | 0 条 |
| QSS 语法错误数 | 运行时 "Could not parse stylesheet" 警告 | 1 处（protocolToolbar） | 0 处 |

### 功能度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|--------|
| 英文模式 UI 覆盖率 | 手动测试: 逐个检查导航树/配置面板/录制按钮/状态栏文本 | 100% 英文 |
| 导航树面板切换 | 手动测试: 点击全部 7 个面板节点 | 全部切换成功且有动画 |
| 录制回放流程 | 手动测试: 录制 -> 暂停 -> 恢复 -> 停止 -> 回放 | 全流程通过 |
