#include "SerialConfigPanel.h"
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
}

void SerialConfigPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // ---- 端口选择区域 ----
    auto* portGroup = new QGroupBox(tr("Port"));
    auto* portLayout = new QHBoxLayout(portGroup);

    m_portCombo = new QComboBox;
    m_portCombo->setMinimumWidth(150);

    m_refreshBtn = new QPushButton(tr("Refresh"));
    connect(m_refreshBtn, &QPushButton::clicked, this, &SerialConfigPanel::refreshPorts);

    portLayout->addWidget(m_portCombo, 1);
    portLayout->addWidget(m_refreshBtn);
    mainLayout->addWidget(portGroup);

    // ---- 串口参数区域 ----
    auto* paramGroup = new QGroupBox(tr("Parameters"));
    auto* formLayout = new QFormLayout(paramGroup);

    // 波特率
    m_baudCombo = new QComboBox;
    m_baudCombo->setEditable(true);  // 允许自定义波特率
    QStringList baudRates = {"1200", "2400", "4800", "9600", "19200",
                             "38400", "57600", "115200", "230400",
                             "460800", "921600", "1000000"};
    m_baudCombo->addItems(baudRates);
    m_baudCombo->setCurrentText("115200");
    formLayout->addRow(tr("Baud Rate:"), m_baudCombo);

    // 数据位
    m_dataBitsCombo = new QComboBox;
    m_dataBitsCombo->addItems({"5", "6", "7", "8"});
    m_dataBitsCombo->setCurrentIndex(3);  // 默认8
    formLayout->addRow(tr("Data Bits:"), m_dataBitsCombo);

    // 校验
    m_parityCombo = new QComboBox;
    m_parityCombo->addItems({tr("None"), tr("Even"), tr("Odd"),
                              tr("Mark"), tr("Space")});
    formLayout->addRow(tr("Parity:"), m_parityCombo);

    // 停止位
    m_stopBitsCombo = new QComboBox;
    m_stopBitsCombo->addItems({"1", "1.5", "2"});
    formLayout->addRow(tr("Stop Bits:"), m_stopBitsCombo);

    // 流控
    m_flowControlCombo = new QComboBox;
    m_flowControlCombo->addItems({tr("None"), tr("RTS/CTS"), tr("XON/XOFF")});
    formLayout->addRow(tr("Flow Control:"), m_flowControlCombo);

    mainLayout->addWidget(paramGroup);

    // ---- 控制信号 ----
    auto* signalGroup = new QGroupBox(tr("Control Signals"));
    auto* signalLayout = new QHBoxLayout(signalGroup);

    m_dtrCheck = new QCheckBox("DTR");
    m_dtrCheck->setChecked(true);
    m_rtsCheck = new QCheckBox("RTS");
    m_rtsCheck->setChecked(true);

    signalLayout->addWidget(m_dtrCheck);
    signalLayout->addWidget(m_rtsCheck);
    signalLayout->addStretch();
    mainLayout->addWidget(signalGroup);

    // ---- 连接按钮 ----
    m_connectBtn = new QPushButton(tr("Connect"));
    m_connectBtn->setMinimumHeight(36);
    m_connectBtn->setStyleSheet(
        "QPushButton { background-color: #a6e3a1; color: #1e1e2e; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { background-color: #94e2d5; }");
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

void SerialConfigPanel::applyConfigToConnection(SerialConnection* conn)
{
    // currentData() 返回的是实际端口名 "COM3"，而非显示文本 "COM3 - USB Serial"
    conn->setPortName(m_portCombo->currentData().toString());
    conn->setBaudRate(m_baudCombo->currentText().toInt());

    // 数据位
    int dataBitsIndex = m_dataBitsCombo->currentIndex();
    QSerialPort::DataBits dataBits[] = {
        QSerialPort::Data5, QSerialPort::Data6,
        QSerialPort::Data7, QSerialPort::Data8
    };
    conn->setDataBits(dataBits[dataBitsIndex]);

    // 校验
    QSerialPort::Parity parity[] = {
        QSerialPort::NoParity, QSerialPort::EvenParity,
        QSerialPort::OddParity, QSerialPort::MarkParity,
        QSerialPort::SpaceParity
    };
    conn->setParity(parity[m_parityCombo->currentIndex()]);

    // 停止位
    QSerialPort::StopBits stopBits[] = {
        QSerialPort::OneStop, QSerialPort::OneAndHalfStop,
        QSerialPort::TwoStop
    };
    conn->setStopBits(stopBits[m_stopBitsCombo->currentIndex()]);

    // 流控
    QSerialPort::FlowControl flow[] = {
        QSerialPort::NoFlowControl, QSerialPort::HardwareControl,
        QSerialPort::SoftwareControl
    };
    conn->setFlowControl(flow[m_flowControlCombo->currentIndex()]);

    // DTR/RTS
    conn->setDtr(m_dtrCheck->isChecked());
    conn->setRts(m_rtsCheck->isChecked());
}

void SerialConfigPanel::loadConfigFromConnection(SerialConnection* conn)
{
    m_portCombo->setCurrentText(conn->portName());
    m_baudCombo->setCurrentText(QString::number(conn->baudRate()));
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

    // 恢复之前的选择
    if (!currentPort.isEmpty()) {
        int idx = m_portCombo->findData(currentPort);
        if (idx >= 0) {
            m_portCombo->setCurrentIndex(idx);
        }
    }
}

void SerialConfigPanel::setConnected(bool connected)
{
    m_connected = connected;
    if (connected) {
        m_connectBtn->setText(tr("Disconnect"));
        m_connectBtn->setStyleSheet(
            "QPushButton { background-color: #f38ba8; color: #1e1e2e; font-weight: bold; font-size: 14px; }"
            "QPushButton:hover { background-color: #eba0ac; }");
        // 连接后禁用配置修改
        m_portCombo->setEnabled(false);
        m_baudCombo->setEnabled(false);
        m_dataBitsCombo->setEnabled(false);
        m_parityCombo->setEnabled(false);
        m_stopBitsCombo->setEnabled(false);
        m_flowControlCombo->setEnabled(false);
        m_refreshBtn->setEnabled(false);
    } else {
        m_connectBtn->setText(tr("Connect"));
        m_connectBtn->setStyleSheet(
            "QPushButton { background-color: #a6e3a1; color: #1e1e2e; font-weight: bold; font-size: 14px; }"
            "QPushButton:hover { background-color: #94e2d5; }");
        // 断开后恢复配置可编辑
        m_portCombo->setEnabled(true);
        m_baudCombo->setEnabled(true);
        m_dataBitsCombo->setEnabled(true);
        m_parityCombo->setEnabled(true);
        m_stopBitsCombo->setEnabled(true);
        m_flowControlCombo->setEnabled(true);
        m_refreshBtn->setEnabled(true);
    }
}

bool SerialConfigPanel::isConnected() const
{
    return m_connected;
}
