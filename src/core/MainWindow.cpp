#include "MainWindow.h"
#include "utils/HexConverter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QCloseEvent>
#include <QApplication>
#include <QSplitter>
#include <QFileDialog>
#include <QTimer>
#include <QStringListModel>
#include <QShortcut>
#include <QKeySequence>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_connManager(new ConnectionManager(this))
    , m_terminalModel(new TerminalModel(this))
    , m_timedSender(new TimedSender(this))
    , m_sendHistory(new SendHistory(this))
    , m_dataExporter(new DataExporter(this))
    , m_dataLogger(new DataLogger(this))
    , m_statsTimer(new QTimer(this))
    , m_frameParser(new FrameParser(this))
    , m_otaManager(new OtaManager(this))
{
    setupUI();
    setupToolbar();
    setupStatusBar();
    connectSignals();

    // 加载保存的设置（主题、窗口几何、串口配置）
    loadSettings();

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
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(m_mainSplitter);

    // ---- 左侧导航树 ----
    m_navTree = new QTreeView;
    m_navTree->setObjectName("navTree");
    m_navTree->setHeaderHidden(true);
    m_navTree->setMinimumWidth(180);
    m_navTree->setMaximumWidth(280);
    m_navTree->setIndentation(16);

    auto* treeModel = new QStandardItemModel(this);
    auto* rootItem = treeModel->invisibleRootItem();

    // 串口分组
    auto* serialItem = new QStandardItem(tr("串口"));
    serialItem->setEditable(false);
    auto* configItem = new QStandardItem(tr("配置"));
    configItem->setEditable(false);
    auto* terminalItem = new QStandardItem(tr("终端"));
    terminalItem->setEditable(false);
    auto* statsItem = new QStandardItem(tr("统计"));
    statsItem->setEditable(false);
    serialItem->appendRow(configItem);
    serialItem->appendRow(terminalItem);
    serialItem->appendRow(statsItem);
    auto* protocolItem = new QStandardItem(tr("协议"));
    protocolItem->setEditable(false);
    auto* frameEditorItem = new QStandardItem(tr("帧编辑器"));
    frameEditorItem->setEditable(false);
    auto* chartItem = new QStandardItem(tr("波形图"));
    chartItem->setEditable(false);
    serialItem->appendRow(protocolItem);
    serialItem->appendRow(frameEditorItem);
    serialItem->appendRow(chartItem);
    auto* otaItem = new QStandardItem(tr("OTA升级"));
    otaItem->setEditable(false);
    serialItem->appendRow(otaItem);

    // 网络分组
    auto* networkItem = new QStandardItem(tr("网络"));
    networkItem->setEditable(false);
    auto* tcpClientItem = new QStandardItem(tr("TCP客户端"));
    tcpClientItem->setEditable(false);
    auto* tcpServerItem = new QStandardItem(tr("TCP服务端"));
    tcpServerItem->setEditable(false);
    auto* udpItem = new QStandardItem(tr("UDP"));
    udpItem->setEditable(false);
    networkItem->appendRow(tcpClientItem);
    networkItem->appendRow(tcpServerItem);
    networkItem->appendRow(udpItem);

    // 工具分组
    auto* toolsItem = new QStandardItem(tr("工具"));
    toolsItem->setEditable(false);
    auto* exportItem = new QStandardItem(tr("数据导出"));
    exportItem->setEditable(false);
    toolsItem->appendRow(exportItem);

    rootItem->appendRow(serialItem);
    rootItem->appendRow(networkItem);
    rootItem->appendRow(toolsItem);

    m_navTree->setModel(treeModel);
    m_navTree->expandAll();

    m_mainSplitter->addWidget(m_navTree);

    // ---- 右侧内容面板 ----
    auto* rightWidget = new QWidget;
    auto* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    m_rightPanel = new QStackedWidget;
    rightLayout->addWidget(m_rightPanel, 1);

    // ---- 面板0: 串口配置 + 终端 ----
    auto* serialPanel = new QWidget;
    auto* serialLayout = new QVBoxLayout(serialPanel);
    serialLayout->setContentsMargins(0, 0, 0, 0);
    serialLayout->setSpacing(0);

    // 串口配置面板(点击"Config"时显示)
    m_serialConfig = new SerialConfigPanel;
    m_serialConfig->setVisible(false);
    serialLayout->addWidget(m_serialConfig);

    // 数据统计面板(点击"Statistics"时显示)
    m_dataStats = new DataStatistics;
    m_dataStats->setVisible(false);
    serialLayout->addWidget(m_dataStats);

    // 协议解析面板(点击"Protocol"时显示)
    m_protocolView = new ProtocolView;
    m_protocolView->setVisible(false);
    serialLayout->addWidget(m_protocolView);

    // 帧编辑器面板(点击"Frame Editor"时显示)
    m_frameEditor = new FrameVisualEditor;
    m_frameEditor->setVisible(false);
    serialLayout->addWidget(m_frameEditor);

    // 波形图面板(点击"Chart"时显示)
    m_chartWidget = new ChartWidget;
    m_chartWidget->setVisible(false);
    serialLayout->addWidget(m_chartWidget);

    // OTA升级面板(点击"OTA"时显示)
    m_otaWidget = new OtaWidget(m_otaManager);
    m_otaWidget->setVisible(false);
    serialLayout->addWidget(m_otaWidget);

    // 终端容器: 搜索栏 + 终端
    auto* terminalContainer = new QWidget;
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

    // 发送区域（带历史自动补全）
    auto* sendFrame = new QFrame;
    sendFrame->setFrameShape(QFrame::StyledPanel);
    auto* sendLayout = new QHBoxLayout(sendFrame);
    sendLayout->setContentsMargins(8, 4, 8, 4);

    m_sendModeCombo = new QComboBox;
    m_sendModeCombo->addItems({tr("文本"), tr("HEX")});
    m_sendModeCombo->setFixedWidth(60);

    m_sendInput = new QLineEdit;
    m_sendInput->setObjectName("sendInput");
    m_sendInput->setPlaceholderText(tr("输入要发送的数据..."));

    // 发送历史自动补全
    m_sendCompleter = new QCompleter(m_sendHistory->recentTexts(), this);
    m_sendCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_sendCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_sendInput->setCompleter(m_sendCompleter);

    m_sendBtn = new QPushButton(tr("发送"));
    m_sendBtn->setFixedWidth(70);

    sendLayout->addWidget(m_sendModeCombo);
    sendLayout->addWidget(m_sendInput, 1);
    sendLayout->addWidget(m_sendBtn);

    serialLayout->addWidget(sendFrame);

    m_rightPanel->addWidget(serialPanel);
    m_rightPanel->setCurrentIndex(0);

    m_mainSplitter->addWidget(rightWidget);
    m_mainSplitter->setSizes({200, 1000});
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);

    // Ctrl+F 快捷键激活搜索栏
    auto* searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(searchShortcut, &QShortcut::activated, m_searchBar, &TerminalSearchBar::activate);
}

void MainWindow::setupToolbar()
{
    m_toolbar = addToolBar(tr("主工具栏"));
    m_toolbar->setMovable(false);
    m_toolbar->setFloatable(false);

    m_displayModeCombo = new QComboBox;
    m_displayModeCombo->addItems({tr("文本"), tr("HEX"), tr("混合")});
    m_displayModeCombo->setFixedWidth(80);
    m_toolbar->addWidget(m_displayModeCombo);

    m_timestampAction = m_toolbar->addAction(tr("时间戳"));
    m_timestampAction->setCheckable(true);
    m_timestampAction->setChecked(false);

    m_clearAction = m_toolbar->addAction(tr("清屏"));

    m_toolbar->addSeparator();

    // 导出按钮
    m_exportAction = m_toolbar->addAction(tr("导出"));

    m_toolbar->addSeparator();

    // 日志录制按钮
    m_recordAction = m_toolbar->addAction(tr("录制"));
    m_recordAction->setCheckable(true);
    m_recordAction->setChecked(false);

    m_stopRecordAction = m_toolbar->addAction(tr("停止录制"));
    m_stopRecordAction->setEnabled(false);

    // 日志回放按钮
    m_playbackAction = m_toolbar->addAction(tr("回放日志"));
    m_stopPlaybackAction = m_toolbar->addAction(tr("停止回放"));
    m_stopPlaybackAction->setEnabled(false);

    m_toolbar->addSeparator();

    // 主题切换下拉框
    auto* themeLabel = new QLabel(tr(" 主题: "));
    m_toolbar->addWidget(themeLabel);

    m_themeCombo = new QComboBox;
    QStringList themes = ThemeManager::instance().availableThemes();
    for (const QString& name : themes) {
        // 显示友好名称: dark_terminal -> Dark Terminal
        QString display = name;
        display[0] = display[0].toUpper();
        // 将下划线替换为空格并大写每个单词首字母
        QStringList parts = display.split('_');
        for (auto& part : parts) {
            if (!part.isEmpty()) part[0] = part[0].toUpper();
        }
        m_themeCombo->addItem(parts.join(" "), name);
    }
    m_themeCombo->setFixedWidth(130);
    m_toolbar->addWidget(m_themeCombo);
}

void MainWindow::setupStatusBar()
{
    m_connStatusLbl = new QLabel(tr("未连接"));
    m_connStatusLbl->setObjectName("connStatus");
    m_connStatusLbl->setProperty("state", "disconnected");
    m_rxBytesLbl = new QLabel("RX: 0 B");
    m_txBytesLbl = new QLabel("TX: 0 B");

    statusBar()->addWidget(m_connStatusLbl, 1);
    statusBar()->addPermanentWidget(m_rxBytesLbl);
    statusBar()->addPermanentWidget(m_txBytesLbl);
}

void MainWindow::connectSignals()
{
    // 串口连接/断开
    connect(m_serialConfig, &SerialConfigPanel::connectRequested,
            this, &MainWindow::onConnectSerial);
    connect(m_serialConfig, &SerialConfigPanel::disconnectRequested,
            this, &MainWindow::onDisconnectSerial);

    // 发送按钮
    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSendData);
    connect(m_sendInput, &QLineEdit::returnPressed, this, &MainWindow::onSendData);

    // 快捷指令
    connect(m_quickCmdBar, &QuickCommandBar::commandTriggered,
            this, &MainWindow::onQuickCommand);

    // 工具栏
    connect(m_displayModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDisplayModeChanged);
    connect(m_timestampAction, &QAction::toggled, this, &MainWindow::onTimestampToggled);
    connect(m_clearAction, &QAction::triggered, this, &MainWindow::onClearTerminal);
    connect(m_exportAction, &QAction::triggered, this, &MainWindow::onExportData);

    // 日志录制/回放
    connect(m_recordAction, &QAction::toggled, this, &MainWindow::onToggleRecording);
    connect(m_stopRecordAction, &QAction::triggered, this, &MainWindow::onStopRecording);
    connect(m_playbackAction, &QAction::triggered, this, &MainWindow::onOpenPlayback);
    connect(m_stopPlaybackAction, &QAction::triggered, this, &MainWindow::onStopPlayback);
    connect(m_dataLogger, &DataLogger::playbackData,
            this, &MainWindow::onPlaybackData);
    connect(m_dataLogger, &DataLogger::playbackProgress,
            this, &MainWindow::onPlaybackProgress);
    connect(m_dataLogger, &DataLogger::recordingStopped,
            this, &MainWindow::onRecordingStopped);
    connect(m_dataLogger, &DataLogger::playbackFinished, this, [this]() {
        m_stopPlaybackAction->setEnabled(false);
        m_playbackAction->setEnabled(true);
        statusBar()->showMessage(tr("Playback finished"), 3000);
    });
    connect(m_dataLogger, &DataLogger::error, this, [this](const QString& msg) {
        statusBar()->showMessage(msg, 5000);
    });

    // 主题切换
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onThemeChanged);

    // 搜索栏
    connect(m_searchBar, &TerminalSearchBar::searchRequested,
            this, &MainWindow::onSearchRequested);
    connect(m_searchBar, &TerminalSearchBar::searchCleared,
            this, &MainWindow::onSearchCleared);
    connect(m_searchBar, &TerminalSearchBar::closed, this, [this]() {
        // 搜索栏关闭时清除终端搜索高亮（后续实现）
    });

    // 定时发送器
    connect(m_timedSender, &TimedSender::sendData, this, [this](const QByteArray& data) {
        if (m_currentConn && m_currentConn->state() == ConnectionState::Connected) {
            m_currentConn->write(data);
            m_terminalModel->appendSent(data);
            updateStatusBar();
        }
    });

    // 发送历史变化时更新自动补全
    connect(m_sendHistory, &SendHistory::historyChanged, this, [this]() {
        m_sendCompleter->setModel(new QStringListModel(m_sendHistory->recentTexts(), this));
    });

    // 统计刷新定时器
    connect(m_statsTimer, &QTimer::timeout, this, &MainWindow::updateDataStatistics);

    // 帧解析器 → 协议视图 + 波形图
    connect(m_frameParser, &FrameParser::frameParsed,
            m_protocolView, &ProtocolView::onFrameParsed);
    connect(m_frameParser, &FrameParser::frameError,
            m_protocolView, &ProtocolView::onFrameError);
    connect(m_frameParser, &FrameParser::frameParsed,
            m_chartWidget, &ChartWidget::onFrameParsed);

    // 帧编辑器 → 帧解析器（定义变更时更新解析器）
    connect(m_frameEditor, &FrameVisualEditor::definitionChanged,
            this, [this](const FrameDefinition& def) {
                m_frameParser->setDefinition(def);
            });

    // 导航树点击切换面板 — 使用map映射面板名称到widget，消除重复的setVisible调用
    connect(m_navTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        QString text = index.data().toString();

        // 面板映射: 导航名 → 对应的widget
        static const QVector<QPair<QString, QWidget*>> panels = {
            {tr("配置"), nullptr},       // 特殊处理，用m_serialConfig
            {tr("终端"), nullptr},     // 特殊处理，用m_terminal
            {tr("统计"), nullptr},
            {tr("协议"), nullptr},
            {tr("帧编辑器"), nullptr},
            {tr("波形图"), nullptr},
            {tr("OTA升级"), nullptr},
        };

        // 功能性节点（不走面板切换）
        if (text == tr("数据导出")) { onExportData(); return; }
        if (text == tr("TCP客户端")) { onConnectNetwork(ConnectionType::TcpClient); return; }
        if (text == tr("TCP服务端")) { onConnectNetwork(ConnectionType::TcpServer); return; }
        if (text == tr("UDP")) { onConnectNetwork(ConnectionType::Udp); return; }

        // 收集所有可切换面板widget
        QWidget* allPanels[] = {
            m_serialConfig, m_terminal, m_dataStats,
            m_protocolView, m_frameEditor, m_chartWidget, m_otaWidget
        };

        // 确定要显示的widget
        QWidget* target = nullptr;
        if (text == tr("配置"))        target = m_serialConfig;
        else if (text == tr("终端")) target = m_terminal;
        else if (text == tr("统计"))   target = m_dataStats;
        else if (text == tr("协议"))     target = m_protocolView;
        else if (text == tr("帧编辑器")) target = m_frameEditor;
        else if (text == tr("波形图"))        target = m_chartWidget;
        else if (text == tr("OTA升级"))          target = m_otaWidget;

        // 切换: 隐藏所有，只显示目标
        for (auto* w : allPanels) {
            if (w) w->setVisible(w == target);
        }
    });
}

void MainWindow::loadSettings()
{
    auto& settings = SettingsManager::instance();

    // 恢复窗口几何
    QByteArray geometry = settings.loadWindowGeometry();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }

    // 恢复主题
    QString savedTheme = settings.loadTheme();
    if (ThemeManager::instance().loadTheme(savedTheme)) {
        // 同步主题下拉框选中项
        for (int i = 0; i < m_themeCombo->count(); ++i) {
            if (m_themeCombo->itemData(i).toString() == savedTheme) {
                m_themeCombo->setCurrentIndex(i);
                break;
            }
        }
    }

    // 恢复串口配置到配置面板
    QVariantMap serialConfig = settings.loadSerialConfig();
    if (!serialConfig.isEmpty()) {
        m_serialConfig->restoreConfig(serialConfig);
    }
}

void MainWindow::saveSettings()
{
    auto& settings = SettingsManager::instance();

    // 保存窗口几何
    settings.saveWindowGeometry(saveGeometry());

    // 保存当前主题
    settings.saveTheme(ThemeManager::instance().currentTheme());

    // 保存串口配置（从配置面板获取当前值）
    QVariantMap serialConfig;
    serialConfig["portName"] = m_serialConfig->currentPortData();
    serialConfig["baudRate"] = m_serialConfig->currentBaudRate();
    serialConfig["dataBits"] = m_serialConfig->currentDataBitsIndex();
    serialConfig["parity"] = m_serialConfig->currentParityIndex();
    serialConfig["stopBits"] = m_serialConfig->currentStopBitsIndex();
    serialConfig["flowControl"] = m_serialConfig->currentFlowControlIndex();
    settings.saveSerialConfig(serialConfig);

    settings.sync();
}

void MainWindow::onConnectSerial()
{
    // 通过工厂创建连接（不依赖具体类型）
    m_currentConn = m_connManager->createConnection(ConnectionType::Serial);
    if (!m_currentConn) {
        QMessageBox::warning(this, tr("Not Supported"), tr("Serial connection not available"));
        return;
    }

    // 使用 IConnection::configure() 统一配置（消除强转）
    QVariantMap params;
    params["portName"] = m_serialConfig->currentPortData();
    params["baudRate"] = m_serialConfig->currentBaudRate();
    params["dataBits"] = m_serialConfig->currentDataBitsIndex() + 5;
    params["parity"] = m_serialConfig->currentParityIndex();
    params["stopBits"] = m_serialConfig->currentStopBitsIndex();
    params["flowControl"] = m_serialConfig->currentFlowControlIndex();
    params["dtr"] = true;
    params["rts"] = true;
    m_currentConn->configure(params);

    // 连接数据信号
    connect(m_currentConn, &IConnection::dataReceived,
            this, &MainWindow::onDataReceived);
    connect(m_currentConn, &IConnection::stateChanged,
            this, &MainWindow::onConnectionStateChanged);
    connect(m_currentConn, &IConnection::errorOccurred,
            this, [](const QString& msg) {
        qWarning() << "Connection error:" << msg;
    });

    // 尝试连接
    if (!m_currentConn->open()) {
        QMessageBox::warning(this, tr("Connection Failed"),
                             tr("Cannot open serial port"));
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;
        return;
    }

    // 同步连接到OTA管理器
    m_otaManager->setConnection(m_currentConn);
}

void MainWindow::onDisconnectSerial()
{
    if (m_currentConn) {
        m_currentConn->close();
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;
    }
}

void MainWindow::onSendData()
{
    if (!m_currentConn || m_currentConn->state() != ConnectionState::Connected) {
        return;
    }

    QString text = m_sendInput->text();
    if (text.isEmpty()) return;

    bool isHex = (m_sendModeCombo->currentIndex() == 1);
    QByteArray data;
    if (isHex) {
        data = HexConverter::fromHexString(text);
        if (data.isEmpty()) {
            m_sendInput->setProperty("hasError", true);
            m_sendInput->style()->unpolish(m_sendInput);
            m_sendInput->style()->polish(m_sendInput);
            return;
        }
    } else {
        data = text.toUtf8();
    }

    qint64 written = m_currentConn->write(data);
    if (written > 0) {
        m_terminalModel->appendSent(data);
        m_dataLogger->logData(data, DataLogger::Direction::Sent);
        m_sendHistory->addEntry(text, isHex);
        m_sendInput->clear();
        m_sendInput->setProperty("hasError", false);
        m_sendInput->style()->unpolish(m_sendInput);
        m_sendInput->style()->polish(m_sendInput);
        updateStatusBar();
    }
}

void MainWindow::onQuickCommand(const QByteArray& data)
{
    if (!m_currentConn || m_currentConn->state() != ConnectionState::Connected) {
        return;
    }
    m_currentConn->write(data);
    m_terminalModel->appendSent(data);
    m_dataLogger->logData(data, DataLogger::Direction::Sent);
    updateStatusBar();
}

void MainWindow::onDisplayModeChanged(int index)
{
    DisplayMode modes[] = {DisplayMode::Text, DisplayMode::Hex, DisplayMode::Mixed};
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

    if (m_dataExporter->exportToFile(filePath, format, m_terminalModel->lines())) {
        statusBar()->showMessage(tr("Exported to %1").arg(filePath), 3000);
    } else {
        QMessageBox::warning(this, tr("Export Failed"), tr("Cannot write to file"));
    }
}

void MainWindow::onSearchRequested(const QString& pattern, bool regex, bool hex)
{
    // 将搜索请求传递给终端模型（后续在TerminalModel中实现搜索高亮）
    // 目前仅做日志输出
    qDebug() << "Search:" << pattern << "regex:" << regex << "hex:" << hex;
}

void MainWindow::onSearchCleared()
{
    // 清除终端搜索高亮（后续实现）
}

void MainWindow::onThemeChanged(int index)
{
    QString themeName = m_themeCombo->itemData(index).toString();
    if (!themeName.isEmpty()) {
        ThemeManager::instance().loadTheme(themeName);
    }
}

void MainWindow::onConnectNetwork(ConnectionType type)
{
    m_currentConn = m_connManager->createConnection(type);
    if (!m_currentConn) {
        QMessageBox::warning(this, tr("Not Supported"), tr("This connection type is not yet available"));
        return;
    }

    // 默认网络参数配置
    QVariantMap params;
    if (type == ConnectionType::TcpClient) {
        params["mode"] = "client";
        params["host"] = "127.0.0.1";
        params["port"] = 8080;
    } else if (type == ConnectionType::TcpServer) {
        params["mode"] = "server";
        params["port"] = 8080;
    } else if (type == ConnectionType::Udp) {
        params["localPort"] = 8888;
        params["remoteHost"] = "127.0.0.1";
        params["remotePort"] = 8080;
    }
    m_currentConn->configure(params);

    connect(m_currentConn, &IConnection::dataReceived,
            this, &MainWindow::onDataReceived);
    connect(m_currentConn, &IConnection::stateChanged,
            this, &MainWindow::onConnectionStateChanged);
    connect(m_currentConn, &IConnection::errorOccurred,
            this, [](const QString& msg) {
        qWarning() << "Network connection error:" << msg;
    });

    if (!m_currentConn->open()) {
        QMessageBox::warning(this, tr("Connection Failed"),
                             tr("Cannot establish network connection"));
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;
        return;
    }
}

void MainWindow::onConnectionStateChanged(ConnectionState state)
{
    const char* stateStr = "";
    switch (state) {
    case ConnectionState::Connected:
        m_connStatusLbl->setText(tr("已连接: %1").arg(
            m_currentConn ? m_currentConn->name() : ""));
        stateStr = "connected";
        m_serialConfig->setConnected(true);
        m_serialConfig->setVisible(false);
        m_terminal->setVisible(true);
        m_dataStats->setVisible(false);
        break;
    case ConnectionState::Disconnected:
        m_connStatusLbl->setText(tr("未连接"));
        stateStr = "disconnected";
        m_serialConfig->setConnected(false);
        break;
    case ConnectionState::Connecting:
        m_connStatusLbl->setText(tr("连接中..."));
        stateStr = "connecting";
        break;
    case ConnectionState::Error:
        m_connStatusLbl->setText(tr("连接错误"));
        stateStr = "error";
        m_serialConfig->setConnected(false);
        break;
    }
    m_connStatusLbl->setProperty("state", stateStr);
    m_connStatusLbl->style()->unpolish(m_connStatusLbl);
    m_connStatusLbl->style()->polish(m_connStatusLbl);
}

void MainWindow::onDataReceived(const QByteArray& data)
{
    m_terminalModel->appendReceived(data);
    m_frameParser->feed(data);
    m_dataLogger->logData(data, DataLogger::Direction::Received);
    updateStatusBar();
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
    // 停止录制/回放
    if (m_dataLogger->isRecording()) m_dataLogger->stopRecording();
    if (m_dataLogger->isPlaying()) m_dataLogger->stopPlayback();

    // 保存设置到磁盘
    saveSettings();

    // 关闭所有连接
    auto connections = m_connManager->connections();
    for (auto* conn : connections) {
        conn->close();
    }
    event->accept();
}

void MainWindow::onToggleRecording()
{
    if (m_dataLogger->isRecording()) {
        // 正在录制 → 暂停
        if (m_dataLogger->isPaused()) {
            m_dataLogger->resumeRecording();
            m_recordAction->setText(tr("Pause"));
        } else {
            m_dataLogger->pauseRecording();
            m_recordAction->setText(tr("Resume"));
        }
    } else {
        // 开始录制
        QString filter = tr("EmbedDebug Log (*.edl);;All files (*.*)");
        QString path = QFileDialog::getSaveFileName(this, tr("Record Log"),
                                                     QString(), filter);
        if (path.isEmpty()) {
            m_recordAction->setChecked(false);
            return;
        }
        if (!path.endsWith(".edl")) path += ".edl";

        m_dataLogger->startRecording(path);
        m_stopRecordAction->setEnabled(true);
        m_recordAction->setText(tr("Pause"));
        statusBar()->showMessage(tr("Recording: %1").arg(path));
    }
}

void MainWindow::onStopRecording()
{
    m_dataLogger->stopRecording();
    m_recordAction->setChecked(false);
    m_recordAction->setText(tr("Record"));
    m_stopRecordAction->setEnabled(false);
}

void MainWindow::onOpenPlayback()
{
    QString filter = tr("EmbedDebug Log (*.edl);;All files (*.*)");
    QString path = QFileDialog::getOpenFileName(this, tr("Open Log for Playback"),
                                                  QString(), filter);
    if (path.isEmpty()) return;

    m_dataLogger->startPlayback(path);
    m_stopPlaybackAction->setEnabled(true);
    m_playbackAction->setEnabled(false);
    statusBar()->showMessage(tr("Playing: %1").arg(path));
}

void MainWindow::onStopPlayback()
{
    m_dataLogger->stopPlayback();
    m_stopPlaybackAction->setEnabled(false);
    m_playbackAction->setEnabled(true);
}

void MainWindow::onPlaybackData(const QByteArray& data, qint64 direction)
{
    if (direction == 0) {
        m_terminalModel->appendReceived(data);
    } else {
        m_terminalModel->appendSent(data);
    }
    updateStatusBar();
}

void MainWindow::onPlaybackProgress(qreal percent)
{
    statusBar()->showMessage(tr("Playback: %1%").arg(static_cast<int>(percent * 100)));
}

void MainWindow::onRecordingStopped(const QString& filePath, int count, qint64 durationMs)
{
    statusBar()->showMessage(
        tr("Recording saved: %1 (%2 records, %3s)")
            .arg(filePath)
            .arg(count)
            .arg(durationMs / 1000.0, 0, 'f', 1),
        5000);
}
