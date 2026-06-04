/**
 * @file RttConfigPanelSettings.cpp
 * @brief RTT 配置面板 — 设置持久化、表单变更与统计接口
 *
 * 从 RttConfigPanel.cpp 拆分而来，包含:
 *   - 表单值变更槽 (onFormValueChanged)
 *   - 配置持久化 (saveSettings / loadSettings)
 *   - 统计计数器接口 (totalConfigChanges / totalConnectRequests / ...)
 *   - resetConfigStatistics()
 *
 * UI构建和信号连接见 RttConfigPanel.cpp。
 */

#include "rtt/RttConfigPanel.h"

// ---- 表单值变更处理 ----

/** @brief 表单值变化时的统一处理，收集当前值并发出configChanged信号 */
void RttConfigPanel::onFormValueChanged()
{
    ++m_stats.totalConfigChanges;
    emit configChanged(config());
}

// ---- 设置持久化 ----

/** @brief 保存RTT配置到QSettings @param settings QSettings对象 */
void RttConfigPanel::saveSettings(QSettings& settings) const
{
    ++m_stats.totalSettingsSaves; ///< 统计: 配置保存递增
    settings.setValue(QStringLiteral("rtt/device"),
                     m_deviceCombo->currentText());
    settings.setValue(QStringLiteral("rtt/interface"),
                     m_interfaceCombo->currentText());
    settings.setValue(QStringLiteral("rtt/speed"),
                     m_speedSpin->value());
    settings.setValue(QStringLiteral("rtt/channel"),
                     m_channelSpin->value());
}

/** @brief 从QSettings加载RTT配置并更新各控件 @param settings QSettings对象 */
void RttConfigPanel::loadSettings(QSettings& settings)
{
    ++m_stats.totalSettingsLoads; ///< 统计: 配置加载递增
    const QString device = settings.value(
        QStringLiteral("rtt/device")).toString();
    if (!device.isEmpty()) {
        const int idx = m_deviceCombo->findText(device);
        if (idx >= 0) {
            m_deviceCombo->setCurrentIndex(idx);
        } else {
            m_deviceCombo->setCurrentText(device);
        }
    }

    const QString iface = settings.value(
        QStringLiteral("rtt/interface"),
        QStringLiteral("SWD")).toString();
    const int ifaceIdx = m_interfaceCombo->findText(iface);
    if (ifaceIdx >= 0) {
        m_interfaceCombo->setCurrentIndex(ifaceIdx);
    }

    m_speedSpin->setValue(
        settings.value(QStringLiteral("rtt/speed"), 4000).toInt());
    m_channelSpin->setValue(
        settings.value(QStringLiteral("rtt/channel"), 0).toInt());
}

// ---- 统计接口 ----

/** @brief 获取配置变更总次数 @return 累计配置变更次数 */
quint64 RttConfigPanel::totalConfigChanges() const
{
    return m_stats.totalConfigChanges;
}

/** @brief 获取连接请求总次数 @return 累计连接请求次数 */
quint64 RttConfigPanel::totalConnectRequests() const
{
    return m_stats.totalConnectRequests;
}

/** @brief 获取断开请求总次数 @return 累计断开请求次数 */
quint64 RttConfigPanel::totalDisconnectRequests() const
{
    return m_stats.totalDisconnectRequests;
}

/** @brief 重置配置面板统计计数器为初始值 */
void RttConfigPanel::resetConfigStatistics()
{
    m_stats = Stats{};
}
