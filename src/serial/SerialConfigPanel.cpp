#include "SerialConfigPanel.h"
#include "SerialDriverDetector.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSerialPortInfo>

SerialConfigPanel::SerialConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    refreshPorts();
    updateDriverInfo();
}

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

    m_refreshBtn = new QPushButton(tr("刷新"));
    m_refreshBtn->setObjectName("refreshBtn");
    connect(m_refreshBtn, &QPushButton::clicked, this, &SerialConfigPanel::refreshPorts);

    portLayout->addWidget(m_portCombo, 1);
    portLayout->addWidget(m_refreshBtn);
    mainLayout->addWidget(portGroup);

    // ---- 串口参数区域 ----
    auto* paramGroup = new QGroupBox(tr("参数"));
    paramGroup->setObjectName("paramGroup");
    auto* formLayout = new QFormLayout(paramGroup);

    // 波特率
    m_baudCombo = new QComboBox;
    m_baudCombo->setObjectName("baudCombo");
    m_baudCombo->setEditable(true);  // 允许自定义波特率
    QStringList baudRates = {"1200", "2400", "4800", "9600", "19200",
                             "38400", "57600", "115200", "230400",
                             "460800", "921600", "1000000"};
    m_baudCombo->addItems(baudRates);
    m_baudCombo->setCurrentText("115200");
    formLayout->addRow(tr("波特率:"), m_baudCombo);

    // 数据位
    m_dataBitsCombo = new QComboBox;
    m_dataBitsCombo->setObjectName("dataBitsCombo");
    m_dataBitsCombo->addItems({"5", "6", "7", "8"});
    m_dataBitsCombo->setCurrentIndex(3);  // 默认8
    formLayout->addRow(tr("数据位:"), m_dataBitsCombo);

    // 校验
    m_parityCombo = new QComboBox;
    m_parityCombo->setObjectName("parityCombo");
    m_parityCombo->addItems({tr("无"), tr("偶校验"), tr("奇校验"),
                              tr("Mark"), tr("Space")});
    formLayout->addRow(tr("校验位:"), m_parityCombo);

    // 停止位
    m_stopBitsCombo = new QComboBox;
    m_stopBitsCombo->setObjectName("stopBitsCombo");
    m_stopBitsCombo->addItems({"1", "1.5", "2"});
    formLayout->addRow(tr("停止位:"), m_stopBitsCombo);

    // 流控
    m_flowControlCombo = new QComboBox;
    m_flowControlCombo->setObjectName("flowControlCombo");
    m_flowControlCombo->addItems({tr("无"), tr("RTS/CTS"), tr("XON/XOFF")});
    formLayout->addRow(tr("流控:"), m_flowControlCombo);

    mainLayout->addWidget(paramGroup);

    // ---- 控制信号 ----
    auto* signalGroup = new QGroupBox(tr("控制信号"));
    auto* signalLayout = new QHBoxLayout(signalGroup);

    m_dtrCheck = new QCheckBox("DTR");
    m_dtrCheck->setChecked(true);
    m_dtrCheck->setObjectName("dtrCheck");
    m_rtsCheck = new QCheckBox("RTS");
    m_rtsCheck->setChecked(true);
    m_rtsCheck->setObjectName("rtsCheck");

    signalLayout->addWidget(m_dtrCheck);
    signalLayout->addWidget(m_rtsCheck);
    signalLayout->addStretch();

    // DTR/RTS运行时控制信号
    connect(m_dtrCheck, &QCheckBox::toggled, this, &SerialConfigPanel::dtrChanged);
    connect(m_rtsCheck, &QCheckBox::toggled, this, &SerialConfigPanel::rtsChanged);
    mainLayout->addWidget(signalGroup);

    // ---- 驱动检测信息 ----
    m_driverInfoLbl = new QLabel;
    m_driverInfoLbl->setObjectName("driverInfoLbl");
    m_driverInfoLbl->setWordWrap(true);
    m_driverInfoLbl->setStyleSheet("font-size: 11px; padding: 4px;");
    mainLayout->addWidget(m_driverInfoLbl);

    // ---- 连接按钮 ----
    m_connectBtn = new QPushButton(tr("连接"));
    m_connectBtn->setObjectName("connectBtn");
    m_connectBtn->setMinimumHeight(36);
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        if (m_connected) {
            emit disconnectRequested();
        } else {
            emit connectRequested();
        }
    });
    mainLayout->addWidget(m_connectBtn);

    mainLayout->addStretch();
}

void SerialConfigPanel::refreshPorts()
{
    QString currentPort = m_portCombo->currentText();
    m_portCombo->clear();

    auto ports = QSerialPortInfo::availablePorts();
    for (const auto& port : ports) {
        QString name = port.portName();
        QString desc = port.description();
        if (!desc.isEmpty()) {
            m_portCombo->addItem(name + " - " + desc, name);
        } else {
            m_portCombo->addItem(name, name);
        }
    }

    if (!currentPort.isEmpty()) {
        int idx = m_portCombo->findData(currentPort);
        if (idx >= 0) {
            m_portCombo->setCurrentIndex(idx);
        }
    }

    updateDriverInfo();
}

void SerialConfigPanel::setConnected(bool connected)
{
    m_connected = connected;
    m_connectBtn->setText(connected ? tr("断开") : tr("连接"));
    m_connectBtn->setProperty("state", connected ? "connected" : "");
    m_connectBtn->style()->unpolish(m_connectBtn);
    m_connectBtn->style()->polish(m_connectBtn);

    m_portCombo->setEnabled(!connected);
    m_baudCombo->setEnabled(!connected);
    m_dataBitsCombo->setEnabled(!connected);
    m_parityCombo->setEnabled(!connected);
    m_stopBitsCombo->setEnabled(!connected);
    m_flowControlCombo->setEnabled(!connected);
    m_refreshBtn->setEnabled(!connected);
    // DTR/RTS 保持可用，允许连接后实时切换
    m_dtrCheck->setEnabled(true);
    m_rtsCheck->setEnabled(true);
}

bool SerialConfigPanel::isConnected() const
{
    return m_connected;
}

QString SerialConfigPanel::currentPortData() const
{
    return m_portCombo->currentData().toString();
}

int SerialConfigPanel::currentBaudRate() const
{
    return m_baudCombo->currentText().toInt();
}

int SerialConfigPanel::currentDataBitsIndex() const
{
    return m_dataBitsCombo->currentIndex();
}

int SerialConfigPanel::currentParityIndex() const
{
    return m_parityCombo->currentIndex();
}

int SerialConfigPanel::currentStopBitsIndex() const
{
    return m_stopBitsCombo->currentIndex();
}

int SerialConfigPanel::currentFlowControlIndex() const
{
    return m_flowControlCombo->currentIndex();
}

bool SerialConfigPanel::dtrEnabled() const
{
    return m_dtrCheck->isChecked();
}

bool SerialConfigPanel::rtsEnabled() const
{
    return m_rtsCheck->isChecked();
}

void SerialConfigPanel::restoreConfig(const QVariantMap& config)
{
    if (config.contains("portName")) {
        QString portName = config["portName"].toString();
        int idx = m_portCombo->findData(portName);
        if (idx >= 0) {
            m_portCombo->setCurrentIndex(idx);
        }
    }
    if (config.contains("baudRate")) {
        m_baudCombo->setCurrentText(QString::number(config["baudRate"].toInt()));
    }
    if (config.contains("dataBits")) {
        m_dataBitsCombo->setCurrentIndex(config["dataBits"].toInt());
    }
    if (config.contains("parity")) {
        int idx = config["parity"].toInt();
        if (idx >= 0 && idx < m_parityCombo->count()) {
            m_parityCombo->setCurrentIndex(idx);
        }
    }
    if (config.contains("stopBits")) {
        int idx = config["stopBits"].toInt();
        if (idx >= 0 && idx < m_stopBitsCombo->count()) {
            m_stopBitsCombo->setCurrentIndex(idx);
        }
    }
    if (config.contains("flowControl")) {
        int idx = config["flowControl"].toInt();
        if (idx >= 0 && idx < m_flowControlCombo->count()) {
            m_flowControlCombo->setCurrentIndex(idx);
        }
    }
}

void SerialConfigPanel::updateDriverInfo()
{
    QString summary = SerialDriverDetector::driverStatusSummary();
    m_driverInfoLbl->setText(summary);
}
