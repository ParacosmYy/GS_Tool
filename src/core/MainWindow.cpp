/**
 * @file MainWindow.cpp
 * @brief 主窗口实现 - 应用顶层窗口的 UI 构建、信号连接和生命周期管理
 *
 * 本文件实现 MainWindow 的所有方法，遵循"中介者模式":
 * MainWindow 自身不包含业务逻辑，仅负责组装各 Controller/Manager 并连接信号/槽。
 */

#include "MainWindow.h"
#include "chart/ChartModel.h"
#include "utils/HexConverter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QCloseEvent>
#include <QApplication>
#include <QSplitter>
#include <QFileDialog>
#include <QTimer>
#include <QShortcut>
#include <QKeySequence>
#include <QTranslator>
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
    , m_statsTimer(new QTimer(this))
    , m_frameParser(new FrameParser(this))
    , m_protocolBridgeMgr(new ProtocolBridgeManager(m_frameParser, this))
    , m_otaManager(new OtaManager(this))
    , m_panelManager(new PanelManager(this))
    , m_navController(new NavigationController(this))
    , m_toolbarController(new ToolbarController(m_recordingController, this))
    , m_settingsController(new SettingsController(this, this))
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

    // 连接所有模块间的信号/槽
    connectSignals();

    // 构建导航树模型: 通过 PanelManager 获取面板映射表
    m_navController->buildNavTree(m_navTree, m_panelManager->panelMappings());

    // 初始面板状态: 终端为默认可见面板（不触发动画）
    m_navController->setCurrentPanel(m_panelManager->terminal());

    // 注入 UI 引用到 SettingsController，用于同步主题/语言下拉框和串口配置面板
    m_settingsController->setToolbarController(m_toolbarController);
    m_settingsController->setSerialConfigPanel(m_panelManager->serialConfig());

    // 从磁盘加载上次保存的设置（主题、窗口几何、串口配置、语言）
    m_settingsController->loadSettings();

    // 统计刷新定时器: 每 500ms 触发一次 updateDataStatistics()
    m_statsTimer->setInterval(500);
    m_statsTimer->start();

    // 设置窗口属性
    setWindowTitle(App::APP_NAME);
    resize(1200, 800);
    setMinimumSize(900, 600);
}

/** @brief 析构函数 - QObject 父子树自动销毁所有子组件，无需手动 delete */
MainWindow::~MainWindow()
{
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
    setCentralWidget(m_backgroundWidget);
    auto* bgLayout = new QVBoxLayout(m_backgroundWidget);
    bgLayout->setContentsMargins(0, 0, 0, 0);
    bgLayout->setSpacing(0);

    m_mainSplitter = new QSplitter(Qt::Horizontal, m_backgroundWidget);

    // ---- 左侧导航树（数据模型由 NavigationController.buildNavTree() 构建） ----
    m_navTree = new QTreeView;
    m_navTree->setObjectName("navTree");
    m_navTree->setHeaderHidden(true);
    m_navTree->setMinimumWidth(180);
    m_navTree->setMaximumWidth(280);
    m_navTree->setIndentation(16);

    m_mainSplitter->addWidget(m_navTree);

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

    // 通过 PanelManager 统一创建所有面板（面板初始全部隐藏，由 NavigationController 按需显示）
    m_panelManager->createPanels(m_otaManager, m_terminalModel);

    // 将所有面板添加到面板栈布局
    serialLayout->addWidget(m_panelManager->serialConfig());
    serialLayout->addWidget(m_panelManager->dataStats());
    serialLayout->addWidget(m_panelManager->protocolView());
    serialLayout->addWidget(m_panelManager->frameEditor());
    serialLayout->addWidget(m_panelManager->chartWidget());
    serialLayout->addWidget(m_panelManager->otaWidget());

    // 终端布局管理器: 管理混合/分栏模式切换，封装终端容器内的widget层次
    m_layoutManager = new TerminalLayoutManager(this);
    m_layoutManager->initialize(m_panelManager->terminal(), m_panelManager->searchBar());
    m_layoutManager->setTerminalModel(m_terminalModel);

    serialLayout->addWidget(m_layoutManager->container(), 1);

    // 快捷指令栏
    serialLayout->addWidget(m_panelManager->quickCmdBar());

    // 发送区域: 由 SendController 创建和管理（输入框+模式切换+发送按钮+换行符选择）
    QWidget* sendBar = m_sendController->createSendBar(this);
    serialLayout->addWidget(sendBar);

    rightPanelLayout->addWidget(serialPanel);

    m_mainSplitter->addWidget(rightWidget);
    // 初始分割比例: 导航树 200px, 面板区 1000px
    m_mainSplitter->setSizes({200, 1000});
    // 导航树不随窗口拉伸，面板区占满剩余空间
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);

    // 将分割器放入背景层布局
    bgLayout->addWidget(m_mainSplitter, 1);

    // Ctrl+F 快捷键激活搜索栏
    auto* searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(searchShortcut, &QShortcut::activated, m_panelManager->searchBar(), &TerminalSearchBar::activate);
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
    m_rxBytesLbl = new QLabel("RX: 0 B");
    m_rxBytesLbl->setObjectName("rxBytesLabel");
    m_txBytesLbl = new QLabel("TX: 0 B");
    m_txBytesLbl->setObjectName("txBytesLabel");

    statusBar()->addWidget(m_connStatusLbl, 1);          // 左侧可拉伸
    statusBar()->addPermanentWidget(m_rxBytesLbl);        // 右侧固定
    statusBar()->addPermanentWidget(m_txBytesLbl);        // 右侧固定
}

/**
 * @brief 连接所有模块间的信号/槽
 *
 * 信号流向:
 *   SerialConfigPanel → ConnectionController → MainWindow(状态更新)
 *   ConnectionController → TerminalModel(接收数据) + ProtocolBridgeMgr(协议解析)
 *   QuickCommandBar → SendController(发送数据)
 *   ToolbarController → MainWindow(显示模式/时间戳/清屏/导出)
 *   RecordingController → MainWindow(回放数据写入终端)
 *   TerminalSearchBar → TerminalWidget(搜索高亮)
 *   FrameEditor → FrameParser(帧定义) + ChartWidget(波形配置)
 *   NavTree → NavigationController(面板切换)
 */
void MainWindow::connectSignals()
{
    // ---- 串口连接/断开: 委托 ConnectionController 处理 ----
    // 用户点击"连接"时，从 SerialConfigPanel 收集参数并调用 ConnectionController
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::connectRequested,
            this, [this]() {
        QVariantMap params;
        params["portName"] = m_panelManager->serialConfig()->currentPortData();
        params["baudRate"] = m_panelManager->serialConfig()->currentBaudRate();
        params["dataBits"] = m_panelManager->serialConfig()->currentDataBitsIndex() + 5;  // 索引0对应5位
        params["parity"] = m_panelManager->serialConfig()->currentParityIndex();
        params["stopBits"] = m_panelManager->serialConfig()->currentStopBitsIndex();
        params["flowControl"] = m_panelManager->serialConfig()->currentFlowControlIndex();
        params["dtr"] = m_panelManager->serialConfig()->dtrEnabled();
        params["rts"] = m_panelManager->serialConfig()->rtsEnabled();
        m_connController->connectSerial(params);
    });
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::disconnectRequested,
            m_connController, &ConnectionController::disconnectCurrent);
    // DTR/RTS 线路控制信号直连
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::dtrChanged,
            m_connController, &ConnectionController::setDtr);
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::rtsChanged,
            m_connController, &ConnectionController::setRts);

    // 连接状态变化 → 更新 UI（状态栏文本、配置面板按钮状态、呼吸动画）
    connect(m_connController, &ConnectionController::connectionStateChanged,
            this, [this](ConnectionState state, const QString& connName) {
        const char* stateStr = "";
        switch (state) {
        case ConnectionState::Connected:
            m_connStatusLbl->setText(tr("已连接: %1").arg(connName));
            stateStr = "connected";
            m_panelManager->serialConfig()->setConnected(true);
            // 连接成功后自动切换到终端面板（带淡入动画）
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
            // 启动呼吸动画，让用户知道正在连接（脉冲闪烁效果）
            m_navController->startBreathingAnimation(m_connStatusLbl);
            break;
        case ConnectionState::Error:
            m_connStatusLbl->setText(tr("连接错误"));
            stateStr = "error";
            m_panelManager->serialConfig()->setConnected(false);
            m_navController->stopBreathingAnimation(m_connStatusLbl);
            break;
        }
        // 通过动态属性驱动 QSS 状态样式切换
        m_connStatusLbl->setProperty("state", stateStr);
        m_connStatusLbl->style()->unpolish(m_connStatusLbl);
        m_connStatusLbl->style()->polish(m_connStatusLbl);
    });

    // 接收数据 → 终端模型 + 协议解析 + 日志记录
    connect(m_connController, &ConnectionController::dataReceived,
            this, [this](const QByteArray& data) {
        m_terminalModel->appendReceived(data);            // 显示到终端
        m_protocolBridgeMgr->feedData(data);              // 送入协议解析管道
        m_dataLogger->logData(data, DataLogger::Direction::Received);  // 记录日志
    });

    // 状态栏更新和连接失败通知
    connect(m_connController, &ConnectionController::statusBarUpdateRequested,
            this, &MainWindow::updateStatusBar);
    connect(m_connController, &ConnectionController::connectionFailed,
            this, [this](const QString& title, const QString& message) {
        QMessageBox::warning(this, title, message);
    });

    // 快捷指令 → 发送控制器
    connect(m_panelManager->quickCmdBar(), &QuickCommandBar::commandTriggered,
            m_sendController, &SendController::onQuickCommand);
    // 发送成功后更新状态栏
    connect(m_sendController, &SendController::dataSent,
            this, [this](qint64) { updateStatusBar(); });
    // 发送状态消息（发送失败等）显示到状态栏
    connect(m_sendController, &SendController::statusMessage,
            this, [this](const QString& msg) {
                statusBar()->showMessage(msg, 3000);
            });

    // 工具栏信号 → 委托给对应处理方法
    connect(m_toolbarController, &ToolbarController::displayModeChanged,
            this, &MainWindow::onDisplayModeChanged);
    connect(m_toolbarController, &ToolbarController::timestampToggled,
            this, &MainWindow::onTimestampToggled);
    connect(m_toolbarController, &ToolbarController::dirPrefixToggled,
            this, [this](bool checked) {
                // 方向前缀开关: 同步到所有活动的终端widget
                for (auto* tw : m_layoutManager->terminalWidgets()) {
                    tw->setShowDirectionPrefix(checked);
                }
            });
    connect(m_toolbarController, &ToolbarController::clearRequested,
            this, &MainWindow::onClearTerminal);
    connect(m_toolbarController, &ToolbarController::exportRequested,
            this, &MainWindow::onExportData);
    connect(m_toolbarController, &ToolbarController::bgSettingsRequested,
            this, &MainWindow::onBgSettingsToggled);
    connect(m_toolbarController, &ToolbarController::themeChanged,
            m_settingsController, &SettingsController::onThemeChanged);
    connect(m_toolbarController, &ToolbarController::languageChanged,
            m_settingsController, &SettingsController::onLanguageChanged);
    connect(m_toolbarController, &ToolbarController::terminalLayoutChanged,
            this, &MainWindow::onTerminalLayoutChanged);

    // 录制/回放状态消息 → 状态栏显示
    connect(m_recordingController, &RecordingController::statusMessage,
            this, [this](const QString& msg, int timeoutMs) {
                statusBar()->showMessage(msg, timeoutMs);
            });
    // 回放数据写入终端（direction 0=接收, 1=发送）
    connect(m_recordingController, &RecordingController::playbackData,
            this, [this](const QByteArray& data, qint64 direction) {
                if (direction == 0) {
                    m_terminalModel->appendReceived(data);
                } else {
                    m_terminalModel->appendSent(data);
                }
                updateStatusBar();
            });

    // 搜索栏 → 终端搜索高亮（搜索只作用于主终端/混合模式终端）
    connect(m_panelManager->searchBar(), &TerminalSearchBar::searchRequested,
            this, &MainWindow::onSearchRequested);
    connect(m_panelManager->searchBar(), &TerminalSearchBar::searchCleared,
            this, &MainWindow::onSearchCleared);
    connect(m_panelManager->searchBar(), &TerminalSearchBar::closed, this, [this]() {
        m_panelManager->terminal()->clearSearchHighlight();
    });

    // 搜索匹配结果 → 搜索栏显示匹配计数（如 "3/15"）
    // 分栏模式下不连接分栏终端的搜索信号（搜索功能仅作用于主终端）
    connect(m_panelManager->terminal(), &TerminalWidget::searchMatchesChanged,
            this, [this](int total, int current) {
                m_panelManager->searchBar()->setResultText(total == 0 ? QString() :
                    tr("%1/%2").arg(current + 1).arg(total));
            });
    // 定时刷新数据统计面板
    connect(m_statsTimer, &QTimer::timeout, this, &MainWindow::updateDataStatistics);

    // 协议桥 → 协议视图 + 波形图（帧数据分发）
    connect(m_protocolBridgeMgr, &ProtocolBridgeManager::frameParsed,
            m_panelManager->protocolView(), &ProtocolView::onFrameParsed);
    connect(m_protocolBridgeMgr, &ProtocolBridgeManager::frameError,
            m_panelManager->protocolView(), &ProtocolView::onFrameError);
    connect(m_protocolBridgeMgr, &ProtocolBridgeManager::frameParsed,
            m_panelManager->chartWidget()->model(), &ChartModel::onFrameParsed);

    // 帧编辑器 → 更新帧解析器定义 + 波形图通道配置
    connect(m_panelManager->frameEditor(), &FrameVisualEditor::definitionChanged,
            this, [this](const FrameDefinition& def) {
                m_frameParser->setDefinition(def);
                m_panelManager->chartWidget()->configureFromFrameDefinition(def);
            });

    // 导航树点击 → 面板切换
    connect(m_navTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        QString text = index.data().toString();

        // 功能性节点（不走面板切换，直接触发动作）
        if (text == tr("数据导出")) { onExportData(); return; }
        if (text == tr("TCP客户端")) { m_connController->connectNetwork(ConnectionType::TcpClient); return; }
        if (text == tr("TCP服务端")) { m_connController->connectNetwork(ConnectionType::TcpServer); return; }
        if (text == tr("UDP")) { m_connController->connectNetwork(ConnectionType::Udp); return; }

        // 通过映射表查找目标面板 widget
        QWidget* target = m_navController->lookupPanel(text);

        // 未匹配的面板节点（如分组节点"串口""网络""工具"），忽略点击
        if (!target) return;

        // 委托 NavigationController 执行面板切换动画（淡出旧面板→淡入新面板）
        m_navController->switchToPanel(target);
    });
}

/**
 * @brief 终端显示模式切换
 * @param index 下拉框索引: 0=文本, 1=HEX, 2=混合, 3=十进制
 */
void MainWindow::onDisplayModeChanged(int index)
{
    DisplayMode modes[] = {DisplayMode::Text, DisplayMode::Hex, DisplayMode::Mixed, DisplayMode::Decimal};
    // 显示模式切换: 同步到所有活动的终端widget（主终端+分栏终端）
    for (auto* tw : m_layoutManager->terminalWidgets()) {
        tw->setDisplayMode(modes[index]);
    }
}

/**
 * @brief 时间戳显示开关
 * @param checked true=在每行终端数据前显示时间戳
 */
void MainWindow::onTimestampToggled(bool checked)
{
    // 时间戳开关: 同步到所有活动的终端widget
    for (auto* tw : m_layoutManager->terminalWidgets()) {
        tw->setShowTimestamp(checked);
    }
}

/** @brief 清空终端内容、重置数据统计、刷新状态栏 */
void MainWindow::onClearTerminal()
{
    m_terminalModel->clear();
    m_panelManager->dataStats()->reset();
    updateStatusBar();
}

/**
 * @brief 导出终端数据到文件
 *
 * 支持三种格式: .txt(纯文本), .csv(表格), .bin(原始二进制)
 * 使用批量流式导出，分批从 TerminalModel 拉取数据，避免一次性深拷贝全部行。
 */
void MainWindow::onExportData()
{
    if (m_terminalModel->lineCount() == 0) {
        QMessageBox::information(this, tr("Export"), tr("No data to export"));
        return;
    }

    QString filter = tr("Text files (*.txt);;CSV files (*.csv);;Binary files (*.bin)");
    QString filePath = QFileDialog::getSaveFileName(this, tr("Export Data"),
                                                     QString(), filter);
    if (filePath.isEmpty()) return;

    // 根据文件扩展名自动选择导出格式
    DataExporter::Format format = DataExporter::Txt;
    if (filePath.endsWith(".csv", Qt::CaseInsensitive))
        format = DataExporter::Csv;
    else if (filePath.endsWith(".bin", Qt::CaseInsensitive))
        format = DataExporter::Bin;

    // 批量流式导出: 通过 lineProvider 回调分批拉取数据，lines() 内部已加锁保证线程安全
    int totalLines = m_terminalModel->lineCount();
    auto lineProvider = [this](int offset, int count) -> QVector<TerminalLine> {
        return m_terminalModel->lines(offset, count);
    };

    if (m_dataExporter->exportStreamed(filePath, format, lineProvider, totalLines)) {
        statusBar()->showMessage(tr("Exported to %1").arg(filePath), 3000);
    } else {
        QMessageBox::warning(this, tr("Export Failed"), tr("Cannot write to file"));
    }
}

/**
 * @brief 终端搜索请求处理
 * @param pattern 搜索模式字符串
 * @param regex true=使用正则表达式匹配
 * @param hex true=按 HEX 字节搜索
 */
void MainWindow::onSearchRequested(const QString& pattern, bool regex, bool hex)
{
    m_panelManager->terminal()->setSearchHighlight(pattern, regex, hex);
}

/** @brief 清除终端搜索高亮 */
void MainWindow::onSearchCleared()
{
    m_panelManager->terminal()->clearSearchHighlight();
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
        // 从 BackgroundWidget 同步当前参数到弹出面板的控件
        m_bgSettingsPopup->syncFromWidget();
        // 定位到工具栏右侧下方
        QToolBar* tb = m_toolbarController->toolbar();
        QPoint pos = tb->mapToGlobal(QPoint(tb->width() - 270, tb->height() + 2));
        m_bgSettingsPopup->move(pos);
        m_bgSettingsPopup->show();
    }
}

/**
 * @brief 终端布局模式切换
 * @param index 下拉框索引: 0=混合, 1=左右分栏, 2=上下分栏
 *
 * 通过 TerminalLayoutManager 切换布局，分栏模式下创建两个独立的 TerminalWidget:
 * - RX终端: 只显示接收数据
 * - TX终端: 只显示发送数据
 * 两个终端共享同一个 TerminalModel，通过方向过滤器实现数据分流
 */
void MainWindow::onTerminalLayoutChanged(int index)
{
    m_layoutManager->setLayout(index);
}

/**
 * @brief 刷新状态栏中的 RX/TX 字节数显示
 * 自动格式化为 B/KB/MB 单位
 */
void MainWindow::updateStatusBar()
{
    if (m_terminalModel) {
        auto rx = m_terminalModel->rxBytes();
        auto tx = m_terminalModel->txBytes();
        // 字节数格式化: <1KB 显示 B, <1MB 显示 KB, 否则显示 MB
        auto formatBytes = [](quint64 bytes) -> QString {
            if (bytes < 1024) return QString("%1 B").arg(bytes);
            if (bytes < 1024 * 1024) return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
            return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
        };
        m_rxBytesLbl->setText("RX: " + formatBytes(rx));
        m_txBytesLbl->setText("TX: " + formatBytes(tx));
    }
}

/** @brief 定时刷新数据统计面板（由 m_statsTimer 每 500ms 触发） */
void MainWindow::updateDataStatistics()
{
    if (m_terminalModel && m_panelManager->dataStats()) {
        m_panelManager->dataStats()->update(m_terminalModel->rxBytes(), m_terminalModel->txBytes());
    }
}

/**
 * @brief 窗口关闭事件处理
 * 按顺序执行清理: 停止动画 → 停止录制/回放 → 保存设置 → 关闭连接
 * @param event 关闭事件
 */
void MainWindow::closeEvent(QCloseEvent* event)
{
    // 停止连接状态的呼吸动画
    m_navController->stopBreathingAnimation(m_connStatusLbl);

    // 停止录制/回放（防止后台线程写入已关闭文件）
    if (m_dataLogger->isRecording()) m_dataLogger->stopRecording();
    if (m_dataLogger->isPlaying()) m_dataLogger->stopPlayback();

    // 保存当前设置到磁盘（窗口几何、主题、串口配置）
    m_settingsController->saveSettings();

    // 关闭所有活跃连接
    auto connections = m_connManager->connections();
    for (auto* conn : connections) {
        conn->close();
    }
    event->accept();
}
