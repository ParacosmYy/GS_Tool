/**
 * @file ModbusConfigPanel.cpp
 * @brief Modbus配置面板实现
 */
#include "protocol/modbus/ModbusConfigPanel.h"
#include <QFormLayout>
#include <QLabel>

ModbusConfigPanel::ModbusConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("ModbusConfigPanel");

    auto* layout = new QFormLayout(this);

    // 模式选择
    m_modeCombo = new QComboBox(this);
    m_modeCombo->setObjectName("modeCombo");
    m_modeCombo->addItem(tr("RTU"));
    m_modeCombo->addItem(tr("ASCII"));
    m_modeCombo->addItem(tr("TCP"));
    layout->addRow(tr("传输模式:"), m_modeCombo);

    // 从站地址
    m_slaveSpin = new QSpinBox(this);
    m_slaveSpin->setObjectName("slaveSpin");
    m_slaveSpin->setRange(1, 247);
    m_slaveSpin->setValue(1);
    layout->addRow(tr("从站地址:"), m_slaveSpin);

    // 超时时间
    m_timeoutSpin = new QSpinBox(this);
    m_timeoutSpin->setObjectName("timeoutSpin");
    m_timeoutSpin->setRange(100, 30000);
    m_timeoutSpin->setValue(1000);
    m_timeoutSpin->setSuffix(tr(" ms"));
    layout->addRow(tr("响应超时:"), m_timeoutSpin);
}

QVariantMap ModbusConfigPanel::config() const {
    QVariantMap cfg;
    cfg["mode"]         = m_modeCombo->currentText();
    cfg["slaveAddress"] = m_slaveSpin->value();
    cfg["timeout"]      = m_timeoutSpin->value();
    return cfg;
}
