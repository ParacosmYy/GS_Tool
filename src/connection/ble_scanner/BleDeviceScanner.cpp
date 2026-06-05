/**
 * @file BleDeviceScanner.cpp
 * @brief BLE设备扫描器实现 — 扫描控制、模拟发现、设备缓存与RSSI跟踪
 *
 * 模拟模式下通过定时器逐个弹出预生成的BLE设备。
 * 设备按MAC地址去重，RSSI历史保留最近20个采样点。
 * 统计getter见 BleDeviceScannerStats.cpp。
 */

#include "connection/ble_scanner/BleDeviceScanner.h"

#include <QRandomGenerator>

static constexpr int RSSI_HISTORY_MAX = 20; ///< RSSI历史最大采样数

/** @brief 构造BLE设备扫描器，初始化扫描和发现定时器 @param parent 父QObject指针 */
BleDeviceScanner::BleDeviceScanner(QObject* parent)
    : QObject(parent)
    , m_scanTimer(new QTimer(this))
    , m_discoveryTimer(new QTimer(this))
{
    m_scanTimer->setSingleShot(true);
    connect(m_scanTimer, &QTimer::timeout,
            this, &BleDeviceScanner::onScanTimeout);

    connect(m_discoveryTimer, &QTimer::timeout,
            this, &BleDeviceScanner::onSimulateDiscovery);
}

/** @brief 析构，确保扫描停止 */
BleDeviceScanner::~BleDeviceScanner()
{
    stopScan();
}

/** @brief 开始BLE扫描，清空缓存、生成模拟队列并启动定时器 */
void BleDeviceScanner::startScan()
{
    m_deviceCache.clear();
    m_rssiHistoryMap.clear();
    m_simIndex = 0;
    ++m_totalScanStarts;

    generateSimulatedDevices();

    m_elapsed.start();
    m_scanTimer->start(m_scanTimeoutMs);
    m_discoveryTimer->start(m_discoveryIntervalMs);
}

/** @brief 停止BLE扫描，停止定时器并记录时长 */
void BleDeviceScanner::stopScan()
{
    const bool wasScanning = m_scanTimer->isActive()
                             || m_discoveryTimer->isActive();
    if (m_scanTimer->isActive()) {
        m_scanTimer->stop();
    }
    if (m_discoveryTimer->isActive()) {
        m_discoveryTimer->stop();
    }
    if (wasScanning) {
        ++m_totalScanStops;
        if (m_elapsed.isValid()) {
            m_totalScanDurationMs += static_cast<quint64>(m_elapsed.elapsed());
            m_elapsed.invalidate();
        }
    }
}

/** @brief 查询当前是否正在扫描 @return true=扫描进行中 */
bool BleDeviceScanner::isScanning() const
{
    return m_scanTimer->isActive() || m_discoveryTimer->isActive();
}

/** @brief 设置扫描超时 @param ms 毫秒数 */
void BleDeviceScanner::setScanTimeout(int ms)
{
    m_scanTimeoutMs = (ms > 0) ? ms : 10000;
}

/** @brief 设置模拟发现间隔 @param ms 毫秒数 */
void BleDeviceScanner::setDiscoveryInterval(int ms)
{
    m_discoveryIntervalMs = (ms > 0) ? ms : 600;
    m_discoveryTimer->setInterval(m_discoveryIntervalMs);
}

/** @brief 获取所有已发现设备列表 @return BleDeviceInfo列表 */
QList<BleDeviceInfo> BleDeviceScanner::discoveredDevices() const
{
    return m_deviceCache.values();
}

/** @brief 按地址查找设备 @param address MAC地址 @return 设备信息 */
BleDeviceInfo BleDeviceScanner::deviceByAddress(const QString& address) const
{
    return m_deviceCache.value(address);
}

/** @brief 获取已发现设备数量 @return 设备数量 */
int BleDeviceScanner::deviceCount() const
{
    return m_deviceCache.size();
}

/** @brief 获取指定地址的RSSI历史 @param address 设备地址 @return RSSI采样列表 */
QList<int> BleDeviceScanner::rssiHistory(const QString& address) const
{
    return m_rssiHistoryMap.value(address);
}

/** @brief 扫描超时回调，停止发现定时器并发射scanComplete信号 */
void BleDeviceScanner::onScanTimeout()
{
    m_discoveryTimer->stop();
    if (m_elapsed.isValid()) {
        m_totalScanDurationMs += static_cast<quint64>(m_elapsed.elapsed());
        m_elapsed.invalidate();
    }
    ++m_totalScanCycles;
    emit scanComplete();
}

/** @brief 模拟发现单个设备（含RSSI随机波动），弹出队列设备 */
void BleDeviceScanner::onSimulateDiscovery()
{
    if (m_simIndex >= m_simQueue.size()) {
        /* 队列耗尽后对已发现设备做RSSI波动更新 */
        if (m_deviceCache.isEmpty()) {
            return;
        }
        auto keys = m_deviceCache.keys();
        const QString& addr = keys.at(
            QRandomGenerator::global()->bounded(keys.size()));
        BleDeviceInfo info = m_deviceCache.value(addr);
        const int delta = QRandomGenerator::global()->bounded(-6, 7);
        info.rssi = qBound(-128, info.rssi + delta, 0);
        info.lastSeen = QDateTime::currentDateTime();
        addOrUpdateDevice(info);
        return;
    }

    BleDeviceInfo info = m_simQueue.at(m_simIndex);
    ++m_simIndex;

    /* 随机RSSI波动 */
    const int jitter = QRandomGenerator::global()->bounded(-8, 9);
    info.rssi = qBound(-128, info.rssi + jitter, 0);
    info.lastSeen = QDateTime::currentDateTime();

    addOrUpdateDevice(info);
}

/** @brief 生成模拟BLE设备队列 */
void BleDeviceScanner::generateSimulatedDevices()
{
    m_simQueue.clear();

    struct SimEntry {
        const char* name;
        const char* addr;
        int baseRssi;
        const char* svcUuid;
    };

    static const SimEntry entries[] = {
        {"nRF52840-DK",   "AA:BB:CC:11:22:33", -42, "0000180A-0000-1000-8000-00805F9B34FB"},
        {"ESP32-BLE",     "DD:EE:FF:44:55:66", -58, "0000180F-0000-1000-8000-00805F9B34FB"},
        {"STM32WB55",     "11:22:33:AA:BB:CC", -65, "00001800-0000-1000-8000-00805F9B34FB"},
        {"Heart Rate",    "44:55:66:DD:EE:FF", -50, "0000180D-0000-1000-8000-00805F9B34FB"},
        {"Beacon-01",     "77:88:99:00:11:22", -73, "0000FEAA-0000-1000-8000-00805F9B34FB"},
        {"Mi Band 7",     "AA:11:BB:22:CC:33", -55, "0000FEE0-0000-1000-8000-00805F9B34FB"},
        {"Tag-Locator",   "DD:44:EE:55:FF:66", -81, "0000FD6F-0000-1000-8000-00805F9B34FB"},
        {"Battery Mon",   "11:AA:22:BB:33:CC", -47, "0000180F-0000-1000-8000-00805F9B34FB"},
        {"UART Bridge",   "44:DD:55:EE:66:FF", -62, "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"},
        {"",              "77:FF:88:00:99:11", -90, "0000FEAA-0000-1000-8000-00805F9B34FB"},
    };

    for (const auto& e : entries) {
        BleDeviceInfo info;
        info.name = QString::fromLatin1(e.name);
        info.address = QString::fromLatin1(e.addr);
        info.rssi = e.baseRssi;
        info.serviceUuids << QString::fromLatin1(e.svcUuid);
        info.type = classifyByServices(info.serviceUuids);
        m_simQueue.append(info);
    }
}

/** @brief 添加或更新设备到缓存（按地址去重），更新RSSI历史并发射信号 */
void BleDeviceScanner::addOrUpdateDevice(const BleDeviceInfo& info)
{
    ++m_totalDiscoveryEvents;

    /* 更新RSSI统计 */
    m_rssiSum += info.rssi;
    ++m_rssiSampleCount;
    if (m_rssiSampleCount == 1 || info.rssi > m_bestRssi) {
        m_bestRssi = info.rssi;
    }
    if (m_rssiSampleCount == 1 || info.rssi < m_worstRssi) {
        m_worstRssi = info.rssi;
    }

    /* 更新RSSI历史 */
    QList<int>& history = m_rssiHistoryMap[info.address];
    history.append(info.rssi);
    if (history.size() > RSSI_HISTORY_MAX) {
        history.removeFirst();
    }

    const bool isNew = !m_deviceCache.contains(info.address);
    m_deviceCache.insert(info.address, info);

    if (isNew) {
        emit deviceFound(info);
    } else {
        emit deviceUpdated(info);
    }
}

/** @brief 根据服务UUID列表分类设备角色 @param uuids 服务UUID列表 @return 设备角色 */
BleDeviceType BleDeviceScanner::classifyByServices(
    const QStringList& uuids) const
{
    for (const QString& uuid : uuids) {
        const QString upper = uuid.toUpper();
        /* 外围设备常见UUID: 心率、电池、信标、UART等 */
        if (upper.startsWith("0000180") || upper.contains("FEAA")
            || upper.contains("FEE0") || upper.contains("FD6F")
            || upper.contains("6E400001")) {
            return BleDeviceType::Peripheral;
        }
        /* 通用访问/连接管理 — 多为中心设备 */
        if (upper.startsWith("00001800")) {
            return BleDeviceType::Central;
        }
    }
    return BleDeviceType::Unknown;
}

// 统计getter / resetStatistics 见 BleDeviceScannerStats.cpp
