#include "MainWindow.h"
#include "chart/ChartModel.h"
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
#include <QShortcut>
#include <QKeySequence>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTranslator>
#include <QDir>
#include <QPainter>
#include <QPixmap>

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
    buildNavPanelMappings();
    connectSignals();

    // 初始面板状态: 终端为默认可见面板
    m_currentPanel = m_terminal;

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

// 创建导航树连接类型指示圆点图标（8x8透明底+抗锯齿彩色圆）
static QIcon createDotIcon(const QColor& color)
{
    QPixmap dot(8, 8);
    dot.fill(Qt::transparent);
    QPainter painter(&dot);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(1, 1, 6, 6);
    return QIcon(dot);
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

    // 串口分组 — 蓝色圆点
    auto* serialItem = new QStandardItem(createDotIcon(QColor(NavColors::kSerialDot)), tr("串口"));
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
    // TCP客户端/服务端 — 绿色圆点
    auto* tcpClientItem = new QStandardItem(createDotIcon(QColor(NavColors::kTcpDot)), tr("TCP客户端"));
    tcpClientItem->setEditable(false);
    auto* tcpServerItem = new QStandardItem(createDotIcon(QColor(NavColors::kTcpDot)), tr("TCP服务端"));
    tcpServerItem->setEditable(false);
    // UDP — 黄色圆点
    auto* udpItem = new QStandardItem(createDotIcon(QColor(NavColors::kUdpDot)), tr("UDP"));
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

    // 发送历史自动补全（复用同一个QStringListModel，避免每次new泄漏）
    m_sendCompleterModel = new QStringListModel(m_sendHistory->recentTexts(), this);
    m_sendCompleter = new QCompleter(m_sendCompleterModel, this);
    m_sendCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_sendCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_sendInput->setCompleter(m_sendCompleter);

    m_sendBtn = new QPushButton(tr("发送"));
    m_sendBtn->setObjectName("sendButton");
    m_sendBtn->setFixedWidth(70);

    sendLayout->addWidget(m_sendModeCombo);
    sendLayout->addWidget(m_sendInput, 1);
    sendLayout->addWidget(m_sendBtn);

    serialLayout->addWidget(sendFrame);

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

void MainWindow::buildNavPanelMappings()
{
    // 映射表: 导航树叶子节点名称 → 对应的面板widget
    // name 字段是翻译键，运行时通过 tr(name) 匹配导航树中翻译后的文本
    m_navPanelMappings = {
        {QT_TRANSLATE_NOOP("MainWindow", "配置"),       m_serialConfig},
        {QT_TRANSLATE_NOOP("MainWindow", "终端"),       m_terminal},
        {QT_TRANSLATE_NOOP("MainWindow", "统计"),       m_dataStats},
        {QT_TRANSLATE_NOOP("MainWindow", "协议"),       m_protocolView},
        {QT_TRANSLATE_NOOP("MainWindow", "帧编辑器"),   m_frameEditor},
        {QT_TRANSLATE_NOOP("MainWindow", "波形图"),     m_chartWidget},
        {QT_TRANSLATE_NOOP("MainWindow", "OTA升级"),    m_otaWidget},
    };
}

QVector<QWidget*> MainWindow::allSwitchablePanels() const
{
    return {m_serialConfig, m_terminal, m_dataStats,
            m_protocolView, m_frameEditor, m_chartWidget, m_otaWidget};
}

void MainWindow::switchToPanel(QWidget* newPanel)
{
    // 防止动画期间重复触发切换
    if (m_panelSwitching) return;

    // 如果目标是当前已显示的面板，无需切换
    if (m_currentPanel == newPanel) return;

    QWidget* oldPanel = m_currentPanel;

    // 更新当前面板追踪
    m_currentPanel = newPanel;

    // 隐藏所有非当前、非旧面板，并清除残留的 opacity effect
    for (auto* w : allSwitchablePanels()) {
        if (w && w != newPanel && w != oldPanel) {
            if (w->graphicsEffect()) {
                w->setGraphicsEffect(nullptr);
            }
            w->setVisible(false);
        }
    }

    if (!newPanel) return;

    // 如果有旧面板且旧面板可见，执行: 淡出旧面板 → 显示新面板 → 淡入新面板
    if (oldPanel && oldPanel->isVisible()) {
        m_panelSwitching = true;

        // 旧面板: 200ms InCubic opacity 1.0 → 0.0 淡出
        QGraphicsOpacityEffect* fadeOutEffect = new QGraphicsOpacityEffect(oldPanel);
        oldPanel->setGraphicsEffect(fadeOutEffect);

        QPropertyAnimation* fadeOut = new QPropertyAnimation(fadeOutEffect, "opacity");
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        fadeOut->setDuration(200);
        fadeOut->setEasingCurve(QEasingCurve::InCubic);

        // 淡出完成后: 隐藏旧面板 → 显示新面板 → 淡入新面板
        connect(fadeOut, &QPropertyAnimation::finished, this, [this, oldPanel, newPanel]() {
            // 清除旧面板的 effect 并隐藏
            if (oldPanel->graphicsEffect()) {
                oldPanel->setGraphicsEffect(nullptr);
            }
            oldPanel->setVisible(false);

            // 显示新面板
            newPanel->setVisible(true);

            // 新面板: 250ms OutCubic opacity 0.0 → 1.0 淡入
            QGraphicsOpacityEffect* fadeInEffect = new QGraphicsOpacityEffect(newPanel);
            fadeInEffect->setOpacity(0.0);
            newPanel->setGraphicsEffect(fadeInEffect);

            QPropertyAnimation* fadeIn = new QPropertyAnimation(fadeInEffect, "opacity");
            fadeIn->setStartValue(0.0);
            fadeIn->setEndValue(1.0);
            fadeIn->setDuration(250);
            fadeIn->setEasingCurve(QEasingCurve::OutCubic);

            // 淡入完成后清除 effect，恢复正常绘制性能
            connect(fadeIn, &QPropertyAnimation::finished, newPanel, [newPanel, fadeInEffect]() {
                if (newPanel->graphicsEffect() == fadeInEffect) {
                    newPanel->setGraphicsEffect(nullptr);
                }
            });

            connect(fadeIn, &QPropertyAnimation::finished, this, [this]() {
                m_panelSwitching = false;
            });

            fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
        });

        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        // 无旧面板（首次切换或旧面板已隐藏），直接淡入新面板
        newPanel->setVisible(true);

        QGraphicsOpacityEffect* fadeInEffect = new QGraphicsOpacityEffect(newPanel);
        fadeInEffect->setOpacity(0.0);
        newPanel->setGraphicsEffect(fadeInEffect);

        QPropertyAnimation* fadeIn = new QPropertyAnimation(fadeInEffect, "opacity");
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setDuration(250);
        fadeIn->setEasingCurve(QEasingCurve::OutCubic);

        connect(fadeIn, &QPropertyAnimation::finished, newPanel, [newPanel, fadeInEffect]() {
            if (newPanel->graphicsEffect() == fadeInEffect) {
                newPanel->setGraphicsEffect(nullptr);
            }
        });

        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    }
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

    // 定时发送器
    connect(m_timedSender, &TimedSender::sendData, this, [this](const QByteArray& data) {
        sendAndRecord(data);
    });

    // 发送历史变化时更新自动补全（复用模型，不泄漏QStringListModel）
    connect(m_sendHistory, &SendHistory::historyChanged, this, [this]() {
        m_sendCompleterModel->setStringList(m_sendHistory->recentTexts());
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

    // 导航树点击切换面板 — 数据驱动映射表查找，无 if-else 链
    connect(m_navTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        QString text = index.data().toString();

        // 功能性节点（不走面板切换，直接触发动作）
        if (text == tr("数据导出")) { onExportData(); return; }
        if (text == tr("TCP客户端")) { onConnectNetwork(ConnectionType::TcpClient); return; }
        if (text == tr("TCP服务端")) { onConnectNetwork(ConnectionType::TcpServer); return; }
        if (text == tr("UDP")) { onConnectNetwork(ConnectionType::Udp); return; }

        // 通过映射表查找目标面板widget
        QWidget* target = nullptr;
        for (const auto& mapping : m_navPanelMappings) {
            if (text == tr(mapping.name)) {
                target = mapping.widget;
                break;
            }
        }

        // 未匹配的面板节点（如分组节点"串口""网络""工具"），忽略
        if (!target) return;

        // 调用统一的带动画面板切换方法
        switchToPanel(target);
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

bool MainWindow::sendAndRecord(const QByteArray& data, bool isHex)
{
    Q_UNUSED(isHex);
    if (!m_currentConn || m_currentConn->state() != ConnectionState::Connected) {
        return false;
    }
    qint64 written = m_currentConn->write(data);
    if (written > 0) {
        m_terminalModel->appendSent(data);
        m_dataLogger->logData(data, DataLogger::Direction::Sent);
        updateStatusBar();
        return true;
    }
    return false;
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

    if (sendAndRecord(data)) {
        m_sendHistory->addEntry(text, isHex);
        m_sendInput->clear();
        m_sendInput->setProperty("hasError", false);
        m_sendInput->style()->unpolish(m_sendInput);
        m_sendInput->style()->polish(m_sendInput);
    }
}

void MainWindow::onQuickCommand(const QByteArray& data)
{
    sendAndRecord(data);
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
        // 连接成功后自动切换到终端面板（带动画）
        switchToPanel(m_terminal);
        stopBreathingAnimation();
        break;
    case ConnectionState::Disconnected:
        m_connStatusLbl->setText(tr("未连接"));
        stateStr = "disconnected";
        m_serialConfig->setConnected(false);
        stopBreathingAnimation();
        break;
    case ConnectionState::Connecting:
        m_connStatusLbl->setText(tr("连接中..."));
        stateStr = "connecting";
        startBreathingAnimation();
        break;
    case ConnectionState::Error:
        m_connStatusLbl->setText(tr("连接错误"));
        stateStr = "error";
        m_serialConfig->setConnected(false);
        stopBreathingAnimation();
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

void MainWindow::startBreathingAnimation()
{
    // 如果已有呼吸动画在运行，不重复创建
    if (m_breathingAnim && m_breathingAnim->state() == QAbstractAnimation::Running) {
        return;
    }

    // 为状态标签创建透明度效果
    if (!m_connStatusEffect) {
        m_connStatusEffect = new QGraphicsOpacityEffect(m_connStatusLbl);
        m_connStatusLbl->setGraphicsEffect(m_connStatusEffect);
    }
    m_connStatusEffect->setOpacity(1.0);

    // 创建呼吸脉冲动画: 1500ms循环, InOutSine, opacity 0.3 ↔ 1.0
    if (m_breathingAnim) {
        m_breathingAnim->stop();
        delete m_breathingAnim;
    }
    m_breathingAnim = new QPropertyAnimation(m_connStatusEffect, "opacity");
    m_breathingAnim->setStartValue(0.3);
    m_breathingAnim->setEndValue(1.0);
    m_breathingAnim->setDuration(1500);
    m_breathingAnim->setEasingCurve(QEasingCurve::InOutSine);
    m_breathingAnim->setLoopCount(-1);  // 无限循环
    // 注意: loopCount=-1 时动画不会自行停止，所以不能用 DeleteWhenStopped。
    // 生命周期由 startBreathingAnimation/stopBreathingAnimation 手动管理。
    m_breathingAnim->start();
}

void MainWindow::stopBreathingAnimation()
{
    if (m_breathingAnim) {
        m_breathingAnim->stop();
        delete m_breathingAnim;
        m_breathingAnim = nullptr;
    }
    // 恢复状态标签完全不透明
    if (m_connStatusEffect) {
        m_connStatusEffect->setOpacity(1.0);
        m_connStatusLbl->setGraphicsEffect(nullptr);
        delete m_connStatusEffect;
        m_connStatusEffect = nullptr;
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // 停止呼吸动画
    stopBreathingAnimation();

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
