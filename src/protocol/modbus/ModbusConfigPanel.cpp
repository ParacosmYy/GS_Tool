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

    // 配置变更统计: 模式/从站地址/超时时间变化时计数
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { ++m_totalConfigChanges; ++m_totalScanRequests; });
    connect(m_slaveSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() { ++m_totalConfigChanges; });
    connect(m_timeoutSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() { ++m_totalConfigChanges; });
}

QVariantMap ModbusConfigPanel::config() const {
    QVariantMap cfg;
    cfg["mode"]         = m_modeCombo->currentText();
    cfg["slaveAddress"] = m_slaveSpin->value();
    cfg["timeout"]      = m_timeoutSpin->value();
    return cfg;
}

/**
 * @brief 保存Modbus配置到QSettings
 * @param settings QSettings对象
 */
void ModbusConfigPanel::saveSettings(QSettings& settings) const
{
    settings.setValue(QStringLiteral("modbus/mode"),
                     m_modeCombo->currentIndex());
    settings.setValue(QStringLiteral("modbus/slaveAddress"),
                     m_slaveSpin->value());
    settings.setValue(QStringLiteral("modbus/timeout"),
                     m_timeoutSpin->value());
}

/**
 * @brief 从QSettings加载Modbus配置
 * @param settings QSettings对象
 */
void ModbusConfigPanel::loadSettings(QSettings& settings)
{
    m_modeCombo->setCurrentIndex(
        settings.value(QStringLiteral("modbus/mode"), 0).toInt());
    m_slaveSpin->setValue(
        settings.value(QStringLiteral("modbus/slaveAddress"), 1).toInt());
    m_timeoutSpin->setValue(
        settings.value(QStringLiteral("modbus/timeout"), 1000).toInt());
}

/**
 * @brief 重置所有统计计数器
 */
void ModbusConfigPanel::resetStatistics()
{
    m_totalConfigChanges = 0;
    m_totalScanRequests = 0;
}
