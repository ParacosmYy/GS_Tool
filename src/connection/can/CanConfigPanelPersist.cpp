/**
 * @file CanConfigPanelPersist.cpp
 * @brief CAN配置面板 — 设置持久化与统计重置
 *
 * 从 CanConfigPanel.cpp 拆分而来，包含:
 *   - saveSettings(): 将CAN配置写入QSettings
 *   - loadSettings(): 从QSettings恢复CAN配置
 *   - resetStatistics(): 重置统计计数器
 *
 * 面板构造/UI/连接逻辑保留在 CanConfigPanel.cpp。
 */

#include "connection/can/CanConfigPanel.h"

// ============================================================
// 设置持久化
// ============================================================

/** @brief 保存CAN配置到QSettings @param settings QSettings对象 */
void CanConfigPanel::saveSettings(QSettings& settings) const
{
    settings.setValue(QStringLiteral("can/adapter"),
                     m_adapterCombo->currentData().toString());
    settings.setValue(QStringLiteral("can/bitrateIndex"),
                     m_bitrateCombo->currentIndex());
    settings.setValue(QStringLiteral("can/canFd"),
                     m_canFdCheck->isChecked());
    settings.setValue(QStringLiteral("can/samplePoint"),
                     m_samplePointSpin->value());
    settings.setValue(QStringLiteral("can/sjw"),
                     m_sjwSpin->value());
    settings.setValue(QStringLiteral("can/filterId"),
                     m_filterIdEdit->text());
    settings.setValue(QStringLiteral("can/filterMask"),
                     m_filterMaskEdit->text());
    settings.setValue(QStringLiteral("can/filterExt"),
                     m_filterExtCheck->isChecked());
}

/** @brief 从QSettings加载CAN配置 @param settings QSettings对象 */
void CanConfigPanel::loadSettings(QSettings& settings)
{
    const QString adapter = settings.value(
        QStringLiteral("can/adapter")).toString();
    if (!adapter.isEmpty()) {
        const int idx = m_adapterCombo->findData(adapter);
        if (idx >= 0) {
            m_adapterCombo->setCurrentIndex(idx);
        }
    }
    m_bitrateCombo->setCurrentIndex(
        settings.value(QStringLiteral("can/bitrateIndex"), 2).toInt());
    m_canFdCheck->setChecked(
        settings.value(QStringLiteral("can/canFd"), false).toBool());
    m_samplePointSpin->setValue(
        settings.value(QStringLiteral("can/samplePoint"), 0.875).toDouble());
    m_sjwSpin->setValue(
        settings.value(QStringLiteral("can/sjw"), 1).toInt());
    m_filterIdEdit->setText(
        settings.value(QStringLiteral("can/filterId")).toString());
    m_filterMaskEdit->setText(
        settings.value(QStringLiteral("can/filterMask")).toString());
    m_filterExtCheck->setChecked(
        settings.value(QStringLiteral("can/filterExt"), false).toBool());
}

// ============================================================
// 统计重置
// ============================================================

/** @brief 重置所有统计计数器 */
void CanConfigPanel::resetStatistics()
{
    m_totalConfigChanges = 0;
    m_totalBusResets = 0;
}
