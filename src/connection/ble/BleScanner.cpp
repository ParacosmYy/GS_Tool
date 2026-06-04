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
        ++m_totalScanStops;
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
    ++m_totalFilterChanges;
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
// 统计getter/clearHistory/resetScannerStatistics见 BleScannerStats.cpp
