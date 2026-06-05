/**
 * @file BleDeviceScannerStats.cpp
 * @brief BLE设备扫描器 — 统计getter与重置接口实现
 *
 * 从 BleDeviceScanner.cpp 拆分，包含所有统计查询方法和resetStatistics()。
 */

#include "connection/ble_scanner/BleDeviceScanner.h"

/** @brief 获取累计启动扫描次数 @return 扫描启动总次数 */
quint64 BleDeviceScanner::totalScanStarts() const { return m_totalScanStarts; }

/** @brief 获取累计扫描完成次数 @return 扫描周期完成总次数 */
quint64 BleDeviceScanner::totalScanCycles() const { return m_totalScanCycles; }

/** @brief 获取累计发现事件次数(含RSSI更新) @return 发现事件总次数 */
quint64 BleDeviceScanner::totalDiscoveryEvents() const { return m_totalDiscoveryEvents; }

/** @brief 获取累计去重设备地址总数 @return 不同设备地址的数量 */
quint64 BleDeviceScanner::uniqueDevicesSeen() const { return static_cast<quint64>(m_deviceCache.size()); }

/** @brief 获取累计扫描总时长(ms) */
quint64 BleDeviceScanner::totalScanDurationMs() const { return m_totalScanDurationMs; }

/** @brief 获取累计停止扫描次数 */
quint64 BleDeviceScanner::totalScanStops() const { return m_totalScanStops; }

/** @brief 获取最佳RSSI值(信号最强) @return dBm */
int BleDeviceScanner::bestRssi() const { return m_bestRssi; }

/** @brief 获取最差RSSI值(信号最弱) @return dBm */
int BleDeviceScanner::worstRssi() const { return m_worstRssi; }

/** @brief 获取平均RSSI值 @return dBm */
double BleDeviceScanner::averageRssi() const
{
    if (m_rssiSampleCount == 0) { return 0.0; }
    return static_cast<double>(m_rssiSum) / static_cast<double>(m_rssiSampleCount);
}

/** @brief 重置所有扫描器统计计数器，清空缓存和RSSI历史 */
void BleDeviceScanner::resetStatistics()
{
    m_totalScanStarts = 0;
    m_totalScanCycles = 0;
    m_totalDiscoveryEvents = 0;
    m_totalScanDurationMs = 0;
    m_totalScanStops = 0;
    m_bestRssi = 0;
    m_worstRssi = 0;
    m_rssiSum = 0;
    m_rssiSampleCount = 0;
    m_deviceCache.clear();
    m_rssiHistoryMap.clear();
}
