#include "apps/serial_station/ui/SerialPortPanel.h"

#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtSerialPort/QSerialPortInfo>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>

namespace serial_station {

namespace {

QString portDisplayText(const QSerialPortInfo& port)
{
    QStringList parts;
    parts << port.portName();

    if (!port.description().trimmed().isEmpty()) {
        parts << port.description().trimmed();
    }

    if (!port.manufacturer().trimmed().isEmpty()) {
        parts << port.manufacturer().trimmed();
    }

    if (port.hasVendorIdentifier() && port.hasProductIdentifier()) {
        parts << QStringLiteral("VID:%1 PID:%2")
                     .arg(port.vendorIdentifier(), 4, 16, QLatin1Char('0'))
                     .arg(port.productIdentifier(), 4, 16, QLatin1Char('0'))
                     .toUpper();
    }

    return parts.join(QStringLiteral(" - "));
}

template <typename EnumType>
void addEnumItem(QComboBox* combo, const QString& label, EnumType value)
{
    combo->addItem(label, QVariant::fromValue(static_cast<int>(value)));
}

template <typename EnumType>
EnumType currentEnumValue(const QComboBox* combo, EnumType fallback)
{
    if (!combo || combo->currentIndex() < 0) {
        return fallback;
    }
    return static_cast<EnumType>(combo->currentData().toInt());
}

} // namespace

SerialPortPanel::SerialPortPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("serialPortPanel"));
    setupUi();
    setupCombos();
    connectSignals();
    refreshPorts();
    setSessionState(SerialSessionState::Closed);
}

SerialPortConfig SerialPortPanel::currentConfig() const
{
    SerialPortConfig config;
    const int portIndex = m_portCombo->currentIndex();
    const QString storedPortName = m_portCombo->currentData().toString().trimmed();
    const bool usesListedPort =
        portIndex >= 0 && m_portCombo->currentText() == m_portCombo->itemText(portIndex);
    if (usesListedPort && !storedPortName.isEmpty()) {
        config.portName = storedPortName;
    } else {
        config.portName = m_portCombo->currentText().trimmed();
    }
    config.baudRate = m_baudCombo->currentText().toInt();
    config.dataBits = currentEnumValue(m_dataBitsCombo, QSerialPort::Data8);
    config.parity = currentEnumValue(m_parityCombo, QSerialPort::NoParity);
    config.stopBits = currentEnumValue(m_stopBitsCombo, QSerialPort::OneStop);
    config.flowControl = currentEnumValue(m_flowControlCombo, QSerialPort::NoFlowControl);
    config.dtrEnabled = m_dtrCheck->isChecked();
    config.rtsEnabled = m_rtsCheck->isChecked();
    return config;
}

void SerialPortPanel::applyConfig(const SerialPortConfig& config)
{
    const SerialPortConfig normalized = config.normalized();
    int portIndex = m_portCombo->findData(normalized.portName);
    if (portIndex < 0) {
        portIndex = m_portCombo->findText(normalized.portName);
    }
    if (portIndex >= 0) {
        m_portCombo->setCurrentIndex(portIndex);
    } else {
        m_portCombo->setEditText(normalized.portName);
    }

    if (m_baudCombo->findText(QString::number(normalized.baudRate)) < 0) {
        m_baudCombo->addItem(QString::number(normalized.baudRate));
    }
    m_baudCombo->setCurrentText(QString::number(normalized.baudRate));
    m_dataBitsCombo->setCurrentIndex(m_dataBitsCombo->findData(static_cast<int>(normalized.dataBits)));
    m_parityCombo->setCurrentIndex(m_parityCombo->findData(static_cast<int>(normalized.parity)));
    m_stopBitsCombo->setCurrentIndex(m_stopBitsCombo->findData(static_cast<int>(normalized.stopBits)));
    m_flowControlCombo->setCurrentIndex(
        m_flowControlCombo->findData(static_cast<int>(normalized.flowControl)));
    m_dtrCheck->setChecked(normalized.dtrEnabled);
    m_rtsCheck->setChecked(normalized.rtsEnabled);
    updateSummary();
}

void SerialPortPanel::setSessionState(SerialSessionState state)
{
    const bool isOpen = state == SerialSessionState::Open;
    const bool isOpening = state == SerialSessionState::Opening;

    m_connectButton->setEnabled(!isOpen && !isOpening);
    m_disconnectButton->setEnabled(isOpen || isOpening);
    m_portCombo->setEnabled(!isOpen && !isOpening);
    m_baudCombo->setEnabled(!isOpen && !isOpening);
    m_dataBitsCombo->setEnabled(!isOpen && !isOpening);
    m_parityCombo->setEnabled(!isOpen && !isOpening);
    m_stopBitsCombo->setEnabled(!isOpen && !isOpening);
    m_flowControlCombo->setEnabled(!isOpen && !isOpening);
    m_dtrCheck->setEnabled(!isOpen && !isOpening);
    m_rtsCheck->setEnabled(!isOpen && !isOpening);
    m_refreshButton->setEnabled(!isOpen && !isOpening);

    switch (state) {
    case SerialSessionState::Closed:
        setStatusText(tr("未连接"), QStringLiteral("closed"));
        break;
    case SerialSessionState::Opening:
        setStatusText(tr("连接中"), QStringLiteral("opening"));
        break;
    case SerialSessionState::Open:
        setStatusText(tr("已连接"), QStringLiteral("open"));
        break;
    case SerialSessionState::Error:
        setStatusText(tr("连接错误"), QStringLiteral("error"));
        break;
    }
}

void SerialPortPanel::setErrorMessage(const QString& message)
{
    if (message.trimmed().isEmpty()) {
        return;
    }
    setStatusText(message, QStringLiteral("error"));
}

void SerialPortPanel::refreshPorts()
{
    const QString previous = m_portCombo->currentText();
    m_portCombo->clear();

    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& port : ports) {
        m_portCombo->addItem(portDisplayText(port), port.portName());
    }

    if (!previous.isEmpty()) {
        int index = m_portCombo->findData(previous);
        if (index < 0) {
            index = m_portCombo->findText(previous);
        }
        if (index >= 0) {
            m_portCombo->setCurrentIndex(index);
        }
    }

    if (m_portCombo->count() == 0) {
        m_portCombo->setEditable(true);
        m_portCombo->setEditText(QString());
        setStatusText(tr("未发现串口，可手动输入端口名"), QStringLiteral("warning"));
    } else {
        m_portCombo->setEditable(true);
        setStatusText(tr("已刷新串口列表"), QStringLiteral("closed"));
    }

    updateSummary();
    emit refreshRequested();
}

void SerialPortPanel::emitConnectRequested()
{
    emit connectRequested(currentConfig());
}

void SerialPortPanel::setupUi()
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(10);

    auto* group = new QGroupBox(tr("UART 配置"), this);
    group->setObjectName(QStringLiteral("serialPortConfigGroup"));
    auto* form = new QFormLayout(group);
    form->setContentsMargins(12, 16, 12, 12);
    form->setSpacing(8);

    m_portCombo = new QComboBox(group);
    m_portCombo->setObjectName(QStringLiteral("serialPortCombo"));
    m_portCombo->setMinimumWidth(160);

    m_baudCombo = new QComboBox(group);
    m_baudCombo->setObjectName(QStringLiteral("serialBaudCombo"));
    m_baudCombo->setEditable(true);

    m_dataBitsCombo = new QComboBox(group);
    m_dataBitsCombo->setObjectName(QStringLiteral("serialDataBitsCombo"));

    m_parityCombo = new QComboBox(group);
    m_parityCombo->setObjectName(QStringLiteral("serialParityCombo"));

    m_stopBitsCombo = new QComboBox(group);
    m_stopBitsCombo->setObjectName(QStringLiteral("serialStopBitsCombo"));

    m_flowControlCombo = new QComboBox(group);
    m_flowControlCombo->setObjectName(QStringLiteral("serialFlowControlCombo"));

    m_dtrCheck = new QCheckBox(tr("DTR"), group);
    m_dtrCheck->setObjectName(QStringLiteral("serialDtrCheck"));

    m_rtsCheck = new QCheckBox(tr("RTS"), group);
    m_rtsCheck->setObjectName(QStringLiteral("serialRtsCheck"));

    auto* lineControl = new QWidget(group);
    lineControl->setObjectName(QStringLiteral("serialLineControlBox"));
    auto* lineLayout = new QHBoxLayout(lineControl);
    lineLayout->setContentsMargins(0, 0, 0, 0);
    lineLayout->setSpacing(8);
    lineLayout->addWidget(m_dtrCheck);
    lineLayout->addWidget(m_rtsCheck);
    lineLayout->addStretch();

    form->addRow(tr("端口"), m_portCombo);
    form->addRow(tr("波特率"), m_baudCombo);
    form->addRow(tr("数据位"), m_dataBitsCombo);
    form->addRow(tr("校验"), m_parityCombo);
    form->addRow(tr("停止位"), m_stopBitsCombo);
    form->addRow(tr("流控"), m_flowControlCombo);
    form->addRow(tr("控制线"), lineControl);

    auto* buttonRow = new QWidget(this);
    buttonRow->setObjectName(QStringLiteral("serialPortButtonRow"));
    auto* buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(8);

    m_refreshButton = new QPushButton(tr("刷新"), buttonRow);
    m_refreshButton->setObjectName(QStringLiteral("serialRefreshButton"));
    m_connectButton = new QPushButton(tr("连接"), buttonRow);
    m_connectButton->setObjectName(QStringLiteral("serialConnectButton"));
    m_disconnectButton = new QPushButton(tr("断开"), buttonRow);
    m_disconnectButton->setObjectName(QStringLiteral("serialDisconnectButton"));

    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_disconnectButton);
    buttonLayout->addWidget(m_connectButton);

    m_statusLabel = new QLabel(tr("未连接"), this);
    m_statusLabel->setObjectName(QStringLiteral("serialPortStatusLabel"));
    m_statusLabel->setWordWrap(true);

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setObjectName(QStringLiteral("serialUartSummaryLabel"));
    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    rootLayout->addWidget(group);
    rootLayout->addWidget(buttonRow);
    rootLayout->addWidget(m_summaryLabel);
    rootLayout->addWidget(m_statusLabel);
    rootLayout->addStretch();
}

void SerialPortPanel::setupCombos()
{
    const QList<int> baudRates = {
        9600,
        19200,
        38400,
        57600,
        115200,
        230400,
        460800,
        921600,
    };
    for (int baudRate : baudRates) {
        m_baudCombo->addItem(QString::number(baudRate));
    }
    m_baudCombo->setCurrentText(QString::number(115200));

    addEnumItem(m_dataBitsCombo, tr("5"), QSerialPort::Data5);
    addEnumItem(m_dataBitsCombo, tr("6"), QSerialPort::Data6);
    addEnumItem(m_dataBitsCombo, tr("7"), QSerialPort::Data7);
    addEnumItem(m_dataBitsCombo, tr("8"), QSerialPort::Data8);
    m_dataBitsCombo->setCurrentIndex(m_dataBitsCombo->findText(tr("8")));

    addEnumItem(m_parityCombo, tr("无校验"), QSerialPort::NoParity);
    addEnumItem(m_parityCombo, tr("偶校验"), QSerialPort::EvenParity);
    addEnumItem(m_parityCombo, tr("奇校验"), QSerialPort::OddParity);
    addEnumItem(m_parityCombo, tr("空格校验"), QSerialPort::SpaceParity);
    addEnumItem(m_parityCombo, tr("标记校验"), QSerialPort::MarkParity);

    addEnumItem(m_stopBitsCombo, tr("1"), QSerialPort::OneStop);
    addEnumItem(m_stopBitsCombo, tr("1.5"), QSerialPort::OneAndHalfStop);
    addEnumItem(m_stopBitsCombo, tr("2"), QSerialPort::TwoStop);

    addEnumItem(m_flowControlCombo, tr("无流控"), QSerialPort::NoFlowControl);
    addEnumItem(m_flowControlCombo, tr("硬件流控"), QSerialPort::HardwareControl);
    addEnumItem(m_flowControlCombo, tr("软件流控"), QSerialPort::SoftwareControl);
}

void SerialPortPanel::connectSignals()
{
    connect(m_refreshButton, &QPushButton::clicked, this, &SerialPortPanel::refreshPorts);
    connect(m_connectButton, &QPushButton::clicked, this, &SerialPortPanel::emitConnectRequested);
    connect(m_disconnectButton, &QPushButton::clicked, this, &SerialPortPanel::disconnectRequested);
    connect(m_portCombo, &QComboBox::currentTextChanged, this, &SerialPortPanel::updateSummary);
    connect(m_baudCombo, &QComboBox::currentTextChanged, this, &SerialPortPanel::updateSummary);
    connect(m_dataBitsCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &SerialPortPanel::updateSummary);
    connect(m_parityCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &SerialPortPanel::updateSummary);
    connect(m_stopBitsCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &SerialPortPanel::updateSummary);
    connect(m_flowControlCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &SerialPortPanel::updateSummary);
    connect(m_dtrCheck, &QCheckBox::toggled, this, &SerialPortPanel::updateSummary);
    connect(m_rtsCheck, &QCheckBox::toggled, this, &SerialPortPanel::updateSummary);
}

void SerialPortPanel::updateSummary()
{
    if (!m_summaryLabel) {
        return;
    }

    m_summaryLabel->setText(tr("UART: %1").arg(currentConfig().summary()));
}

void SerialPortPanel::setStatusText(const QString& text, const QString& stateName)
{
    m_statusLabel->setText(text);
    m_statusLabel->setProperty("state", stateName);
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
    m_statusLabel->update();
}

} // namespace serial_station
