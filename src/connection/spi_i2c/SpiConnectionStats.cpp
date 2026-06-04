/**
 * @file SpiConnectionStats.cpp
 * @brief SPI连接统计查询和重置方法实现
 *
 * 从 SpiConnection.cpp 拆分而来，包含SPI模式传输统计、
 * transferByMode查询和resetStats方法。
 */

#include "connection/spi_i2c/SpiConnection.h"

/** @brief 获取指定SPI模式的传输次数 @param mode SPI模式(0-3) @return 该模式累计传输次数 */
quint64 SpiConnection::transferByMode(int mode) const
{
    if (mode < 0 || mode >= 4) return 0;
    return m_transferByMode[mode];
}

/** @brief 重置所有SPI统计计数器(传输次数/字节数/错误计数/传输错误/CS切换/模式统计/模式变更/频率变更/打开次数) */
void SpiConnection::resetStats()
{
    m_totalTransfers = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
    m_totalTransferErrors = 0;
    m_totalCsToggles = 0;
    for (int i = 0; i < 4; ++i) {
        m_transferByMode[i] = 0;
    }
    m_totalModeChanges = 0;
    m_totalFrequencyChanges = 0;
    m_totalOpenAttempts = 0;
}
