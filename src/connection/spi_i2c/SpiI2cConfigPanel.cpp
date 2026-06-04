/**
 * @file SpiI2cConfigPanel.cpp
 * @brief SPI/I2C配置面板实现
 */

#include "connection/spi_i2c/SpiI2cConfigPanel.h"
#include <QFormLayout>

/** @brief 构造SPI/I2C配置面板UI，初始化布局和信号 @param parent 父控件 */
SpiI2cConfigPanel::SpiI2cConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("SpiI2cConfigPanel");
    setupUi();
    setupConnections();
    updateModeVisibility();
}

/** @brief 获取当前配置参数，根据模式包含不同字段 @return 包含busMode/adapter/clockSpeed等字段的配置Map */
QVariantMap SpiI2cConfigPanel::config() const
{
    QVariantMap cfg;
    cfg["busMode"] = m_currentMode;

    if (m_adapterCombo) {
        cfg["adapter"] = m_adapterCombo->currentText();
    }
    if (m_clockSpin) {
        cfg["clockSpeed"] = m_clockSpin->value();
    }

    if (m_currentMode == "spi") {
        if (m_spiModeCombo) {
            cfg["spiMode"] = m_spiModeCombo->currentIndex();
        }
        if (m_csPinSpin) {
            cfg["csPin"] = m_csPinSpin->value();
        }
    } else {
        if (m_deviceAddrSpin) {
            cfg["deviceAddress"] = m_deviceAddrSpin->value();
        }
    }
    return cfg;
}

/** @brief 设置当前总线模式并同步UI @param mode "spi"或"i2c" */
void SpiI2cConfigPanel::setMode(const QString& mode)
{
    m_currentMode = mode;
    /// 同步下拉框(阻止信号防止递归触发onModeChanged)
    if (m_busModeCombo) {
        m_busModeCombo->blockSignals(true);
        int idx = (mode == "i2c") ? 1 : 0;
        m_busModeCombo->setCurrentIndex(idx);
        m_busModeCombo->blockSignals(false);
    }
    updateModeVisibility();
}

/** @brief 设置连接状态，更新按钮文本和状态标签 @param connected true=已连接 */
void SpiI2cConfigPanel::setConnected(bool connected)
{
    m_connected = connected;
    if (m_connectBtn) {
        m_connectBtn->setText(connected ? tr("断开") : tr("连接"));
    }
    if (m_statusLabel) {
        m_statusLabel->setText(connected ? tr("已连接") : tr("未连接"));
    }
}

/** @brief 连接按钮点击处理，切换连接/断开并发射相应信号 */
void SpiI2cConfigPanel::onConnectClicked()
{
    if (m_connected) {
        /// 断开连接
        m_connected = false;
        if (m_connectBtn) {
            m_connectBtn->setText(tr("连接"));
        }
        if (m_statusLabel) {
            m_statusLabel->setText(tr("已断开"));
        }
        emit disconnectRequested();
    } else {
        /// 发起连接(传输)
        ++m_totalTransfers;
        emit connectRequested(config());
    }
}

/** @brief 模式切换(SPI/I2C)回调，更新当前模式和UI可见性 @param index 下拉框当前索引 */
void SpiI2cConfigPanel::onModeChanged(int index)
{
    m_currentMode = (index == 1) ? "i2c" : "spi";
    ++m_totalModeChanges;
    updateModeVisibility();
}

// setupUi/setupConnections/updateModeVisibility见 SpiI2cConfigPanelUI.cpp
// saveSettings/loadSettings/resetStatistics见 SpiI2cConfigPanelPersist.cpp
