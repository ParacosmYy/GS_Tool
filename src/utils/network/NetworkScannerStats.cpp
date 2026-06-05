/**
 * @file NetworkScannerStats.cpp
 * @brief NetworkScanner 统计接口实现
 */

#include "utils/network/NetworkScanner.h"

// ============================================================================
// 统计只读接口
// ============================================================================

/** @brief 获取累计扫描次数 @return 计数 */
quint64 NetworkScanner::totalScans() const { return m_totalScans; }

/** @brief 获取累计发现设备数 @return 计数 */
quint64 NetworkScanner::totalDevicesFound() const { return m_totalDevicesFound; }

/** @brief 获取累计扫描 IP 数 @return 计数 */
quint64 NetworkScanner::totalIpsScanned() const { return m_totalIpsScanned; }

/** @brief 获取累计扫描端口数 @return 计数 */
quint64 NetworkScanner::totalPortsScanned() const { return m_totalPortsScanned; }

/** @brief 获取累计超时次数 @return 计数 */
quint64 NetworkScanner::totalTimeouts() const { return m_totalTimeouts; }

/** @brief 获取累计错误次数 @return 计数 */
quint64 NetworkScanner::totalErrors() const { return m_totalErrors; }

/** @brief 重置所有扫描统计计数器(扫描次数/设备数/IP数/端口数/超时/错误) */
void NetworkScanner::resetStatistics() {
    m_totalScans        = 0;
    m_totalDevicesFound = 0;
    m_totalIpsScanned   = 0;
    m_totalPortsScanned = 0;
    m_totalTimeouts     = 0;
    m_totalErrors       = 0;
}
