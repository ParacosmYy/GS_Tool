/**
 * @file BleScanner.cpp
 * @brief BLE设备扫描器实现
 *
 * 模拟扫描模式：通过定时器逐个生成演示BLE设备，
 * 避免Qt Bluetooth模块依赖，便于开发调试。
 */

#include "connection/ble/BleScanner.h"

static constexpr int SCAN_TIMEOUT_MS = 10000;   ///< 默认扫描超时10秒
static constexpr int DISCOVERY_INTERVAL_MS = 800; ///< 模拟发现间隔

/// 模拟BLE设备名称列表
static const QStringList SIM_DEVICE_NAMES = {
    "HC-05", "HC-06", "ESP32-BLE", "nRF52840",
    "ArduinoBLE", "Mi Band 7", "Jelly_Bean", "BLE-DevKit"
};

/// 模拟BLE设备地址列表
static const QStringList SIM_DEVICE_ADDRS = {
    "00:1A:7D:DA:71:01", "00:1A:7D:DA:71:02", "3C:61:05:12:34:56",
    "AA:BB:CC:DD:EE:01", "AA:BB:CC:DD:EE:02", "F8:E4:E3:11:22:33",
    "44:65:0D:AA:BB:CC", "78:02:B8:CD:EF:01"
};

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

/** @brief 开始BLE设备扫描，清空设备列表并启动定时器 */
void BleScanner::startScan()
{
    m_devices.clear();
    m_simIndex = 0;
    ++m_totalScanStarts;
    generateSimulatedDevices();

    m_scanTimer->start(SCAN_TIMEOUT_MS);
    m_discoveryTimer->start();
}

/** @brief 停止BLE设备扫描，停止扫描定时器和发现定时器 */
void BleScanner::stopScan()
{
    if (m_scanTimer->isActive()) {
        m_scanTimer->stop();
    }
    if (m_discoveryTimer->isActive()) {
        m_discoveryTimer->stop();
    }
}

/** @brief 获取已发现的所有BLE设备列表 @return QVariantList，每项包含name/address/rssi字段 */
QVariantList BleScanner::discoveredDevices() const
{
    return m_devices;
}

/** @brief 查询当前是否正在扫描 @return true=扫描进行中 */
bool BleScanner::isScanning() const
{
    return m_scanTimer->isActive();
}

/** @brief 扫描超时回调，停止发现定时器并发射scanFinished信号 */
void BleScanner::onScanTimeout()
{
    m_discoveryTimer->stop();
    ++m_scanCount;
    emit scanFinished();
}

/** @brief 模拟发现单个BLE设备，从队列中逐个弹出并发射deviceFound信号 */
void BleScanner::onSimulateDiscovery()
{
    if (m_simIndex >= m_simQueue.size()) {
        m_discoveryTimer->stop();
        ++m_scanCount;
        emit scanFinished();
        return;
    }

    const QVariantMap device = m_simQueue.at(m_simIndex).toMap();
    m_devices.append(device);
    ++m_totalDiscoveryEvents;

    /* 去重统计 */
    const QString addr = device.value("address").toString();
    if (!m_seenAddresses.contains(addr)) {
        m_seenAddresses.append(addr);
    }

    m_simIndex++;

    emit deviceFound(device);
}

/** @brief 生成模拟BLE设备列表，填充名称/地址/RSSI到队列中 */
void BleScanner::generateSimulatedDevices()
{
    m_simQueue.clear();
    const int count = qMin(SIM_DEVICE_NAMES.size(), SIM_DEVICE_ADDRS.size());

    for (int i = 0; i < count; ++i) {
        QVariantMap dev;
        dev["name"] = SIM_DEVICE_NAMES.at(i);
        dev["address"] = SIM_DEVICE_ADDRS.at(i);
        dev["rssi"] = -(30 + (i * 7));  // 模拟RSSI: -37 ~ -79 dBm
        dev["type"] = QStringLiteral("BLE");
        m_simQueue.append(dev);
    }
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

/** @brief 清空扫描历史记录和已发现设备地址集合 */
void BleScanner::clearHistory()
{
    m_scanCount = 0;
    m_seenAddresses.clear();
}

/** @brief 获取累计启动扫描次数 @return 启动扫描总次数 */
quint64 BleScanner::totalScanStarts() const { return m_totalScanStarts; }

/** @brief 获取累计发现设备事件次数(不去重) @return 发现设备事件总次数 */
quint64 BleScanner::totalDiscoveryEvents() const { return m_totalDiscoveryEvents; }

/** @brief 重置所有扫描器统计计数器 */
void BleScanner::resetScannerStatistics()
{
    m_totalScanStarts = 0;
    m_totalDiscoveryEvents = 0;
    m_scanCount = 0;
    m_seenAddresses.clear();
}
