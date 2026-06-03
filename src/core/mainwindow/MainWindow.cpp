/**
 * @file MainWindow.cpp
 * @brief 主窗口实现 - 应用顶层窗口的 UI 构建、信号连接和生命周期管理
 *
 * 本文件实现 MainWindow 的所有方法，遵循"嵌入式 main 哲学":
 * MainWindow 自身不包含业务逻辑，仅负责:
 *   1. 初始化所有子对象
 *   2. 组装 UI 布局
 *   3. 连接信号/槽
 * 所有业务逻辑委托给各 Controller 处理。
 */

#include "core/mainwindow/MainWindow.h"
#include "serial/data/BookmarkWidget.h"
#include "serial/commands/TimedSender.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QApplication>
#include <QSplitter>
#include <QShortcut>
#include <QKeySequence>
#include <QDir>

/**
 * @brief 构造函数 - 初始化所有子模块并组装主窗口
 *
 * 初始化顺序:
 * 1. 创建所有 Controller/Manager（通过构造函数依赖注入）
 * 2. 注入跨控制器依赖关系
 * 3. 构建 UI 布局和状态栏
 * 4. 连接所有信号/槽
 * 5. 构建导航树和面板映射
 * 6. 加载持久化设置
 * 7. 启动统计刷新定时器
 */
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_connManager(new ConnectionManager(this))
    , m_connController(new ConnectionController(m_connManager, this))
    , m_terminalModel(new TerminalModel(this))
    , m_sendHistory(new SendHistory(this))
    , m_dataExporter(new DataExporter(this))
    , m_dataLogger(new DataLogger(this))
    , m_recordingController(new RecordingController(m_dataLogger, this))
    , m_sendController(new SendController(m_terminalModel, m_dataLogger, m_sendHistory, this))
    , m_frameParser(new FrameParser(this))
    , m_protocolBridgeMgr(new ProtocolBridgeManager(m_frameParser, this))
    , m_otaManager(new OtaManager(this))
    , m_panelManager(new PanelManager(this))
    , m_navController(new NavigationController(this))
    , m_toolbarController(new ToolbarController(m_recordingController, this))
    , m_settingsController(new SettingsController(this, this))
    , m_terminalController(new TerminalController(m_terminalModel, m_dataExporter, this))
{
    // 依赖注入: ConnectionController 需要在连接/断开时通知 SendController/OtaManager/RecordingController
    m_connController->setSendController(m_sendController);
    m_connController->setOtaManager(m_otaManager);
    m_connController->setRecordingController(m_recordingController);

    // 构建 UI 布局（背景层→分割器→导航树→面板栈→发送栏）
    setupUI();
    // 创建工具栏控件并添加到主窗口
    m_toolbarController->createToolbar(this);
    // 创建状态栏标签（连接状态、RX/TX 字节）
    setupStatusBar();

    // 背景设置弹出面板（浮动窗口，父级为背景层）
    m_bgSettingsPopup = new BackgroundSettingsPopup(m_backgroundWidget, this);

    // 从设置加载用户上次选择的自定义背景图
    QString customBg = SettingsManager::instance().get("background/customImagePath").toString();
    if (!customBg.isEmpty() && QFile::exists(customBg)) {
        m_backgroundWidget->setBackgroundImage(customBg);
    }

    // 注入依赖到 TerminalController（状态栏标签在 setupStatusBar() 中直接注入）
    m_terminalController->setLayoutManager(m_layoutManager);
    m_terminalController->setDataStatistics(m_panelManager->dataStats());
    m_terminalController->setMainTerminal(m_panelManager->terminal());

    // 连接所有模块间的信号/槽
    connectSignals();

    // 构建导航树模型: 通过 PanelManager 获取面板映射表
    m_navController->buildNavTree(m_navTree, m_panelManager->panelMappings());

    // 填充图标导航栏分类(从面板映射表提取去重分类)
    if (m_useIconNavBar && m_iconNavBar) {
        QVector<NavCategory> categories;
        const auto& mappings = m_panelManager->panelMappings();
        QMap<QString, NavCategory> seen;
        for (const auto& m : mappings) {
            if (!seen.contains(m.category)) {
                NavCategory cat;
                cat.id = QString::fromUtf8(m.category);
                cat.label = tr(m.category);
                cat.panelIds.append(QString::fromUtf8(m.name));
                seen.insert(m.category, cat);
            } else {
                seen[m.category].panelIds.append(QString::fromUtf8(m.name));
            }
        }
        categories = seen.values().toVector();
        m_iconNavBar->setCategories(categories);
        connect(m_iconNavBar, &IconNavBar::categoryClicked, this,
                [this](const QString& id) {
            Q_UNUSED(id);
            // 后续: 展开导航树对应分类并高亮第一个面板
        });
    }

    // 初始面板状态: 终端为默认可见面板（不触发动画）
    m_navController->setCurrentPanel(m_panelManager->terminal());

    // 命令面板 (Ctrl+P 快速导航)
    m_commandPalette = new CommandPalette(this);
    {
        QVector<CommandEntry> cmds;
        for (const auto& m : m_panelManager->panelMappings()) {
            CommandEntry e;
            e.id = "nav." + QString(m.name);
            e.category = tr("导航");
            e.label = tr(m.name);
            e.action = [this, m]() { m_navController->switchToPanel(m.widget); };
            cmds.append(e);
        }
        m_commandPalette->registerCommands(cmds);
    }
    auto* cmdShortcut = new QShortcut(QKeySequence("Ctrl+P"), this);
    connect(cmdShortcut, &QShortcut::activated, m_commandPalette, &CommandPalette::showPalette);

    // 脚本录制器 — 从 PanelManager 获取已创建的面板实例
    m_scriptRecorder = m_panelManager->scriptRecorder();
    m_scriptRecorder->setObjectName("scriptRecorder");
    // 回放发送请求 → SendController（lambda 桥接 QString→QByteArray）
    connect(m_scriptRecorder, &ScriptRecorder::playbackSendRequested,
            this, [this](const QString& data, bool isHex) {
        Q_UNUSED(isHex);
        m_sendController->onQuickCommand(data.toUtf8());
    });
    // 录制状态变化 → 状态栏提示
    connect(m_scriptRecorder, &ScriptRecorder::recordingChanged,
            this, [this](bool recording) {
        statusBar()->showMessage(recording ? tr("脚本录制中...") : tr("录制已停止"), 3000);
    });

    // 设置 ThemeManager 主题切换淡入淡出动画目标（中央背景层）
    ThemeManager::instance().setTransitionWidget(m_backgroundWidget);

    // 注入 UI 引用到 SettingsController，用于同步主题/语言下拉框和串口配置面板
    m_settingsController->setToolbarController(m_toolbarController);
    m_settingsController->setSerialConfigPanel(m_panelManager->serialConfig());

    // 创建会话管理器，注入依赖引用
    m_sessionManager = new SessionManager(this, this);
    m_sessionManager->setSettingsController(m_settingsController);
    m_sessionManager->setSerialConfigPanel(m_panelManager->serialConfig());

    // 从磁盘恢复上次保存的完整工作区（窗口几何、串口配置、面板索引）
    int lastPanel = m_sessionManager->loadSession();

    // 恢复语言/主题/面板/统计定时器
    restoreUserSession(lastPanel);

    // 设置窗口属性
    setWindowTitle(App::APP_NAME);
    resize(1200, 800);
    setMinimumSize(900, 600);

    // ---- 响应式布局: 监听窗口宽度变化，自动切换断点 ----
    m_responsiveLayout = new ResponsiveLayout(this);
    m_responsiveLayout->watchWindow(this);
    connect(m_responsiveLayout, &ResponsiveLayout::breakpointChanged,
            this, [this](ResponsiveLayout::Breakpoint bp) {
        /* 断点切换时，Compact模式自动折叠导航树 */
        if (bp == ResponsiveLayout::Breakpoint::Compact) {
            /* 紧凑模式: 折叠导航树，隐藏图标栏 */
            if (m_mainSplitter && m_mainSplitter->sizes().at(0) > 0) {
                m_mainSplitter->setSizes({0, width()});
            }
            if (m_iconNavBar) {
                m_iconNavBar->hide();
            }
            /* 紧凑模式下隐藏非关键面板区域 */
            statusBar()->showMessage(tr("已切换到紧凑布局"), 2000);
            m_panelManager->setCompactMode(true);
        } else if (bp == ResponsiveLayout::Breakpoint::Desktop) {
            /* 桌面模式: 恢复导航树全宽 */
            if (m_mainSplitter && m_mainSplitter->sizes().at(0) == 0) {
                int navWidth = SettingsManager::instance()
                    .get("layout/navTreeWidth").toInt();
                if (navWidth <= 0) navWidth = 200;
                m_mainSplitter->setSizes({navWidth, width() - navWidth});
            }
            if (m_iconNavBar && m_useIconNavBar) {
                m_iconNavBar->show();
            }
            statusBar()->showMessage(tr("已切换到桌面布局"), 2000);
            m_panelManager->setCompactMode(false);
        } else {
            /* 中等模式: 恢复导航树但较窄 */
            if (m_mainSplitter && m_mainSplitter->sizes().at(0) == 0) {
                m_mainSplitter->setSizes({180, width() - 180});
            }
            if (m_iconNavBar && m_useIconNavBar) {
                m_iconNavBar->show();
            }
        }
    });

    // ---- 快捷键管理器: 统一注册全局快捷键 ----
    m_shortcutManager = &ShortcutManager::instance();
    m_shortcutManager->registerShortcut(
        "search.find", QKeySequence("Ctrl+F"),
        this, [this]() { m_panelManager->searchBar()->activate(); },
        tr("搜索"));
    m_shortcutManager->registerShortcut(
        "nav.commandPalette", QKeySequence("Ctrl+P"),
        this, [this]() { m_commandPalette->showPalette(); },
        tr("命令面板"));
    m_shortcutManager->registerShortcut(
        "script.recordToggle", QKeySequence("Ctrl+Shift+R"),
        this, [this]() {
            if (m_scriptRecorder->isRecording()) {
                m_scriptRecorder->stopRecording();
            } else {
                m_scriptRecorder->startRecording();
            }
        }, tr("录制脚本"));
}

/** @brief 从磁盘恢复用户偏好（语言、主题、面板索引）并启动统计定时器 */
void MainWindow::restoreUserSession(int lastPanel)
{
    // 恢复语言选择到工具栏下拉框
    if (m_toolbarController) {
        QString savedLang = SettingsManager::instance().loadLanguage();
        m_toolbarController->setCurrentLanguage(savedLang);
    }

    // 恢复主题: 加载主题文件 + 同步工具栏下拉框选中项
    QString savedTheme = SettingsManager::instance().loadTheme();
    if (ThemeManager::instance().loadTheme(savedTheme)) {
        if (m_toolbarController) m_toolbarController->setCurrentTheme(savedTheme);
    }

    // 恢复上次活跃面板（如果有保存记录）
    if (lastPanel >= 0) m_navController->restorePanelByIndex(lastPanel);

    // 启动统计刷新定时器（每 500ms 触发一次）
    m_terminalController->startStatsTimer();
}

/**
 * @brief 构建 UI 布局
 *
 * 布局层次:
 * BackgroundWidget (中央部件)
 *   └── QVBoxLayout
 *       └── QSplitter (水平分割)
 *           ├── QTreeView (左侧导航树, 180~280px)
 *           └── QWidget (右侧面板容器)
 *               └── m_rightPanel → serialPanel (面板栈)
 *                   ├── SerialConfigPanel / DataStatistics / ProtocolView / FrameVisualEditor / ChartWidget / OtaWidget
 *                   ├── terminalContainer (搜索栏 + 终端)
 *                   ├── QuickCommandBar (快捷指令)
 *                   └── SendBar (发送栏, 由 SendController 创建)
 */
void MainWindow::setupUI()
{
    // ---- 背景: 磨砂玻璃背景层作为中央部件 ----
    m_backgroundWidget = new BackgroundWidget(this);
    m_backgroundWidget->setObjectName("backgroundWidget");  // QSS 选择器需要
    setCentralWidget(m_backgroundWidget);
    auto* bgLayout = new QVBoxLayout(m_backgroundWidget);
    bgLayout->setContentsMargins(0, 0, 0, 0);
    bgLayout->setSpacing(0);

    m_mainSplitter = new QSplitter(Qt::Horizontal, m_backgroundWidget);
    m_mainSplitter->setObjectName("mainSplitter");  // QSS 选择器需要

    m_mainSplitter->addWidget(createNavigationArea());
    m_mainSplitter->addWidget(createContentArea());
    m_mainSplitter->setSizes({200, 1000});
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);

    // 图标导航栏功能开关(默认关闭, 通过 SettingsManager "ui/iconNavBar" 开启)
    m_useIconNavBar = SettingsManager::instance().get("ui/iconNavBar").toBool();
    if (m_useIconNavBar) {
        m_iconNavBar = new IconNavBar(m_backgroundWidget);
        m_iconNavBar->setObjectName("iconNavBar");
        m_mainSplitter->insertWidget(0, m_iconNavBar);
        m_mainSplitter->setSizes({56, 200, 1000});
    }

    bgLayout->addWidget(m_mainSplitter, 1);

    // Ctrl+F 快捷键激活搜索栏
    auto* searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(searchShortcut, &QShortcut::activated, m_panelManager->searchBar(), &TerminalSearchBar::activate);

    // 导航树宽度持久化: 分割器拖动后自动保存
    connect(m_mainSplitter, &QSplitter::splitterMoved,
            this, [this]() {
        auto sizes = m_mainSplitter->sizes();
        if (sizes.size() > 0) {
            SettingsManager::instance().set("layout/navTreeWidth", sizes.at(0));
        }
    });

    // 恢复上次的导航树宽度
    int savedNavWidth = SettingsManager::instance().get("layout/navTreeWidth").toInt();
    if (savedNavWidth > 0) {
        m_mainSplitter->setSizes({savedNavWidth, width() - savedNavWidth});
    }
}

/** @brief 创建左侧导航树区域(导航树+选中滑动指示器) */
QWidget* MainWindow::createNavigationArea()
{
    // ---- 左侧导航树（数据模型由 NavigationController.buildNavTree() 构建） ----
    m_navTree = new QTreeView;
    m_navTree->setObjectName("navTree");
    m_navTree->setHeaderHidden(true);
    m_navTree->setMinimumWidth(Layout::kNavTreeMinWidth);
    m_navTree->setMaximumWidth(Layout::kNavTreeMaxWidth);
    m_navTree->setIndentation(16);

    // 导航树选中滑动指示器（覆盖在 navTree 上方，透明背景，accent 色竖线动画）
    m_navIndicator = new NavIndicatorWidget(m_navTree);

    return m_navTree;
}

/** @brief 创建右侧面板内容区域(面板栈+终端+快捷指令+发送栏) */
QWidget* MainWindow::createContentArea()
{
    // ---- 右侧内容面板 ----
    auto* rightWidget = new QWidget;
    rightWidget->setObjectName("rightWidget");
    rightWidget->setAttribute(Qt::WA_StyledBackground, true);
    auto* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    m_rightPanel = new QWidget;
    m_rightPanel->setObjectName("rightPanel");
    m_rightPanel->setAttribute(Qt::WA_StyledBackground, true);
    auto* rightPanelLayout = new QVBoxLayout(m_rightPanel);
    rightPanelLayout->setContentsMargins(0, 0, 0, 0);
    rightPanelLayout->setSpacing(0);
    rightLayout->addWidget(m_rightPanel, 1);

    // 面板栈: 所有面板共用同一位置，通过 NavigationController 切换显示
    auto* serialPanel = new QWidget;
    serialPanel->setObjectName("serialPanel");
    serialPanel->setAttribute(Qt::WA_StyledBackground, true);
    auto* serialLayout = new QVBoxLayout(serialPanel);
    serialLayout->setContentsMargins(0, 0, 0, 0);
    serialLayout->setSpacing(0);

    // 通过 PanelManager 统一创建所有面板
    m_panelManager->createPanels(m_otaManager, m_terminalModel);
    m_panelManager->wrapPanels();  // 将可切换面板包装在 BasePanel 容器中

    // 将所有面板添加到面板栈布局（数据驱动，遍历 PanelManager.allPanels()）
    for (auto* panel : m_panelManager->allPanels()) {
        if (panel) {
            serialLayout->addWidget(panel);
        }
    }

    // 终端布局管理器
    m_layoutManager = new TerminalLayoutManager(this);
    m_layoutManager->initialize(m_panelManager->terminal(), m_panelManager->searchBar());
    m_layoutManager->setTerminalModel(m_terminalModel);

    serialLayout->addWidget(m_layoutManager->container(), 1);

    // 快捷指令栏
    serialLayout->addWidget(m_panelManager->quickCmdBar());

    // 发送区域: 由 SendController 创建和管理
    QWidget* sendBar = m_sendController->createSendBar(this);
    serialLayout->addWidget(sendBar);

    rightPanelLayout->addWidget(serialPanel);
    return rightWidget;
}

/**
 * @brief 创建并初始化状态栏
 * 左侧显示连接状态，右侧显示 RX/TX 字节计数
 */
void MainWindow::setupStatusBar()
{
    m_connStatusLbl = new QLabel(tr("未连接"));
    m_connStatusLbl->setObjectName("connStatus");
    m_connStatusLbl->setProperty("state", "disconnected");
    auto* rxLbl = new QLabel(tr("RX: 0 B"));
    rxLbl->setObjectName("rxBytesLabel");
    auto* txLbl = new QLabel(tr("TX: 0 B"));
    txLbl->setObjectName("txBytesLabel");

    statusBar()->addWidget(m_connStatusLbl, 1);
    statusBar()->addPermanentWidget(rxLbl);
    statusBar()->addPermanentWidget(txLbl);

    // 将字节标签注入 TerminalController（此处创建后直接注入，避免 findChild 查找）
    if (m_terminalController) {
        m_terminalController->setStatusBarLabels(rxLbl, txLbl);
    }
}

/**
 * @brief 处理连接状态变更
 *
 * 根据连接状态更新:
 *   - 状态栏文本和动态属性（驱动 QSS 状态样式）
 *   - 串口配置面板的连接/断开按钮状态
 *   - 呼吸动画（连接中时启动脉冲闪烁）
 *   - 自动切换到终端面板（连接成功后）
 *
 * @param state 连接状态枚举
 * @param connName 连接名称
 */
void MainWindow::handleConnectionState(ConnectionState state, const QString& connName)
{
    const char* stateStr = "";
    switch (state) {
    case ConnectionState::Connected:
        m_connStatusLbl->setText(tr("已连接: %1").arg(connName));
        stateStr = "connected";
        m_panelManager->serialConfig()->setConnected(true);
        m_navController->switchToPanel(m_panelManager->terminal());
        m_navController->stopBreathingAnimation(m_connStatusLbl);
        break;
    case ConnectionState::Disconnected:
        m_connStatusLbl->setText(tr("未连接"));
        stateStr = "disconnected";
        m_panelManager->serialConfig()->setConnected(false);
        m_navController->stopBreathingAnimation(m_connStatusLbl);
        break;
    case ConnectionState::Connecting:
        m_connStatusLbl->setText(tr("连接中..."));
        stateStr = "connecting";
        m_navController->startBreathingAnimation(m_connStatusLbl);
        break;
    case ConnectionState::Error:
        m_connStatusLbl->setText(tr("连接错误"));
        stateStr = "error";
        m_panelManager->serialConfig()->setConnected(false);
        m_navController->stopBreathingAnimation(m_connStatusLbl);
        break;
    default:
        stateStr = "unknown";
        break;
    }
    // 通过动态属性驱动 QSS 状态样式切换
    m_connStatusLbl->setProperty("state", stateStr);
    m_connStatusLbl->style()->unpolish(m_connStatusLbl);
    m_connStatusLbl->style()->polish(m_connStatusLbl);
}

/**
 * @brief 切换背景设置弹出面板的显示/隐藏
 * 面板定位在工具栏右下角，使用 Qt::Popup 属性实现点击外部自动关闭
 */
void MainWindow::onBgSettingsToggled()
{
    if (m_bgSettingsPopup->isVisible()) {
        m_bgSettingsPopup->hide();
    } else {
        m_bgSettingsPopup->syncFromWidget();
        QToolBar* tb = m_toolbarController->toolbar();
        QPoint pos = tb->mapToGlobal(QPoint(tb->width() - 270, tb->height() + 2));
        m_bgSettingsPopup->move(pos);
        m_bgSettingsPopup->show();
    }
}

/**
 * @brief 窗口关闭事件处理
 * 按顺序执行清理: 停止动画 → 停止录制/回放 → 停止统计定时器 → 保存设置 → 关闭连接
 * @param event 关闭事件
 */
void MainWindow::closeEvent(QCloseEvent* event)
{
    // 停止连接状态的呼吸动画
    m_navController->stopBreathingAnimation(m_connStatusLbl);

    // 停止录制/回放（防止后台线程写入已关闭文件）
    if (m_dataLogger->isRecording()) m_dataLogger->stopRecording();
    if (m_dataLogger->isPlaying()) m_dataLogger->stopPlayback();

    // 停止统计刷新定时器
    m_terminalController->stopStatsTimer();

    // 停止定时发送器（防止关闭后仍有挂起的发送）
    if (m_sendController && m_sendController->timedSender()) {
        m_sendController->timedSender()->stop();
    }

    // 保存当前面板索引（供下次恢复使用）
    if (m_navController) {
        int currentIdx = m_navController->currentPanelIndex();
        if (currentIdx >= 0) {
            m_settingsController->saveLastPanel(currentIdx);
        }
    }

    // 保存当前断点模式(下次启动时立即应用正确的布局)
    if (m_responsiveLayout) {
        SettingsManager::instance().set("layout/lastBreakpoint",
            static_cast<int>(m_responsiveLayout->currentBreakpoint()));
    }

    // 保存导航树宽度(Compact模式时保存上次展开宽度)
    if (m_mainSplitter) {
        auto sizes = m_mainSplitter->sizes();
        if (sizes.size() > 0 && sizes.at(0) > 0) {
            SettingsManager::instance().set("layout/navTreeWidth", sizes.at(0));
        }
    }

    // 通过会话管理器保存完整工作区到磁盘（窗口几何、串口配置、主题）
    m_sessionManager->saveSession();

    // 关闭所有活跃连接
    auto connections = m_connManager->connections();
    for (auto* conn : connections) {
        conn->close();
    }
    event->accept();
}
