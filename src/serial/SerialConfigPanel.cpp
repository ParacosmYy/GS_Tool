/**
 * @file SerialConfigPanel.cpp
 * @brief 串口配置面板实现 - 串口参数配置、连接控制和状态指示
 *
 * 面板布局: 端口选择 → 参数配置 → 控制信号 → 驱动检测 → 连接按钮+状态指示器
 * 状态指示器通过QSS的statusIndicator[state="xxx"]控制圆点颜色。
 */

#include "SerialConfigPanel.h"
#include "SerialDriverDetector.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QFormLayout>
#include <QGroupBox>
#include <QSerialPortInfo>
#include <QIntValidator>
#include <QGraphicsOpacityEffect>

// ---- 构造 ----

SerialConfigPanel::SerialConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_breathTimer(new QTimer(this))
{
    setupUI();
    refreshPorts();
    updateDriverInfo();

    // 呼吸动画定时器: 连接中状态时周期性更新透明度
    m_breathTimer->setInterval(50);  // 50ms一帧，约20fps
    connect(m_breathTimer, &QTimer::timeout, this, [this]() {
        if (m_breathIncreasing) {
            m_breathOpacity += 0.05;
            if (m_breathOpacity >= 1.0) { m_breathOpacity = 1.0; m_breathIncreasing = false; }
        } else {
            m_breathOpacity -= 0.05;
            if (m_breathOpacity <= 0.3) { m_breathOpacity = 0.3; m_breathIncreasing = true; }
        }
        if (auto* effect = qobject_cast<QGraphicsOpacityEffect*>(m_statusIndicator->graphicsEffect()))
            effect->setOpacity(m_breathOpacity);
    });
}

// ---- UI布局 ----

void SerialConfigPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    // ---- 端口选择区域 ----
    auto* portGroup = new QGroupBox(tr("端口"));
    portGroup->setObjectName("portGroup");
    auto* portLayout = new QHBoxLayout(portGroup);

    m_portCombo = new QComboBox;
    m_portCombo->setObjectName("portCombo");
    m_portCombo->setMinimumWidth(150);
    connect(m_portCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SerialConfigPanel::onPortComboChanged);

    m_refreshBtn = new QPushButton(tr("刷新"));
    m_refreshBtn->setObjectName("refreshBtn");
    m_refreshBtn->setToolTip(tr("重新扫描系统中的串口设备"));
    connect(m_refreshBtn, &QPushButton::clicked, this, &SerialConfigPanel::refreshPorts);

    portLayout->addWidget(m_portCombo, 1);
    portLayout->addWidget(m_refreshBtn);
    mainLayout->addWidget(portGroup);

    // ---- 串口参数区域 ----
    auto* paramGroup = new QGroupBox(tr("参数"));
    paramGroup->setObjectName("paramGroup");
    auto* formLayout = new QFormLayout(paramGroup);

    m_baudCombo = new QComboBox;
    m_baudCombo->setObjectName("baudCombo");
    m_baudCombo->setEditable(true);
    m_baudCombo->setToolTip(tr("通信速率(比特/秒)，常用值: 9600, 115200\n可直接输入自定义波特率"));
    m_baudCombo->addItems({"1200", "2400", "4800", "9600", "19200",
                           "38400", "57600", "115200", "230400",
                           "460800", "921600", "1000000"});
    m_baudCombo->setCurrentText("115200");
    m_baudCombo->lineEdit()->setValidator(new QIntValidator(300, 10000000, this));
    formLayout->addRow(tr("波特率:"), m_baudCombo);

    m_dataBitsCombo = new QComboBox;
    m_dataBitsCombo->setObjectName("dataBitsCombo");
    m_dataBitsCombo->setToolTip(tr("每个数据帧的数据位数，绝大多数设备使用 8 位"));
    m_dataBitsCombo->addItems({"5", "6", "7", "8"});
    m_dataBitsCombo->setCurrentIndex(3);
    formLayout->addRow(tr("数据位:"), m_dataBitsCombo);

    m_parityCombo = new QComboBox;
    m_parityCombo->setObjectName("parityCombo");
    m_parityCombo->setToolTip(tr("校验方式:\n无 - 不校验(最常用)\n偶校验/奇校验 - 简单错误检测"));
    m_parityCombo->addItems({tr("无"), tr("偶校验"), tr("奇校验"), tr("Mark"), tr("Space")});
    formLayout->addRow(tr("校验位:"), m_parityCombo);

    m_stopBitsCombo = new QComboBox;
    m_stopBitsCombo->setObjectName("stopBitsCombo");
    m_stopBitsCombo->setToolTip(tr("帧结束的停止位数，绝大多数设备使用 1 位"));
    m_stopBitsCombo->addItems({"1", "1.5", "2"});
    formLayout->addRow(tr("停止位:"), m_stopBitsCombo);

    m_flowControlCombo = new QComboBox;
    m_flowControlCombo->setObjectName("flowControlCombo");
    m_flowControlCombo->setToolTip(tr("流量控制:\n无 - 不使用流控(最常用)\nRTS/CTS - 硬件流控\nXON/XOFF - 软件流控"));
    m_flowControlCombo->addItems({tr("无"), tr("RTS/CTS"), tr("XON/XOFF")});
    formLayout->addRow(tr("流控:"), m_flowControlCombo);

    mainLayout->addWidget(paramGroup);

    // ---- 控制信号 ----
    auto* signalGroup = new QGroupBox(tr("控制信号"));
    signalGroup->setObjectName("signalGroup");
    auto* signalLayout = new QHBoxLayout(signalGroup);

    m_dtrCheck = new QCheckBox(tr("DTR"));
    m_dtrCheck->setChecked(true);
    m_dtrCheck->setObjectName("dtrCheck");
    m_dtrCheck->setToolTip(tr("数据终端就绪信号，部分设备需要 DTR 拉低才能复位"));
    m_rtsCheck = new QCheckBox(tr("RTS"));
    m_rtsCheck->setChecked(true);
    m_rtsCheck->setObjectName("rtsCheck");
    m_rtsCheck->setToolTip(tr("请求发送信号，部分设备需要 RTS 拉低进入 bootloader"));

    signalLayout->addWidget(m_dtrCheck);
    signalLayout->addWidget(m_rtsCheck);
    signalLayout->addStretch();

    connect(m_dtrCheck, &QCheckBox::toggled, this, &SerialConfigPanel::dtrChanged);
    connect(m_rtsCheck, &QCheckBox::toggled, this, &SerialConfigPanel::rtsChanged);
    mainLayout->addWidget(signalGroup);

    // ---- 驱动检测信息 ----
    m_driverInfoLbl = new QLabel;
    m_driverInfoLbl->setObjectName("driverInfoLbl");
    m_driverInfoLbl->setWordWrap(true);
    mainLayout->addWidget(m_driverInfoLbl);

    // ---- 连接按钮 + 状态指示器 ----
    auto* connectLayout = new QHBoxLayout;

    m_statusIndicator = new QLabel;
    m_statusIndicator->setObjectName("statusIndicator");
    m_statusIndicator->setFixedSize(8, 8);
    m_statusIndicator->setToolTip(tr("未连接"));
    // 初始状态为断开(灰色圆点)
    m_statusIndicator->setProperty("state", "disconnected");
    m_statusIndicator->style()->unpolish(m_statusIndicator);
    m_statusIndicator->style()->polish(m_statusIndicator);

    m_connectBtn = new QPushButton(tr("连接"));
    m_connectBtn->setObjectName("connectBtn");
    m_connectBtn->setMinimumHeight(36);
    m_connectBtn->setToolTip(tr("建立串口连接，快捷键: 无"));
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        if (m_connecting) return;
        if (m_connected) {
            emit disconnectRequested();
        } else {
            m_connecting = true;
            m_connectBtn->setEnabled(false);
            m_connectBtn->setText(tr("连接中..."));
            m_connectBtn->setProperty("state", "connecting");
            m_connectBtn->style()->unpolish(m_connectBtn);
            m_connectBtn->style()->polish(m_connectBtn);
            emit connectRequested();
        }
    });

    connectLayout->addWidget(m_statusIndicator);
    connectLayout->addWidget(m_connectBtn, 1);
    mainLayout->addLayout(connectLayout);

    mainLayout->addStretch();
}

// ---- 状态管理 ----

void SerialConfigPanel::setConnected(bool connected)
{
    m_connected = connected;
    m_connecting = false;

    m_connectBtn->setEnabled(true);
    m_connectBtn->setText(connected ? tr("断开") : tr("连接"));
    m_connectBtn->setProperty("state", connected ? "connected" : "");
    m_connectBtn->style()->unpolish(m_connectBtn);
    m_connectBtn->style()->polish(m_connectBtn);

    // 锁定/解锁配置控件
    m_portCombo->setEnabled(!connected);
    m_baudCombo->setEnabled(!connected);
    m_dataBitsCombo->setEnabled(!connected);
    m_parityCombo->setEnabled(!connected);
    m_stopBitsCombo->setEnabled(!connected);
    m_flowControlCombo->setEnabled(!connected);
    m_refreshBtn->setEnabled(!connected);
    m_dtrCheck->setEnabled(true);
    m_rtsCheck->setEnabled(true);

    // 更新状态指示器
    if (connected) {
        m_breathTimer->stop();
        updateStatusIndicator("connected");
        m_statusIndicator->setToolTip(tr("已连接"));
    } else {
        m_breathTimer->stop();
        updateStatusIndicator("disconnected");
        m_statusIndicator->setToolTip(tr("未连接"));
        updateConnectButtonState();
    }
}

void SerialConfigPanel::setError(const QString& errorMsg)
{
    m_connected = false;
    m_connecting = false;
    m_breathTimer->stop();

    m_connectBtn->setEnabled(true);
    m_connectBtn->setText(tr("连接失败"));
    m_connectBtn->setProperty("state", "error");
    m_connectBtn->style()->unpolish(m_connectBtn);
    m_connectBtn->style()->polish(m_connectBtn);

    updateStatusIndicator("error");
    m_statusIndicator->setToolTip(tr("连接错误: ") + errorMsg);

    // 3秒后自动恢复为正常断开状态
    QTimer::singleShot(3000, this, [this]() {
        if (!m_connected && !m_connecting) {
            m_connectBtn->setText(tr("连接"));
            m_connectBtn->setProperty("state", "");
            m_connectBtn->style()->unpolish(m_connectBtn);
            m_connectBtn->style()->polish(m_connectBtn);
            updateStatusIndicator("disconnected");
            m_statusIndicator->setToolTip(tr("未连接"));
        }
    });
}

void SerialConfigPanel::setConnecting()
{
    m_connecting = true;
    updateStatusIndicator("connecting");
    m_statusIndicator->setToolTip(tr("正在连接..."));

    // 启动呼吸动画
    m_breathOpacity = 1.0;
    m_breathIncreasing = false;
    auto* effect = new QGraphicsOpacityEffect(m_statusIndicator);
    m_statusIndicator->setGraphicsEffect(effect);
    m_breathTimer->start();
}

void SerialConfigPanel::updateStatusIndicator(const QString& state)
{
    m_statusIndicator->setProperty("state", state);
    m_statusIndicator->style()->unpolish(m_statusIndicator);
    m_statusIndicator->style()->polish(m_statusIndicator);
}

// ---- 端口管理 ----

void SerialConfigPanel::refreshPorts()
{
    QString currentPort = m_portCombo->currentData().toString();
    m_portCombo->clear();

    auto ports = QSerialPortInfo::availablePorts();
    for (const auto& port : ports) {
        QString name = port.portName();
        QString desc = port.description();
        if (!desc.isEmpty()) m_portCombo->addItem(name + " - " + desc, name);
        else m_portCombo->addItem(name, name);
    }

    if (!currentPort.isEmpty()) {
        int idx = m_portCombo->findData(currentPort);
        if (idx >= 0) m_portCombo->setCurrentIndex(idx);
    }
    updateDriverInfo();
    updateConnectButtonState();
}

// ---- 配置读取 ----

bool SerialConfigPanel::isConnected() const { return m_connected; }
QString SerialConfigPanel::currentPortData() const { return m_portCombo->currentData().toString(); }
int SerialConfigPanel::currentBaudRate() const { return m_baudCombo->currentText().toInt(); }
int SerialConfigPanel::currentDataBitsIndex() const { return m_dataBitsCombo->currentIndex(); }
int SerialConfigPanel::currentParityIndex() const { return m_parityCombo->currentIndex(); }
int SerialConfigPanel::currentStopBitsIndex() const { return m_stopBitsCombo->currentIndex(); }
int SerialConfigPanel::currentFlowControlIndex() const { return m_flowControlCombo->currentIndex(); }
bool SerialConfigPanel::dtrEnabled() const { return m_dtrCheck->isChecked(); }
bool SerialConfigPanel::rtsEnabled() const { return m_rtsCheck->isChecked(); }

void SerialConfigPanel::restoreConfig(const QVariantMap& config)
{
    if (config.contains("portName")) {
        int idx = m_portCombo->findData(config["portName"].toString());
        if (idx >= 0) m_portCombo->setCurrentIndex(idx);
    }
    if (config.contains("baudRate"))
        m_baudCombo->setCurrentText(QString::number(config["baudRate"].toInt()));
    if (config.contains("dataBits") && config["dataBits"].toInt() >= 0
        && config["dataBits"].toInt() < m_dataBitsCombo->count())
        m_dataBitsCombo->setCurrentIndex(config["dataBits"].toInt());
    if (config.contains("parity") && config["parity"].toInt() >= 0
        && config["parity"].toInt() < m_parityCombo->count())
        m_parityCombo->setCurrentIndex(config["parity"].toInt());
    if (config.contains("stopBits") && config["stopBits"].toInt() >= 0
        && config["stopBits"].toInt() < m_stopBitsCombo->count())
        m_stopBitsCombo->setCurrentIndex(config["stopBits"].toInt());
    if (config.contains("flowControl") && config["flowControl"].toInt() >= 0
        && config["flowControl"].toInt() < m_flowControlCombo->count())
        m_flowControlCombo->setCurrentIndex(config["flowControl"].toInt());
    if (config.contains("dtr")) {
        m_dtrCheck->blockSignals(true);
        m_dtrCheck->setChecked(config["dtr"].toBool());
        m_dtrCheck->blockSignals(false);
    }
    if (config.contains("rts")) {
        m_rtsCheck->blockSignals(true);
        m_rtsCheck->setChecked(config["rts"].toBool());
        m_rtsCheck->blockSignals(false);
    }
}

// ---- 内部方法 ----

void SerialConfigPanel::updateDriverInfo()
{
    m_driverInfoLbl->setText(SerialDriverDetector::driverStatusSummary());
}

void SerialConfigPanel::onPortComboChanged() { updateConnectButtonState(); }

void SerialConfigPanel::updateConnectButtonState()
{
    if (!m_connected && !m_connecting) {
        bool hasPorts = m_portCombo->count() > 0;
        m_connectBtn->setEnabled(hasPorts);
        m_connectBtn->setText(hasPorts ? tr("连接") : tr("无端口"));
    }
}
