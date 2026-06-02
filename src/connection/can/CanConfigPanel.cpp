/**
 * @file CanConfigPanel.cpp
 * @brief CAN配置面板实现 — 适配器选择、波特率组合、连接切换
 */

#include "connection/can/CanConfigPanel.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QSerialPortInfo>

CanConfigPanel::CanConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_adapterCombo(new QComboBox(this))
    , m_bitrateCombo(new QComboBox(this))
    , m_canFdCheck(new QCheckBox(tr("启用 CAN-FD"), this))
    , m_connectBtn(new QPushButton(tr("连接"), this))
    , m_statusLabel(new QLabel(tr("未连接"), this))
{
    setObjectName("CanConfigPanel");

    /* 填充可用串口到适配器列表 */
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto& info : ports) {
        m_adapterCombo->addItem(info.portName() + " - " + info.description(),
                                info.portName());
    }

    /* 波特率: 125k, 250k, 500k(默认), 1M */
    struct BitrateEntry { QString label; int value; };
    const BitrateEntry bitrates[] = {
        {"125 kbps", 125000}, {"250 kbps", 250000},
        {"500 kbps", 500000}, {"1 Mbps", 1000000}
    };
    for (const auto& e : bitrates) {
        m_bitrateCombo->addItem(e.label, e.value);
    }
    m_bitrateCombo->setCurrentIndex(2);  // 默认500kbps

    /* 连接/断开按钮 + 状态 */
    auto connectLayout = new QHBoxLayout();
    connectLayout->addWidget(m_canFdCheck);
    connectLayout->addStretch();
    connectLayout->addWidget(m_statusLabel);
    connectLayout->addWidget(m_connectBtn);

    auto form = new QFormLayout(this);
    form->addRow(tr("CAN适配器:"), m_adapterCombo);
    form->addRow(tr("波特率:"), m_bitrateCombo);
    form->addRow(connectLayout);

    /* 按钮点击 → 根据当前状态发送不同信号 */
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        if (m_connected) {
            emit disconnectRequested();
        } else {
            emit connectRequested();
        }
    });
}

QVariantMap CanConfigPanel::config() const
{
    QVariantMap cfg;
    cfg["adapter"] = m_adapterCombo->currentData().toString();
    cfg["bitrate"] = m_bitrateCombo->currentData().toInt();
    cfg["canFd"] = m_canFdCheck->isChecked();
    return cfg;
}

void CanConfigPanel::setConnected(bool connected)
{
    m_connected = connected;
    m_connectBtn->setText(connected ? tr("断开") : tr("连接"));
    m_statusLabel->setText(connected ? tr("已连接") : tr("未连接"));
    m_adapterCombo->setEnabled(!connected);
    m_bitrateCombo->setEnabled(!connected);
}
