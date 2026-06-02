/**
 * @file BleConfigPanel.cpp
 * @brief BLE配置面板实现
 */

#include "connection/ble/BleConfigPanel.h"
#include <QFormLayout>
#include <QHBoxLayout>

BleConfigPanel::BleConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_deviceCombo(new QComboBox(this))
    , m_addressEdit(new QLineEdit(this))
    , m_scanBtn(new QPushButton(tr("扫描"), this))
    , m_connectBtn(new QPushButton(tr("连接"), this))
{
    setObjectName("BleConfigPanel");

    m_addressEdit->setPlaceholderText(tr("例如 00:11:22:33:44:55"));

    auto scanLayout = new QHBoxLayout();
    scanLayout->addWidget(m_deviceCombo, 1);
    scanLayout->addWidget(m_scanBtn);

    auto connectLayout = new QHBoxLayout();
    connectLayout->addWidget(m_addressEdit, 1);
    connectLayout->addWidget(m_connectBtn);

    auto form = new QFormLayout(this);
    form->addRow(tr("已发现设备:"), scanLayout);
    form->addRow(tr("设备地址:"), connectLayout);

    connect(m_scanBtn, &QPushButton::clicked, this, &BleConfigPanel::scanRequested);
    connect(m_connectBtn, &QPushButton::clicked, this, &BleConfigPanel::connectRequested);
}

QVariantMap BleConfigPanel::config() const
{
    QVariantMap cfg;
    cfg["address"] = m_addressEdit->text().trimmed();
    return cfg;
}
