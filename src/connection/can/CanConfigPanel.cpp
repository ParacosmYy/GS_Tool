/**
 * @file CanConfigPanel.cpp
 * @brief CAN配置面板实现
 */

#include "connection/can/CanConfigPanel.h"
#include <QFormLayout>
#include <QHBoxLayout>

CanConfigPanel::CanConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_adapterCombo(new QComboBox(this))
    , m_bitrateSpin(new QSpinBox(this))
    , m_canFdCheck(new QCheckBox(tr("启用 CAN-FD"), this))
    , m_connectBtn(new QPushButton(tr("连接"), this))
{
    setObjectName("CanConfigPanel");

    m_bitrateSpin->setRange(10000, 1000000);
    m_bitrateSpin->setValue(500000);
    m_bitrateSpin->setSingleStep(100000);
    m_bitrateSpin->setSuffix(tr(" bps"));

    auto connectLayout = new QHBoxLayout();
    connectLayout->addWidget(m_canFdCheck);
    connectLayout->addStretch();
    connectLayout->addWidget(m_connectBtn);

    auto form = new QFormLayout(this);
    form->addRow(tr("CAN适配器:"), m_adapterCombo);
    form->addRow(tr("波特率:"), m_bitrateSpin);
    form->addRow(connectLayout);

    connect(m_connectBtn, &QPushButton::clicked,
            this, &CanConfigPanel::connectRequested);
}

QVariantMap CanConfigPanel::config() const
{
    QVariantMap cfg;
    cfg["adapter"] = m_adapterCombo->currentText();
    cfg["bitrate"] = m_bitrateSpin->value();
    cfg["canFd"] = m_canFdCheck->isChecked();
    return cfg;
}
