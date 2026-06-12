#include "apps/serial_station/ui/SerialStatusBar.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStyle>

namespace serial_station {

SerialStatusBar::SerialStatusBar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("serialStatusBar"));
    setupUi();
    setSessionState(SerialSessionState::Closed);
    refreshCounters();
}

void SerialStatusBar::setSessionState(SerialSessionState state)
{
    m_stateLabel->setText(stateText(state));
    switch (state) {
    case SerialSessionState::Closed:
        m_stateLabel->setProperty("state", QStringLiteral("closed"));
        break;
    case SerialSessionState::Opening:
        m_stateLabel->setProperty("state", QStringLiteral("opening"));
        break;
    case SerialSessionState::Open:
        m_stateLabel->setProperty("state", QStringLiteral("open"));
        break;
    case SerialSessionState::Error:
        m_stateLabel->setProperty("state", QStringLiteral("error"));
        break;
    }
    m_stateLabel->style()->unpolish(m_stateLabel);
    m_stateLabel->style()->polish(m_stateLabel);
}

void SerialStatusBar::setPortConfig(const SerialPortConfig& config)
{
    const QString port = config.portName.trimmed().isEmpty() ? tr("未选择端口") : config.portName;
    m_portLabel->setText(tr("端口: %1").arg(port));
    m_baudLabel->setText(tr("波特率: %1").arg(config.baudRate));
}

void SerialStatusBar::incrementTx()
{
    ++m_txCount;
    refreshCounters();
}

void SerialStatusBar::incrementRx()
{
    ++m_rxCount;
    refreshCounters();
}

void SerialStatusBar::incrementErrors()
{
    ++m_errorCount;
    refreshCounters();
}

void SerialStatusBar::resetCounters()
{
    m_txCount = 0;
    m_rxCount = 0;
    m_errorCount = 0;
    refreshCounters();
}

void SerialStatusBar::setupUi()
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 4, 10, 4);
    layout->setSpacing(12);

    m_stateLabel = new QLabel(this);
    m_stateLabel->setObjectName(QStringLiteral("serialStatusStateLabel"));

    m_portLabel = new QLabel(tr("端口: 未选择端口"), this);
    m_portLabel->setObjectName(QStringLiteral("serialStatusPortLabel"));

    m_baudLabel = new QLabel(tr("波特率: 115200"), this);
    m_baudLabel->setObjectName(QStringLiteral("serialStatusBaudLabel"));

    m_txLabel = new QLabel(this);
    m_txLabel->setObjectName(QStringLiteral("serialStatusTxLabel"));

    m_rxLabel = new QLabel(this);
    m_rxLabel->setObjectName(QStringLiteral("serialStatusRxLabel"));

    m_errorLabel = new QLabel(this);
    m_errorLabel->setObjectName(QStringLiteral("serialStatusErrorLabel"));

    layout->addWidget(m_stateLabel);
    layout->addWidget(m_portLabel);
    layout->addWidget(m_baudLabel);
    layout->addStretch();
    layout->addWidget(m_txLabel);
    layout->addWidget(m_rxLabel);
    layout->addWidget(m_errorLabel);
}

void SerialStatusBar::refreshCounters()
{
    m_txLabel->setText(tr("TX: %1").arg(m_txCount));
    m_rxLabel->setText(tr("RX: %1").arg(m_rxCount));
    m_errorLabel->setText(tr("ERR: %1").arg(m_errorCount));
}

QString SerialStatusBar::stateText(SerialSessionState state) const
{
    switch (state) {
    case SerialSessionState::Closed:
        return tr("未连接");
    case SerialSessionState::Opening:
        return tr("连接中");
    case SerialSessionState::Open:
        return tr("已连接");
    case SerialSessionState::Error:
        return tr("错误");
    }
    return tr("未知");
}

} // namespace serial_station
