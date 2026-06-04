/**
 * @file BleScannerFilter.cpp
 * @brief BLE设备过滤器检查、设备列表更新与名称解析
 *
 * 从 BleScannerDiscovery.cpp 拆分而来，包含:
 *   - passesFilter():       按名称前缀/地址/RSSI阈值/隐藏无名过滤设备
 *   - updateDeviceList():   更新或添加设备到已发现列表(新设备/RSSI刷新)
 *   - resolveDeviceName():  从厂商数据AD Type 0x09提取设备名称
 *
 * 模拟BLE发现流程和设备生成见 BleScannerDiscovery.cpp。
 */

#include "connection/ble/BleScanner.h"

/** @brief 检查设备是否通过所有过滤条件 @param device 待检查设备 @return true=通过 */
bool BleScanner::passesFilter(const QVariantMap& device) const
{
    /* 名称前缀过滤 */
    if (!m_filter.namePrefix.isEmpty()) {
        const QString name = device.value("name").toString();
        if (!name.startsWith(m_filter.namePrefix, Qt::CaseInsensitive)) {
            return false;
        }
    }

    /* 地址子串过滤 */
    if (!m_filter.addressFilter.isEmpty()) {
        const QString addr = device.value("address").toString();
        if (!addr.contains(m_filter.addressFilter, Qt::CaseInsensitive)) {
            return false;
        }
    }

    /* RSSI阈值过滤 */
    const int rssi = device.value("rssi").toInt();
    if (rssi < m_filter.minRssi) {
        return false;
    }

    /* 隐藏无名设备 */
    if (m_filter.hideUnnamed) {
        const QString name = device.value("name").toString().trimmed();
        if (name.isEmpty()) {
            return false;
        }
    }

    return true;
}

/** @brief 更新或添加设备到已发现列表 @param device 设备信息 @return true=新设备, false=已有设备更新 */
bool BleScanner::updateDeviceList(const QVariantMap& device)
{
    const QString addr = device.value("address").toString();

    if (m_deviceByAddr.contains(addr)) {
        /* 已有设备: 更新RSSI和最后出现时间 */
        QVariantMap& existing = m_deviceByAddr[addr];
        existing["rssi"] = device.value("rssi");
        existing["lastSeen"] = device.value("lastSeen");

        /* 如果获得了设备名称，更新名称 */
        const QString newName = device.value("name").toString();
        if (!newName.isEmpty() && existing.value("name").toString().isEmpty()) {
            existing["name"] = newName;
        }
        return false;
    }

    /* 新设备: 添加到列表 */
    m_deviceByAddr[addr] = device;
    m_devices.append(device);
    return true;
}

/** @brief 解析设备厂商数据中的名称 @param manufacturerData 原始厂商数据 @return 解析出的名称 */
QString BleScanner::resolveDeviceName(const QByteArray& manufacturerData) const
{
    /* 尝试从厂商数据中提取完整设备名(AD Type 0x09) */
    if (manufacturerData.size() < 2) {
        return {};
    }

    /* 简化解析: 查找可读ASCII字符串段 */
    for (int i = 0; i < manufacturerData.size() - 2; ++i) {
        const uint8_t len = static_cast<uint8_t>(manufacturerData.at(i));
        if (i + 1 < manufacturerData.size()) {
            const uint8_t type = static_cast<uint8_t>(manufacturerData.at(i + 1));
            if (type == 0x09 && len > 1 && i + 1 + len <= manufacturerData.size()) {
                return QString::fromUtf8(
                    manufacturerData.mid(i + 2, len - 1));
            }
        }
    }
    return {};
}
