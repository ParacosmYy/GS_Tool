/**
 * @file BleConnectionStats.cpp
 * @brief BLE连接统计计数器重置方法
 *
 * 从 BleConnection.cpp 拆分而来，集中管理BLE连接的统计重置。
 */

#include "connection/ble/BleConnection.h"

/** @brief 重置所有BLE连接统计计数器 */
void BleConnection::resetStats()
{
    m_totalScans = 0;
    m_totalConnections = 0;
    m_totalDisconnections = 0;
    m_totalServicesDiscovered = 0;
    m_totalCharacteristicsRead = 0;
    m_totalWrites = 0;
    m_totalReads = 0;
    m_totalCharacteristicWrites = 0;
    m_totalCharacteristicReads = 0;
    m_totalNotifications = 0;
    m_totalBytesWritten = 0;
    m_totalBytesRead = 0;
    m_errorCount = 0;
}
