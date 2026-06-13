#include "apps/serial_station/ui/SerialProtocolPanel.h"

#include <QtCore/QSignalBlocker>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>

namespace serial_station {

SerialProtocolPanel::SerialProtocolPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("serialProtocolPanel"));
    setupUi();
    connectSignals();
    refreshStatus();
}

QString SerialProtocolPanel::activeProtocol() const
{
    return m_protocolCombo->currentData().toString();
}

void SerialProtocolPanel::setProtocols(const QStringList& protocolNames,
                                       const QString& activeProtocol)
{
    const QSignalBlocker blocker(m_protocolCombo);
    m_protocolCombo->clear();

    for (const QString& protocolName : protocolNames) {
        const QString trimmed = protocolName.trimmed();
        if (!trimmed.isEmpty()) {
            m_protocolCombo->addItem(trimmed, trimmed);
        }
    }

    setActiveProtocol(activeProtocol);
    const bool hasProtocols = m_protocolCombo->count() > 0;
    m_protocolCombo->setEnabled(hasProtocols);
    m_hintLabel->setEnabled(hasProtocols);
    refreshStatus();
}

void SerialProtocolPanel::setActiveProtocol(const QString& protocolName)
{
    const QString trimmed = protocolName.trimmed();
    const int index = m_protocolCombo->findData(trimmed);
    if (index >= 0) {
        const QSignalBlocker blocker(m_protocolCombo);
        m_protocolCombo->setCurrentIndex(index);
    }

    refreshStatus();
}

void SerialProtocolPanel::emitProtocolSelected(int index)
{
    const QString protocolName = m_protocolCombo->itemData(index).toString();
    if (!protocolName.trimmed().isEmpty()) {
        emit protocolSelected(protocolName);
    }

    refreshStatus();
}

void SerialProtocolPanel::setupUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    auto* title = new QLabel(tr("协议"), this);
    title->setObjectName(QStringLiteral("serialProtocolTitle"));

    m_hintLabel = new QLabel(tr("选择 protocol 模式发送和接收解析使用的默认协议"), this);
    m_hintLabel->setObjectName(QStringLiteral("serialProtocolHint"));
    m_hintLabel->setWordWrap(true);

    auto* comboRow = new QHBoxLayout();
    comboRow->setContentsMargins(0, 0, 0, 0);
    comboRow->setSpacing(8);

    auto* protocolLabel = new QLabel(tr("默认协议"), this);
    protocolLabel->setObjectName(QStringLiteral("serialProtocolComboLabel"));

    m_protocolCombo = new QComboBox(this);
    m_protocolCombo->setObjectName(QStringLiteral("serialProtocolCombo"));

    comboRow->addWidget(protocolLabel);
    comboRow->addWidget(m_protocolCombo, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName(QStringLiteral("serialProtocolStatusLabel"));
    m_statusLabel->setWordWrap(true);

    root->addWidget(title);
    root->addWidget(m_hintLabel);
    root->addLayout(comboRow);
    root->addWidget(m_statusLabel);
}

void SerialProtocolPanel::connectSignals()
{
    connect(m_protocolCombo, QOverload<int>::of(&QComboBox::activated),
            this, &SerialProtocolPanel::emitProtocolSelected);
}

void SerialProtocolPanel::refreshStatus()
{
    const QString protocolName = activeProtocol();
    if (protocolName.isEmpty()) {
        m_statusLabel->setText(tr("未加载协议"));
        m_statusLabel->setProperty("state", QStringLiteral("empty"));
    } else {
        m_statusLabel->setText(tr("当前协议: %1").arg(protocolName));
        m_statusLabel->setProperty("state", QStringLiteral("active"));
    }
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
}

} // namespace serial_station
