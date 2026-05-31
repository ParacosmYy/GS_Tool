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
    , m_otaManager(new OtaManager(this))
    , m_navController(new NavigationController(this))
{
    // 依赖注入: ConnectionController 需要通知 SendController/OtaManager/RecordingController
    m_connController->setSendController(m_sendController);
    m_connController->setOtaManager(m_otaManager);
    m_connController->setRecordingController(m_recordingController);

    setupUI();
    setupToolbar();
    setupStatusBar();
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
    auto* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    m_rightPanel = new QWidget;
    auto* rightPanelLayout = new QVBoxLayout(m_rightPanel);
    rightPanelLayout->setContentsMargins(0, 0, 0, 0);
    rightPanelLayout->setSpacing(0);
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

    // 发送区域: 由SendController创建和管理
    QWidget* sendBar = m_sendController->createSendBar(this);
    serialLayout->addWidget(sendBar);

    rightPanelLayout->addWidget(serialPanel);

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

    // 日志录制/回放按钮（委托给RecordingController管理）
    m_recordingController->setupActions(m_toolbar);

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

    // 语言切换下拉框
    auto* langLabel = new QLabel(tr(" 语言: "));
    m_toolbar->addWidget(langLabel);

    m_langCombo = new QComboBox;
    m_langCombo->addItem(QStringLiteral("中文"), Language::CHINESE);
    m_langCombo->addItem(QStringLiteral("English"), Language::ENGLISH);
    m_langCombo->setFixedWidth(90);
    m_toolbar->addWidget(m_langCombo);
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
        params["dtr"] = true;
        params["rts"] = true;
        m_connController->connectSerial(params);
    });
    connect(m_serialConfig, &SerialConfigPanel::disconnectRequested,
            m_connController, &ConnectionController::disconnectSerial);

    // ConnectionController → MainWindow UI 更新
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
        m_frameParser->feed(data);
        m_dataLogger->logData(data, DataLogger::Direction::Received);
    });

    connect(m_connController, &ConnectionController::statusBarUpdateRequested,
            this, &MainWindow::updateStatusBar);

    // 连接失败弹窗
    connect(m_connController, &ConnectionController::connectionFailed,
            this, [this](const QString& title, const QString& message) {
        QMessageBox::warning(this, title, message);
    });

    // 快捷指令 → SendController
    connect(m_quickCmdBar, &QuickCommandBar::commandTriggered,
            m_sendController, &SendController::onQuickCommand);

    // SendController信号 → MainWindow状态栏更新
    connect(m_sendController, &SendController::dataSent,
            this, [this](qint64) { updateStatusBar(); });
    connect(m_sendController, &SendController::statusMessage,
            this, [this](const QString& msg) {
                statusBar()->showMessage(msg, 3000);
            });

    // 工具栏
    connect(m_displayModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDisplayModeChanged);
    connect(m_timestampAction, &QAction::toggled, this, &MainWindow::onTimestampToggled);
    connect(m_clearAction, &QAction::triggered, this, &MainWindow::onClearTerminal);
    connect(m_exportAction, &QAction::triggered, this, &MainWindow::onExportData);

    // 日志录制/回放: RecordingController内部已连接DataLogger信号
    // 仅连接外部通知信号 → MainWindow
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

    // 主题切换
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onThemeChanged);

    // 语言切换
    connect(m_langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLanguageChanged);

    // 搜索栏
    connect(m_searchBar, &TerminalSearchBar::searchRequested,
            this, &MainWindow::onSearchRequested);
    connect(m_searchBar, &TerminalSearchBar::searchCleared,
            this, &MainWindow::onSearchCleared);
    connect(m_searchBar, &TerminalSearchBar::closed, this, [this]() {
        // 搜索栏关闭时清除终端搜索高亮（后续实现）
    });

    // 统计刷新定时器
    connect(m_statsTimer, &QTimer::timeout, this, &MainWindow::updateDataStatistics);

    // 帧解析器 → 协议视图 + 波形图数据模型
    connect(m_frameParser, &FrameParser::frameParsed,
            m_protocolView, &ProtocolView::onFrameParsed);
    connect(m_frameParser, &FrameParser::frameError,
            m_protocolView, &ProtocolView::onFrameError);
    // 帧数据通过ChartModel分发，ChartWidget内部连接model信号刷新渲染
    connect(m_frameParser, &FrameParser::frameParsed,
            m_chartWidget->model(), &ChartModel::onFrameParsed);

    // 帧编辑器 → 帧解析器 + 波形图通道配置（定义变更时同步更新）
    connect(m_frameEditor, &FrameVisualEditor::definitionChanged,
            this, [this](const FrameDefinition& def) {
                m_frameParser->setDefinition(def);
                // 自动根据帧定义生成通道配置
                m_chartWidget->configureFromFrameDefinition(def);
            });

    // 导航树点击切换面板 — 委托NavigationController处理面板切换
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

    // 恢复语言选择
    QString savedLang = settings.loadLanguage();
    for (int i = 0; i < m_langCombo->count(); ++i) {
        if (m_langCombo->itemData(i).toString() == savedLang) {
            m_langCombo->setCurrentIndex(i);
            break;
        }
    }
}

void MainWindow::saveSettings()
{
    auto& settings = SettingsManager::instance();

    // 保存窗口几何
    settings.saveWindowGeometry(saveGeometry());

    // 保存当前主题
    settings.saveTheme(ThemeManager::instance().currentTheme());

    // 保存语言选择（已在onLanguageChanged中实时保存，此处确保一致性）

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

void MainWindow::onLanguageChanged(int index)
{
    QString langCode = m_langCombo->itemData(index).toString();
    SettingsManager::instance().saveLanguage(langCode);

    // 用硬编码字符串而非tr()，因为翻译此刻尚未生效
    if (langCode == Language::ENGLISH) {
        statusBar()->showMessage(QStringLiteral("Language changed to English, restart to apply"), 3000);
    } else {
        statusBar()->showMessage(QStringLiteral("语言已切换为中文，重启后生效"), 3000);
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
    saveSettings();

    // 关闭所有连接
    auto connections = m_connManager->connections();
    for (auto* conn : connections) {
        conn->close();
    }
    event->accept();
}
