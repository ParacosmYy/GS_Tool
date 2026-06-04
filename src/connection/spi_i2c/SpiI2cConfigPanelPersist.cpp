/**
 * @file SpiI2cConfigPanelPersist.cpp
 * @brief SPI/I2C配置面板 - 设置持久化与统计重置实现
 *
 * 从 SpiI2cConfigPanel.cpp 拆分而来，包含配置的保存/加载
 * 和统计计数器重置方法。
 */

#include "connection/spi_i2c/SpiI2cConfigPanel.h"
#include <QSettings>

/** @brief 保存SPI/I2C配置到QSettings @param settings QSettings对象 */
void SpiI2cConfigPanel::saveSettings(QSettings& settings) const
{
    settings.setValue(QStringLiteral("spi_i2c/mode"), m_currentMode);
    settings.setValue(QStringLiteral("spi_i2c/adapter"),
                     m_adapterCombo->currentText());
    settings.setValue(QStringLiteral("spi_i2c/clockSpeed"),
                     m_clockSpin->value());
    settings.setValue(QStringLiteral("spi_i2c/spiMode"),
                     m_spiModeCombo->currentIndex());
    settings.setValue(QStringLiteral("spi_i2c/csPin"),
                     m_csPinSpin->value());
    settings.setValue(QStringLiteral("spi_i2c/deviceAddr"),
                     m_deviceAddrSpin->value());
}

/** @brief 从QSettings加载SPI/I2C配置 @param settings QSettings对象 */
void SpiI2cConfigPanel::loadSettings(QSettings& settings)
{
    const QString mode = settings.value(
        QStringLiteral("spi_i2c/mode"),
        QStringLiteral("spi")).toString();
    setMode(mode);

    const QString adapter = settings.value(
        QStringLiteral("spi_i2c/adapter")).toString();
    if (!adapter.isEmpty()) {
        const int idx = m_adapterCombo->findText(adapter);
        if (idx >= 0) {
            m_adapterCombo->setCurrentIndex(idx);
        }
    }

    m_clockSpin->setValue(
        settings.value(QStringLiteral("spi_i2c/clockSpeed"), 1000000).toInt());
    m_spiModeCombo->setCurrentIndex(
        settings.value(QStringLiteral("spi_i2c/spiMode"), 0).toInt());
    m_csPinSpin->setValue(
        settings.value(QStringLiteral("spi_i2c/csPin"), 0).toInt());
    m_deviceAddrSpin->setValue(
        settings.value(QStringLiteral("spi_i2c/deviceAddr"), 0).toInt());
}

/** @brief 重置所有统计计数器 */
void SpiI2cConfigPanel::resetStatistics()
{
    m_totalTransfers = 0;
    m_totalConfigChanges = 0;
    m_totalModeChanges = 0;
    m_totalSpeedChanges = 0;
    m_totalTransferErrors = 0;
}
