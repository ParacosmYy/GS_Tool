/**
 * @file MainWindowSetupUI.cpp
 * @brief MainWindow UI布局构建方法
 *
 * 从 MainWindow.cpp 拆分而来，包含 setupUI、createNavigationArea、
 * createContentArea 和 setupStatusBar 四个 UI 构建方法。
 * 信号连接见 MainWindowSignalConnect.cpp。
 */

#include "core/mainwindow/MainWindow.h"
#include "core/mainwindow/MainWindowLayoutState.h"
#include "shared/LayoutConstants.h"

#include <QVBoxLayout>
#include <QSplitter>

/** @brief 构建完整的UI布局（背景层→分割器→导航树→面板栈→发送栏） */
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

    // 导航树宽度持久化: 分割器拖动后自动保存
    connect(m_mainSplitter, &QSplitter::splitterMoved,
            this, [this]() {
        auto sizes = m_mainSplitter->sizes();
        const int navTreeWidth = savedNavTreeWidthFromSplitterSizes(sizes, m_useIconNavBar);
        if (navTreeWidth > 0) {
            SettingsManager::instance().set("layout/navTreeWidth", navTreeWidth);
        }
    });

    // 恢复上次的导航树宽度（在buildNavTree之后，导航树已有模型）
    int savedNavWidth = SettingsManager::instance().get("layout/navTreeWidth").toInt();
    if (savedNavWidth > 0 && m_mainSplitter) {
        m_mainSplitter->setSizes(restoredNavigationSplitterSizes(savedNavWidth, width(), m_useIconNavBar));
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

    // 面板栈布局(自上而下): 配置面板区 → 终端区 → 快捷指令 → 发送栏
    for (auto* panel : m_panelManager->allPanels()) {
        if (panel) {
            serialLayout->addWidget(panel);
        }
    }

    // 终端管理器: 提供分屏+Tab管理
    m_layoutManager = new TerminalLayoutManager(this);
    m_layoutManager->initialize(m_panelManager->terminal(), m_panelManager->searchBar());
    m_layoutManager->setTerminalModel(m_terminalModel);

    serialLayout->addWidget(m_layoutManager->container(), 1);

    // 快捷指令栏（底部固定）
    serialLayout->addWidget(m_panelManager->quickCmdBar());

    // 发送栏（由 SendController 创建和管理生命周期）
    QWidget* sendBar = m_sendController->createSendBar(this);
    serialLayout->addWidget(sendBar);

    rightPanelLayout->addWidget(serialPanel);
    return rightWidget;
}

/** @brief 构建状态栏（左侧连接状态、右侧RX/TX字节计数） */
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
