/**
 * @file BleScannerDiscovery.cpp
 * @brief BLE设备模拟发现、过滤器检查与设备列表更新
 *
 * 从 BleScanner.cpp 拆分，包含:
 * - 模拟BLE设备逐个发现的定时器回调
 * - 模拟设备列表生成
 * - 设备过滤器检查(名称前缀/地址/RSSI/隐藏无名)
 * - 已发现设备列表更新(新增或RSSI刷新)
 * - 设备名称解析(从厂商数据AD Type 0x09提取)
 */

#include "connection/ble/BleScanner.h"
#include <QDateTime>
#include <QRandomGenerator>

static constexpr int RSSI_JITTER_RANGE = 6; ///< RSSI随机波动范围(±dBm)

/// 模拟BLE设备信息（名称、地址、厂商ID、默认RSSI、设备类型）
struct SimBleDevice {
    const char* name;
    const char* address;
    int baseRssi;
    const char* type;
    const char* manufacturerId;  ///< 模拟厂商数据中的标识
};

/// 模拟BLE设备列表 — 包含各种典型BLE外设
static const SimBleDevice SIM_DEVICES[] = {
    {"HC-05",        "00:1A:7D:DA:71:01", -42, "Classic",  "Linx"},
    {"HC-06",        "00:1A:7D:DA:71:02", -55, "Classic",  "Linx"},
    {"ESP32-BLE",    "3C:61:05:12:34:56", -38, "BLE",      "Espressif"},
    {"nRF52840-DK",  "AA:BB:CC:DD:EE:01", -33, "BLE",      "Nordic"},
    {"ArduinoBLE",   "AA:BB:CC:DD:EE:02", -61, "BLE",      "Arduino"},
    {"Mi Band 7",    "F8:E4:E3:11:22:33", -47, "BLE",      "Xiaomi"},
    {"Jelly_Bean",   "44:65:0D:AA:BB:CC", -72, "BLE",      "Google"},
    {"BLE-DevKit",   "78:02:B8:CD:EF:01", -50, "BLE",      "Generic"},
    {"CC2640-Sensor","5C:31:3E:AA:BB:01", -65, "BLE",      "TI"},
    {"BST-BLE-Tag",  "B8:27:EB:CC:DD:01", -58, "BLE",      "Bosch"},
    {nullptr,        "D1:D2:D3:D4:D5:06", -80, "BLE",      nullptr},  ///< 无名设备
    {"",             "E1:E2:E3:E4:E5:07", -75, "BLE",      nullptr},  ///< 空名设备
};
static constexpr int SIM_DEVICE_COUNT = sizeof(SIM_DEVICES) / sizeof(SIM_DEVICES[0]);

/** @brief 模拟发现单个BLE设备，从队列中逐个弹出，模拟RSSI波动 */
void BleScanner::onSimulateDiscovery()
{
    if (m_simIndex >= m_simQueue.size()) {
        m_discoveryTimer->stop();

        if (m_scanElapsed.isValid()) {
            m_totalScanDurationMs += static_cast<quint64>(m_scanElapsed.elapsed());
            m_scanElapsed.invalidate();
        }

        ++m_scanCount;
        emit scanFinished();
        emit scanStateChanged(false);
        return;
    }

    QVariantMap device = m_simQueue.at(m_simIndex).toMap();

    /* 模拟RSSI波动: 在基础值上加随机偏移 */
    const int baseRssi = device.value("rssi").toInt();
    const int jitter = (QRandomGenerator::global()->bounded(2 * RSSI_JITTER_RANGE + 1)) - RSSI_JITTER_RANGE;
    const int newRssi = baseRssi + jitter;
    device["rssi"] = newRssi;

    /* 更新最后出现时间戳 */
    device["lastSeen"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    ++m_totalDiscoveryEvents;

    /* RSSI统计更新 */
    m_rssiSum += newRssi;
    ++m_rssiSampleCount;
    if (m_rssiSampleCount == 1 || newRssi > m_bestRssi) {
        m_bestRssi = newRssi;
    }
    if (m_rssiSampleCount == 1 || newRssi < m_worstRssi) {
        m_worstRssi = newRssi;
    }

    const QString addr = device.value("address").toString();

    /* RSSI历史记录 */
    m_rssiHistory[addr].append(newRssi);

    /* 过滤器检查 */
    if (!passesFilter(device)) {
        ++m_filteredDeviceCount;
        emit deviceFiltered(device, tr("不匹配当前过滤器"));
        m_simIndex++;
        return;
    }

    /* 更新或添加设备 */
    const bool isNew = updateDeviceList(device);
    if (isNew) {
        if (!m_seenAddresses.contains(addr)) {
            m_seenAddresses.append(addr);
        }
        emit deviceFound(device);
    } else {
        /* 已知设备RSSI更新 */
        emit deviceRssiUpdated(addr, newRssi);
    }

    m_simIndex++;
}

/** @brief 生成模拟BLE设备列表，填充完整设备信息到队列中 */
void BleScanner::generateSimulatedDevices()
{
    m_simQueue.clear();

    for (int i = 0; i < SIM_DEVICE_COUNT; ++i) {
        const SimBleDevice& sim = SIM_DEVICES[i];
        QVariantMap dev;
        dev["name"] = QString::fromUtf8(sim.name ? sim.name : "");
        dev["address"] = QString::fromUtf8(sim.address);
        dev["rssi"] = sim.baseRssi;
        dev["type"] = QString::fromUtf8(sim.type);

        /* 模拟厂商数据(包含厂商标识) */
        if (sim.manufacturerId) {
            QByteArray mfrData;
            mfrData.append(sim.manufacturerId, static_cast<int>(qstrlen(sim.manufacturerId)));
            dev["manufacturerData"] = mfrData.toHex();
            dev["manufacturerId"] = QString::fromUtf8(sim.manufacturerId);
        }

        /* 设备类别标记(用于过滤) */
        dev["deviceClass"] = (sim.baseRssi > -50) ? "nearby" : "distant";

        /* 连接状态(模拟: 已配对/可发现) */
        dev["pairState"] = (i % 3 == 0) ? "bonded" : "discoverable";

        m_simQueue.append(dev);
    }
}

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
