/**
 * @file BleScanner.cpp
 * @brief BLE设备扫描器实现 — 扫描控制、设备查询与统计
 *
 * 模拟扫描模式：通过定时器逐个生成演示BLE设备。
 * 设备发现、过滤、列表更新和名称解析见 BleScannerDiscovery.cpp。
 */

#include "connection/ble/BleScanner.h"

static constexpr int SCAN_TIMEOUT_MS = 10000;    ///< 默认扫描超时10秒
static constexpr int DISCOVERY_INTERVAL_MS = 800; ///< 模拟发现间隔

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
    ++m_totalScanCycles;
    emit scanFinished();
    emit scanStateChanged(false);
}

// onSimulateDiscovery/generateSimulatedDevices/passesFilter/updateDeviceList/resolveDeviceName
// 见 BleScannerDiscovery.cpp

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

/** @brief 获取累计扫描周期完成次数 @return 扫描周期完成总次数 */
quint64 BleScanner::totalScanCycles() const { return m_totalScanCycles; }

/** @brief 获取累计去重设备地址总数 @return 不同设备地址的总数 */
quint64 BleScanner::uniqueDevicesSeen() const { return static_cast<quint64>(m_seenAddresses.size()); }

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
    m_totalScanCycles = 0;
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
