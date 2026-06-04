/**
 * @file BleScannerStats.cpp
 * @brief BLE设备扫描器 — 统计计数器与历史管理接口实现
 *
 * 从 BleScanner.cpp 拆分而来，包含:
 *   - 统计getter (scanCount / totalDevicesFound / filteredDeviceCount / ...)
 *   - clearHistory(): 清空扫描历史记录
 *   - resetScannerStatistics(): 重置所有统计计数器
 *   - RSSI聚合查询 (bestRssi / worstRssi / averageRssi)
 *
 * 扫描控制、设备查询、发现逻辑见 BleScanner.cpp / BleScannerDiscovery.cpp。
 */

#include "connection/ble/BleScanner.h"

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
