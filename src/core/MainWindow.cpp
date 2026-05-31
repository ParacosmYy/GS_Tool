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
    , m_navController(new NavigationController(this))
    , m_toolbarController(new ToolbarController(m_recordingController, this))
    , m_settingsController(new SettingsController(this, this))
{
    // 依赖注入: ConnectionController 需要通知 SendController/OtaManager/RecordingController
    m_connController->setSendController(m_sendController);
    m_connController->setOtaManager(m_otaManager);
    m_connController->setRecordingController(m_recordingController);

    setupUI();
    m_toolbarController->createToolbar(this);
    setupStatusBar();

    // 背景设置弹出面板
    m_bgSettingsPopup = new BackgroundSettingsPopup(m_backgroundWidget, this);

    connectSignals();

    // 构建面板映射表并传递给NavigationController构建导航树
    m_navController->buildNavTree(m_navTree, {
        {QT_TRANSLATE_NOOP("MainWindow", "配置"),       m_serialConfig},
        {QT_TRANSLATE_NOOP("MainWindow", "终端"),       m_terminal},
        {QT_TRANSLATE_NOOP("MainWindow", "统计"),       m_dataStats},
        {QT_TRANSLATE_NOOP("MainWindow", "协议"),       m_protocolView},
        {QT_TRANSLATE_NOOP("MainWindow", "帧编辑器"),   m_frameEditor},
        {QT_TRANSLATE_NOOP("MainWindow", "波形图"),     m_chartWidget},
        {QT_TRANSLATE_NOOP("MainWindow", "OTA升级"),    m_otaWidget},
    });

    // 初始面板状态: 终端为默认可见面板
    m_navController->setCurrentPanel(m_terminal);

    // 注入UI引用到SettingsController（通过ToolbarController接口同步主题/语言）
    m_settingsController->setToolbarController(m_toolbarController);
    m_settingsController->setSerialConfigPanel(m_serialConfig);

    // 加载保存的设置（主题、窗口几何、串口配置、语言）
    m_settingsController->loadSettings();

    // 统计刷新定时器: 每500ms刷新一次
    m_statsTimer->setInterval(500);
    m_statsTimer->start();

    // 设置窗口属性
    setWindowTitle(App::APP_NAME);
    resize(1200, 800);
    setMinimumSize(900, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // ---- 背景: 磨砂玻璃背景层 ----
    m_backgroundWidget = new BackgroundWidget(this);
    setCentralWidget(m_backgroundWidget);
    auto* bgLayout = new QVBoxLayout(m_backgroundWidget);
    bgLayout->setContentsMargins(0, 0, 0, 0);
    bgLayout->setSpacing(0);

    m_mainSplitter = new QSplitter(Qt::Horizontal, m_backgroundWidget);

    // ---- 左侧导航树（模型由NavigationController构建） ----
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
    auto* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    m_rightPanel = new QWidget;
    m_rightPanel->setObjectName("rightPanel");
    auto* rightPanelLayout = new QVBoxLayout(m_rightPanel);
    rightPanelLayout->setContentsMargins(0, 0, 0, 0);
    rightPanelLayout->setSpacing(0);
    rightLayout->addWidget(m_rightPanel, 1);

    // 面板: 串口配置 + 终端
    auto* serialPanel = new QWidget;
    serialPanel->setObjectName("serialPanel");
    auto* serialLayout = new QVBoxLayout(serialPanel);
    serialLayout->setContentsMargins(0, 0, 0, 0);
    serialLayout->setSpacing(0);

    m_serialConfig = new SerialConfigPanel;
    m_serialConfig->setVisible(false);
    serialLayout->addWidget(m_serialConfig);

    m_dataStats = new DataStatistics;
    m_dataStats->setVisible(false);
    serialLayout->addWidget(m_dataStats);

    m_protocolView = new ProtocolView;
    m_protocolView->setVisible(false);
    serialLayout->addWidget(m_protocolView);

    m_frameEditor = new FrameVisualEditor;
    m_frameEditor->setVisible(false);
    serialLayout->addWidget(m_frameEditor);

    m_chartWidget = new ChartWidget;
    m_chartWidget->setVisible(false);
    serialLayout->addWidget(m_chartWidget);

    m_otaWidget = new OtaWidget(m_otaManager);
    m_otaWidget->setVisible(false);
    serialLayout->addWidget(m_otaWidget);

    // 终端容器: 搜索栏 + 终端
    auto* terminalContainer = new QWidget;
    terminalContainer->setObjectName("terminalContainer");
    auto* terminalLayout = new QVBoxLayout(terminalContainer);
    terminalLayout->setContentsMargins(0, 0, 0, 0);
    terminalLayout->setSpacing(0);

    // 搜索栏嵌入终端顶部
    m_searchBar = new TerminalSearchBar(terminalContainer);
    terminalLayout->addWidget(m_searchBar);

    // 终端显示区
    m_terminal = new TerminalWidget;
    m_terminal->setModel(m_terminalModel);
    terminalLayout->addWidget(m_terminal, 1);

    serialLayout->addWidget(terminalContainer, 1);

    // 快捷指令栏
    m_quickCmdBar = new QuickCommandBar;
    m_quickCmdBar->setCommands({
        {"AT", "AT\r\n", false},
        {"Reset", "AA 55 01 00 FE", true},
        {"Status", "AT+STATUS?\r\n", false}
    });
    serialLayout->addWidget(m_quickCmdBar);

    // 发送区域: 由SendController创建和管理
    QWidget* sendBar = m_sendController->createSendBar(this);
    serialLayout->addWidget(sendBar);

    rightPanelLayout->addWidget(serialPanel);

    m_mainSplitter->addWidget(rightWidget);
    m_mainSplitter->setSizes({200, 1000});
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);

    // 将splitter放入背景层布局
    bgLayout->addWidget(m_mainSplitter, 1);

    // Ctrl+F 快捷键激活搜索栏
    auto* searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(searchShortcut, &QShortcut::activated, m_searchBar, &TerminalSearchBar::activate);
}

void MainWindow::setupStatusBar()
{
    m_connStatusLbl = new QLabel(tr("未连接"));
    m_connStatusLbl->setObjectName("connStatus");
    m_connStatusLbl->setProperty("state", "disconnected");
    m_rxBytesLbl = new QLabel("RX: 0 B");
    m_rxBytesLbl->setObjectName("rxBytesLabel");
    m_txBytesLbl = new QLabel("TX: 0 B");
    m_txBytesLbl->setObjectName("txBytesLabel");

    statusBar()->addWidget(m_connStatusLbl, 1);
    statusBar()->addPermanentWidget(m_rxBytesLbl);
    statusBar()->addPermanentWidget(m_txBytesLbl);
}

void MainWindow::connectSignals()
{
    // ---- 串口连接/断开: 委托ConnectionController ----
    connect(m_serialConfig, &SerialConfigPanel::connectRequested,
            this, [this]() {
        QVariantMap params;
        params["portName"] = m_serialConfig->currentPortData();
        params["baudRate"] = m_serialConfig->currentBaudRate();
        params["dataBits"] = m_serialConfig->currentDataBitsIndex() + 5;
        params["parity"] = m_serialConfig->currentParityIndex();
        params["stopBits"] = m_serialConfig->currentStopBitsIndex();
        params["flowControl"] = m_serialConfig->currentFlowControlIndex();
        params["dtr"] = m_serialConfig->dtrEnabled();
        params["rts"] = m_serialConfig->rtsEnabled();
        m_connController->connectSerial(params);
    });
    connect(m_serialConfig, &SerialConfigPanel::disconnectRequested,
            m_connController, &ConnectionController::disconnectSerial);
    connect(m_serialConfig, &SerialConfigPanel::dtrChanged,
            m_connController, &ConnectionController::setDtr);
    connect(m_serialConfig, &SerialConfigPanel::rtsChanged,
            m_connController, &ConnectionController::setRts);

    // 连接状态 → UI更新
    connect(m_connController, &ConnectionController::connectionStateChanged,
            this, [this](ConnectionState state, const QString& connName) {
        const char* stateStr = "";
        switch (state) {
        case ConnectionState::Connected:
            m_connStatusLbl->setText(tr("已连接: %1").arg(connName));
            stateStr = "connected";
            m_serialConfig->setConnected(true);
            // 连接成功后自动切换到终端面板（带动画）
            m_navController->switchToPanel(m_terminal);
            m_navController->stopBreathingAnimation(m_connStatusLbl);
            break;
        case ConnectionState::Disconnected:
            m_connStatusLbl->setText(tr("未连接"));
            stateStr = "disconnected";
            m_serialConfig->setConnected(false);
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
            m_serialConfig->setConnected(false);
            m_navController->stopBreathingAnimation(m_connStatusLbl);
            break;
        }
        m_connStatusLbl->setProperty("state", stateStr);
        m_connStatusLbl->style()->unpolish(m_connStatusLbl);
        m_connStatusLbl->style()->polish(m_connStatusLbl);
    });

    connect(m_connController, &ConnectionController::dataReceived,
            this, [this](const QByteArray& data) {
        m_terminalModel->appendReceived(data);
        m_protocolBridgeMgr->feedData(data);
        m_dataLogger->logData(data, DataLogger::Direction::Received);
    });

    connect(m_connController, &ConnectionController::statusBarUpdateRequested,
            this, &MainWindow::updateStatusBar);
    connect(m_connController, &ConnectionController::connectionFailed,
            this, [this](const QString& title, const QString& message) {
        QMessageBox::warning(this, title, message);
    });

    // 快捷指令 + 发送
    connect(m_quickCmdBar, &QuickCommandBar::commandTriggered,
            m_sendController, &SendController::onQuickCommand);
    connect(m_sendController, &SendController::dataSent,
            this, [this](qint64) { updateStatusBar(); });
    connect(m_sendController, &SendController::statusMessage,
            this, [this](const QString& msg) {
                statusBar()->showMessage(msg, 3000);
            });

    // 工具栏信号 -> 委托ToolbarController转发
    connect(m_toolbarController, &ToolbarController::displayModeChanged,
            this, &MainWindow::onDisplayModeChanged);
    connect(m_toolbarController, &ToolbarController::timestampToggled,
            this, &MainWindow::onTimestampToggled);
    connect(m_toolbarController, &ToolbarController::dirPrefixToggled,
            m_terminal, &TerminalWidget::setShowDirectionPrefix);
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

    // 录制/回放
    connect(m_recordingController, &RecordingController::statusMessage,
            this, [this](const QString& msg, int timeoutMs) {
                statusBar()->showMessage(msg, timeoutMs);
            });
    connect(m_recordingController, &RecordingController::playbackData,
            this, [this](const QByteArray& data, qint64 direction) {
                if (direction == 0) {
                    m_terminalModel->appendReceived(data);
                } else {
                    m_terminalModel->appendSent(data);
                }
                updateStatusBar();
            });

    // 搜索栏
    connect(m_searchBar, &TerminalSearchBar::searchRequested,
            this, &MainWindow::onSearchRequested);
    connect(m_searchBar, &TerminalSearchBar::searchCleared,
            this, &MainWindow::onSearchCleared);
    connect(m_searchBar, &TerminalSearchBar::closed, this, [this]() {
        m_terminal->clearSearchHighlight();
    });

    // 搜索匹配结果 → 搜索栏显示
    connect(m_terminal, &TerminalWidget::searchMatchesChanged,
            this, [this](int total, int current) {
                m_searchBar->setResultText(total == 0 ? QString() :
                    tr("%1/%2").arg(current + 1).arg(total));
            });
    connect(m_statsTimer, &QTimer::timeout, this, &MainWindow::updateDataStatistics);

    // 协议桥 → 视图 + 波形图
    connect(m_protocolBridgeMgr, &ProtocolBridgeManager::frameParsed,
            m_protocolView, &ProtocolView::onFrameParsed);
    connect(m_protocolBridgeMgr, &ProtocolBridgeManager::frameError,
            m_protocolView, &ProtocolView::onFrameError);
    connect(m_protocolBridgeMgr, &ProtocolBridgeManager::frameParsed,
            m_chartWidget->model(), &ChartModel::onFrameParsed);

    // 帧编辑器 → 帧解析器 + 波形图
    connect(m_frameEditor, &FrameVisualEditor::definitionChanged,
            this, [this](const FrameDefinition& def) {
                m_frameParser->setDefinition(def);
                m_chartWidget->configureFromFrameDefinition(def);
            });

    // 导航树点击切换面板
    connect(m_navTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        QString text = index.data().toString();

        // 功能性节点（不走面板切换，直接触发动作）
        if (text == tr("数据导出")) { onExportData(); return; }
        if (text == tr("TCP客户端")) { m_connController->connectNetwork(ConnectionType::TcpClient); return; }
        if (text == tr("TCP服务端")) { m_connController->connectNetwork(ConnectionType::TcpServer); return; }
        if (text == tr("UDP")) { m_connController->connectNetwork(ConnectionType::Udp); return; }

        // 通过NavigationController映射表查找目标面板widget
        QWidget* target = m_navController->lookupPanel(text);

        // 未匹配的面板节点（如分组节点"串口""网络""工具"），忽略
        if (!target) return;

        // 委托NavigationController执行面板切换动画
        m_navController->switchToPanel(target);
    });
}

void MainWindow::onDisplayModeChanged(int index)
{
    DisplayMode modes[] = {DisplayMode::Text, DisplayMode::Hex, DisplayMode::Mixed, DisplayMode::Decimal};
    m_terminal->setDisplayMode(modes[index]);
}

void MainWindow::onTimestampToggled(bool checked)
{
    m_terminal->setShowTimestamp(checked);
}

void MainWindow::onClearTerminal()
{
    m_terminalModel->clear();
    m_dataStats->reset();
    updateStatusBar();
}

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

    // 根据扩展名选择格式
    DataExporter::Format format = DataExporter::Txt;
    if (filePath.endsWith(".csv", Qt::CaseInsensitive))
        format = DataExporter::Csv;
    else if (filePath.endsWith(".bin", Qt::CaseInsensitive))
        format = DataExporter::Bin;

    // 使用批量流式导出，分批从 TerminalModel 拉取数据，避免一次性深拷贝全部行
    // lines(offset, count) 内部已加锁，线程安全
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

void MainWindow::onSearchRequested(const QString& pattern, bool regex, bool hex)
{
    m_terminal->setSearchHighlight(pattern, regex, hex);
}

void MainWindow::onSearchCleared()
{
    m_terminal->clearSearchHighlight();
}

void MainWindow::onBgSettingsToggled()
{
    if (m_bgSettingsPopup->isVisible()) {
        m_bgSettingsPopup->hide();
    } else {
        m_bgSettingsPopup->syncFromWidget();
        // 在工具栏下方弹出
        QToolBar* tb = m_toolbarController->toolbar();
        QPoint pos = tb->mapToGlobal(QPoint(tb->width() - 270, tb->height() + 2));
        m_bgSettingsPopup->move(pos);
        m_bgSettingsPopup->show();
    }
}

void MainWindow::updateStatusBar()
{
    if (m_terminalModel) {
        auto rx = m_terminalModel->rxBytes();
        auto tx = m_terminalModel->txBytes();
        auto formatBytes = [](quint64 bytes) -> QString {
            if (bytes < 1024) return QString("%1 B").arg(bytes);
            if (bytes < 1024 * 1024) return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
            return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
        };
        m_rxBytesLbl->setText("RX: " + formatBytes(rx));
        m_txBytesLbl->setText("TX: " + formatBytes(tx));
    }
}

void MainWindow::updateDataStatistics()
{
    if (m_terminalModel && m_dataStats) {
        m_dataStats->update(m_terminalModel->rxBytes(), m_terminalModel->txBytes());
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // 停止呼吸动画
    m_navController->stopBreathingAnimation(m_connStatusLbl);

    // 停止录制/回放
    if (m_dataLogger->isRecording()) m_dataLogger->stopRecording();
    if (m_dataLogger->isPlaying()) m_dataLogger->stopPlayback();

    // 保存设置到磁盘
    m_settingsController->saveSettings();

    // 关闭所有连接
    auto connections = m_connManager->connections();
    for (auto* conn : connections) {
        conn->close();
    }
    event->accept();
}
