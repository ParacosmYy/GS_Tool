/**
 * @file BleConfigPanelSlots.cpp
 * @brief BLE配置面板 — 设备发现与交互槽函数实现
 *
 * 从 BleConfigPanel.cpp 拆分而来，包含设备发现回调、
 * 扫描完成回调、设备选中回调和统计重置方法。
 */

#include "connection/ble/BleConfigPanel.h"

/** @brief 设备发现回调，按地址去重并更新下拉框 @param device 包含name/address/rssi的设备信息Map */
void BleConfigPanel::onDeviceFound(const QVariantMap& device)
{
    const QString name = device.value("name").toString();
    const QString addr = device.value("address").toString();
    const int rssi = device.value("rssi").toInt();

    /* 去重: 按地址检查是否已存在 */
    for (int i = 0; i < m_deviceList.size(); ++i) {
        if (m_deviceList.at(i).toMap().value("address").toString() == addr) {
            /* 更新已有条目的RSSI和名称 */
            m_deviceList[i] = device;
            m_deviceCombo->setItemText(i,
                QStringLiteral("%1 (%2) [%3 dBm]")
                    .arg(name, addr, QString::number(rssi)));
            return;
        }
    }

    const QString display = QStringLiteral("%1 (%2) [%3 dBm]")
        .arg(name, addr, QString::number(rssi));

    m_deviceCombo->addItem(display);
    m_deviceList.append(device);
    ++m_totalDevicesDiscovered;
    m_statusLabel->setText(
        tr("发现设备: %1").arg(name));
}

/** @brief 扫描完成回调，更新状态标签显示发现设备数量 */
void BleConfigPanel::onScanFinished()
{
    m_statusLabel->setText(
        tr("扫描完成，发现 %1 个设备").arg(m_deviceList.size()));
}

/** @brief 设备选中回调，将选中设备地址填入输入框 @param index 下拉框选中索引 */
void BleConfigPanel::onDeviceSelected(int index)
{
    if (index >= 0 && index < m_deviceList.size()) {
        ++m_totalDeviceSelections;
        const QVariantMap dev = m_deviceList.at(index).toMap();
        m_addressEdit->setText(dev.value("address").toString());
    }
}

/** @brief 保存BLE地址配置到QSettings @param settings QSettings对象 */
void BleConfigPanel::saveSettings(QSettings& settings) const
{
    settings.setValue(QStringLiteral("ble/address"),
                     m_addressEdit->text().trimmed());
}

/** @brief 从QSettings加载BLE地址配置 @param settings QSettings对象 */
void BleConfigPanel::loadSettings(QSettings& settings)
{
    const QString addr = settings.value(
        QStringLiteral("ble/address")).toString();
    if (!addr.isEmpty()) {
        m_addressEdit->setText(addr);
    }
}

/** @brief 重置所有统计计数器(扫描/设备选择/连接/发现设备/地址编辑) */
void BleConfigPanel::resetStatistics()
{
    m_totalScansInitiated = 0;
    m_totalDeviceSelections = 0;
    m_totalConnectAttempts = 0;
    m_totalDevicesDiscovered = 0;
    m_totalAddressEdits = 0;
}
