/**
 * @file BleScanner.cpp
 * @brief BLE设备扫描器实现
 *
 * 模拟扫描模式：通过定时器逐个生成演示BLE设备，
 * 支持RSSI波动模拟、设备名称解析、过滤器和统计。
 */

#include "connection/ble/BleScanner.h"
#include <QDateTime>
#include <QRandomGenerator>

static constexpr int SCAN_TIMEOUT_MS = 10000;    ///< 默认扫描超时10秒
static constexpr int DISCOVERY_INTERVAL_MS = 800; ///< 模拟发现间隔
static constexpr int RSSI_JITTER_RANGE = 6;       ///< RSSI随机波动范围(±dBm)

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

/** @brief 构造BLE扫描器，初始化扫描定时器和模拟发现定时器 @param parent 父QObject指针 */
BleScanner::BleScanner(QObject* parent)
    : QObject(parent)
    , m_scanTimer(new QTimer(this))
    , m_discoveryTimer(new QTimer(this))
{
    m_scanTimer->setSingleShot(true);
    connect(m_scanTimer, &QTimer::timeout,
            this, &BleScanner::onScanTimeout);

    m_discoveryTimer->setInterval(DISCOVERY_INTERVAL_MS);
    connect(m_discoveryTimer, &QTimer::timeout,
            this, &BleScanner::onSimulateDiscovery);
}

/** @brief 析构BLE扫描器，停止正在进行的扫描 */
BleScanner::~BleScanner()
{
    stopScan();
}

/** @brief 开始BLE设备扫描，清空设备列表、重置统计并启动定时器 */
void BleScanner::startScan()
{
    m_devices.clear();
    m_deviceByAddr.clear();
    m_simIndex = 0;
    ++m_totalScanStarts;
    generateSimulatedDevices();

    m_scanElapsed.start();
    m_scanTimer->start(SCAN_TIMEOUT_MS);
    m_discoveryTimer->start();

    emit scanStateChanged(true);
}

/** @brief 停止BLE设备扫描，停止扫描定时器和发现定时器 */
void BleScanner::stopScan()
{
    const bool wasScanning = m_scanTimer->isActive() || m_discoveryTimer->isActive();

    if (m_scanTimer->isActive()) {
        m_scanTimer->stop();
    }
    if (m_discoveryTimer->isActive()) {
        m_discoveryTimer->stop();
    }

    if (wasScanning) {
        /* 记录本次扫描时长 */
        if (m_scanElapsed.isValid()) {
            m_totalScanDurationMs += static_cast<quint64>(m_scanElapsed.elapsed());
            m_scanElapsed.invalidate();
        }
        emit scanStateChanged(false);
    }
}

/** @brief 设置扫描过滤器 @param filter 过滤条件 */
void BleScanner::setFilter(const ScanFilter& filter)
{
    m_filter = filter;
}

/** @brief 获取当前扫描过滤器 @return 当前过滤器配置 */
BleScanner::ScanFilter BleScanner::filter() const
{
    return m_filter;
}

/** @brief 获取已发现的所有BLE设备列表 @return QVariantList，每项包含name/address/rssi/type/lastSeen字段 */
QVariantList BleScanner::discoveredDevices() const
{
    return m_devices;
}

/** @brief 根据地址查找设备信息 @param address BLE设备地址 @return 设备信息Map，未找到返回空Map */
QVariantMap BleScanner::deviceByAddress(const QString& address) const
{
    return m_deviceByAddr.value(address, QVariantMap());
}

/** @brief 查询当前是否正在扫描 @return true=扫描进行中 */
bool BleScanner::isScanning() const
{
    return m_scanTimer->isActive() || m_discoveryTimer->isActive();
}

/** @brief 获取当前扫描已持续时间(毫秒) @return 扫描时长，未扫描时返回0 */
qint64 BleScanner::scanElapsedTime() const
{
    if (!m_scanElapsed.isValid()) {
        return 0;
    }
    return m_scanElapsed.elapsed();
}

/** @brief 扫描超时回调，停止发现定时器并发射scanFinished信号 */
void BleScanner::onScanTimeout()
{
    m_discoveryTimer->stop();

    /* 记录扫描时长 */
    if (m_scanElapsed.isValid()) {
        m_totalScanDurationMs += static_cast<quint64>(m_scanElapsed.elapsed());
        m_scanElapsed.invalidate();
    }

    ++m_scanCount;
    emit scanFinished();
    emit scanStateChanged(false);
}

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

/** @brief 获取已完成的扫描次数 @return 累计扫描完成计数 */
int BleScanner::scanCount() const
{
    return m_scanCount;
}

/** @brief 获取累计发现的设备总数(去重后) @return 不同设备地址的数量 */
int BleScanner::totalDevicesFound() const
{
    return m_seenAddresses.size();
}

/** @brief 获取被过滤器过滤掉的设备数量 */
quint64 BleScanner::filteredDeviceCount() const
{
    return m_filteredDeviceCount;
}

/** @brief 清空扫描历史记录和已发现设备地址集合 */
void BleScanner::clearHistory()
{
    m_scanCount = 0;
    m_seenAddresses.clear();
    m_deviceByAddr.clear();
    m_rssiHistory.clear();
    m_filteredDeviceCount = 0;
    m_totalScanDurationMs = 0;
    m_bestRssi = 0;
    m_worstRssi = 0;
    m_rssiSum = 0;
    m_rssiSampleCount = 0;
}

/** @brief 获取累计启动扫描次数 @return 启动扫描总次数 */
quint64 BleScanner::totalScanStarts() const { return m_totalScanStarts; }

/** @brief 获取累计发现设备事件次数(不去重，包含RSSI更新) @return 发现设备事件总次数 */
quint64 BleScanner::totalDiscoveryEvents() const { return m_totalDiscoveryEvents; }

/** @brief 获取累计扫描总时长(毫秒) */
quint64 BleScanner::totalScanDurationMs() const { return m_totalScanDurationMs; }

/** @brief 获取最佳RSSI值(信号最强) */
int BleScanner::bestRssi() const { return m_bestRssi; }

/** @brief 获取最差RSSI值(信号最弱) */
int BleScanner::worstRssi() const { return m_worstRssi; }

/** @brief 获取平均RSSI值 */
double BleScanner::averageRssi() const
{
    if (m_rssiSampleCount == 0) return 0.0;
    return static_cast<double>(m_rssiSum) / static_cast<double>(m_rssiSampleCount);
}

/** @brief 重置所有扫描器统计计数器 */
void BleScanner::resetScannerStatistics()
{
    m_totalScanStarts = 0;
    m_totalDiscoveryEvents = 0;
    m_scanCount = 0;
    m_seenAddresses.clear();
    m_filteredDeviceCount = 0;
    m_totalScanDurationMs = 0;
    m_bestRssi = 0;
    m_worstRssi = 0;
    m_rssiSum = 0;
    m_rssiSampleCount = 0;
    m_rssiHistory.clear();
    m_deviceByAddr.clear();
}
