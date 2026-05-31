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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_connManager(new ConnectionManager(this))
    , m_terminalModel(new TerminalModel(this))
    , m_timedSender(new TimedSender(this))
{
    setupUI();
    setupToolbar();
    setupStatusBar();
    connectSignals();

    // 加载主题
    ThemeManager::instance().loadTheme("dark_terminal");

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
    // 主分割器：左侧导航 + 右侧内容
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(m_mainSplitter);

    // ---- 左侧导航树 ----
    m_navTree = new QTreeView;
    m_navTree->setHeaderHidden(true);
    m_navTree->setMinimumWidth(180);
    m_navTree->setMaximumWidth(280);
    m_navTree->setIndentation(16);

    // 创建一个简单的树模型
    auto* treeModel = new QStandardItemModel(this);
    auto* rootItem = treeModel->invisibleRootItem();

    auto* serialItem = new QStandardItem(tr("Serial Port"));
    serialItem->setEditable(false);
    auto* configItem = new QStandardItem(tr("Config"));
    configItem->setEditable(false);
    auto* terminalItem = new QStandardItem(tr("Terminal"));
    terminalItem->setEditable(false);
    serialItem->appendRow(configItem);
    serialItem->appendRow(terminalItem);

    rootItem->appendRow(serialItem);

    m_navTree->setModel(treeModel);
    m_navTree->expandAll();

    m_mainSplitter->addWidget(m_navTree);

    // ---- 右侧内容面板 ----
    auto* rightWidget = new QWidget;
    auto* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // 右侧使用 QStackedWidget 切换不同面板
    m_rightPanel = new QStackedWidget;
    rightLayout->addWidget(m_rightPanel, 1);

    // ---- 面板0: 串口配置 + 终端 ----
    auto* serialPanel = new QWidget;
    auto* serialLayout = new QVBoxLayout(serialPanel);
    serialLayout->setContentsMargins(0, 0, 0, 0);
    serialLayout->setSpacing(0);

    // 串口配置面板(默认隐藏，点击"Config"时显示)
    m_serialConfig = new SerialConfigPanel;
    m_serialConfig->setVisible(false);
    serialLayout->addWidget(m_serialConfig);

    // 终端显示区
    m_terminal = new TerminalWidget;
    m_terminal->setModel(m_terminalModel);
    serialLayout->addWidget(m_terminal, 1);

    // 快捷指令栏
    m_quickCmdBar = new QuickCommandBar;
    // 添加一些默认快捷指令
    m_quickCmdBar->setCommands({
        {"AT", "AT\r\n", false},
        {"Reset", "AA 55 01 00 FE", true},
        {"Status", "AT+STATUS?\r\n", false}
    });
    serialLayout->addWidget(m_quickCmdBar);

    // 发送区域
    auto* sendFrame = new QFrame;
    sendFrame->setFrameShape(QFrame::StyledPanel);
    auto* sendLayout = new QHBoxLayout(sendFrame);
    sendLayout->setContentsMargins(8, 4, 8, 4);

    m_sendModeCombo = new QComboBox;
    m_sendModeCombo->addItems({tr("Text"), tr("HEX")});
    m_sendModeCombo->setFixedWidth(60);

    m_sendInput = new QLineEdit;
    m_sendInput->setPlaceholderText(tr("Enter data to send..."));

    m_sendBtn = new QPushButton(tr("Send"));
    m_sendBtn->setFixedWidth(70);

    sendLayout->addWidget(m_sendModeCombo);
    sendLayout->addWidget(m_sendInput, 1);
    sendLayout->addWidget(m_sendBtn);

    serialLayout->addWidget(sendFrame);

    m_rightPanel->addWidget(serialPanel);

    // 默认显示串口面板
    m_rightPanel->setCurrentIndex(0);

    m_mainSplitter->addWidget(rightWidget);

    // 设置分割比例
    m_mainSplitter->setSizes({200, 1000});
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);
}

void MainWindow::setupToolbar()
{
    m_toolbar = addToolBar(tr("Main Toolbar"));
    m_toolbar->setMovable(false);
    m_toolbar->setFloatable(false);

    // 显示模式切换
    m_displayModeCombo = new QComboBox;
    m_displayModeCombo->addItems({tr("Text"), tr("HEX"), tr("Mixed")});
    m_displayModeCombo->setFixedWidth(80);
    m_toolbar->addWidget(m_displayModeCombo);

    // 时间戳开关
    m_timestampAction = m_toolbar->addAction(tr("Timestamp"));
    m_timestampAction->setCheckable(true);
    m_timestampAction->setChecked(false);

    // 清空按钮
    m_clearAction = m_toolbar->addAction(tr("Clear"));

    m_toolbar->addSeparator();
}

void MainWindow::setupStatusBar()
{
    m_connStatusLbl = new QLabel(tr("Disconnected"));
    m_rxBytesLbl = new QLabel("RX: 0 B");
    m_txBytesLbl = new QLabel("TX: 0 B");

    statusBar()->addWidget(m_connStatusLbl, 1);
    statusBar()->addPermanentWidget(m_rxBytesLbl);
    statusBar()->addPermanentWidget(m_txBytesLbl);
}

void MainWindow::connectSignals()
{
    // 串口配置面板的连接/断开信号
    connect(m_serialConfig, &SerialConfigPanel::connectRequested,
            this, &MainWindow::onConnectSerial);
    connect(m_serialConfig, &SerialConfigPanel::disconnectRequested,
            this, &MainWindow::onDisconnectSerial);

    // 发送按钮
    connect(m_sendBtn, &QPushButton::clicked,
            this, &MainWindow::onSendData);
    connect(m_sendInput, &QLineEdit::returnPressed,
            this, &MainWindow::onSendData);

    // 快捷指令
    connect(m_quickCmdBar, &QuickCommandBar::commandTriggered,
            this, &MainWindow::onQuickCommand);

    // 工具栏
    connect(m_displayModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDisplayModeChanged);
    connect(m_timestampAction, &QAction::toggled,
            this, &MainWindow::onTimestampToggled);
    connect(m_clearAction, &QAction::triggered,
            this, &MainWindow::onClearTerminal);

    // 定时发送器
    connect(m_timedSender, &TimedSender::sendData,
            this, [this](const QByteArray& data) {
        if (m_currentConn && m_currentConn->state() == ConnectionState::Connected) {
            m_currentConn->write(data);
            m_terminalModel->appendSent(data);
            updateStatusBar();
        }
    });

    // 导航树点击切换面板
    connect(m_navTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        QString text = index.data().toString();
        if (text == tr("Config")) {
            m_serialConfig->setVisible(true);
            m_terminal->setVisible(false);
        } else if (text == tr("Terminal")) {
            m_serialConfig->setVisible(false);
            m_terminal->setVisible(true);
        }
    });
}

void MainWindow::onConnectSerial()
{
    // 创建新的串口连接
    m_currentConn = m_connManager->createSerialConnection();

    // 应用配置
    auto* serialConn = qobject_cast<SerialConnection*>(m_currentConn);
    m_serialConfig->applyConfigToConnection(serialConn);

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
                             tr("Cannot open serial port: ") + m_currentConn->name());
        m_connManager->removeConnection(m_currentConn);
        m_currentConn = nullptr;
        return;
    }

    // 切换到终端视图
    m_serialConfig->setVisible(false);
    m_terminal->setVisible(true);
}

void MainWindow::onDisconnectSerial()
{
    if (m_currentConn) {
        m_currentConn->close();    // 触发 stateChanged → 更新UI
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

    QByteArray data;
    if (m_sendModeCombo->currentIndex() == 1) {
        // HEX模式
        data = HexConverter::fromHexString(text);
        if (data.isEmpty()) {
            m_sendInput->setStyleSheet("QLineEdit { border: 1px solid red; }");
            return;
        }
    } else {
        // 文本模式
        data = text.toUtf8();
    }

    qint64 written = m_currentConn->write(data);
    if (written > 0) {
        m_terminalModel->appendSent(data);
        m_sendInput->clear();
        m_sendInput->setStyleSheet("");
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
    updateStatusBar();
}

void MainWindow::onConnectionStateChanged(ConnectionState state)
{
    switch (state) {
    case ConnectionState::Connected:
        m_connStatusLbl->setText(tr("Connected: %1").arg(
            m_currentConn ? m_currentConn->name() : ""));
        m_connStatusLbl->setStyleSheet("color: #a6e3a1;");
        m_serialConfig->setConnected(true);
        // 切换到终端视图
        m_serialConfig->setVisible(false);
        m_terminal->setVisible(true);
        break;
    case ConnectionState::Disconnected:
        m_connStatusLbl->setText(tr("Disconnected"));
        m_connStatusLbl->setStyleSheet("color: #f38ba8;");
        m_serialConfig->setConnected(false);
        break;
    case ConnectionState::Connecting:
        m_connStatusLbl->setText(tr("Connecting..."));
        m_connStatusLbl->setStyleSheet("color: #f9e2af;");
        break;
    case ConnectionState::Error:
        m_connStatusLbl->setText(tr("Error"));
        m_connStatusLbl->setStyleSheet("color: #f38ba8;");
        m_serialConfig->setConnected(false);
        break;
    }
}

void MainWindow::onDataReceived(const QByteArray& data)
{
    m_terminalModel->appendReceived(data);
    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    if (m_terminalModel) {
        auto rx = m_terminalModel->rxBytes();
        auto tx = m_terminalModel->txBytes();

        // 格式化字节数为人类可读格式
        auto formatBytes = [](quint64 bytes) -> QString {
            if (bytes < 1024) return QString("%1 B").arg(bytes);
            if (bytes < 1024 * 1024) return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
            return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
        };

        m_rxBytesLbl->setText("RX: " + formatBytes(rx));
        m_txBytesLbl->setText("TX: " + formatBytes(tx));
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // 关闭所有连接
    auto connections = m_connManager->connections();
    for (auto* conn : connections) {
        conn->close();
    }
    event->accept();
}
