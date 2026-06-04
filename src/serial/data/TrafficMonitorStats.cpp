/**
 * @file TrafficMonitorStats.cpp
 * @brief 流量监控器 — 统计计数器查询与重置实现
 *
 * 从 TrafficMonitor.cpp 拆分而来，包含采样/带宽/峰值等
 * 统计 getter 和 resetTrafficStatistics 方法。
 */

#include "serial/data/TrafficMonitor.h"

/** @brief 获取采样总次数 @return 累计采样点数 */
quint64 TrafficMonitor::totalSamples() const
{
    return m_totalSamples;
}

/** @brief 获取历史最高带宽(RX+TX之和) @return 峰值带宽(bytes/s) */
double TrafficMonitor::peakBandwidth() const
{
    return m_peakRxRate + m_peakTxRate;
}

/** @brief 获取监控的字节总数(RX+TX) @return 累计字节数 */
quint64 TrafficMonitor::totalBytesMonitored() const
{
    return static_cast<quint64>(m_totalRxBytes) + static_cast<quint64>(m_totalTxBytes);
}

/** @brief 获取累计接收字节记录次数 @return recordRxBytes调用次数 */
quint64 TrafficMonitor::totalBytesIn() const
{
    return m_totalBytesIn;
}

/** @brief 获取累计发送字节记录次数 @return recordTxBytes调用次数 */
quint64 TrafficMonitor::totalBytesOut() const
{
    return m_totalBytesOut;
}

/** @brief 获取累计速率采样次数 @return 历史点追加次数 */
quint64 TrafficMonitor::totalRateSamples() const
{
    return m_totalRateSamples;
}

/** @brief 获取累计峰值速率刷新事件次数 @return 峰值被刷新的总次数 */
quint64 TrafficMonitor::totalPeakRateExceededEvents() const
{
    return m_totalPeakRateExceededEvents;
}

/** @brief 重置流量监控统计计数器(仅影响计数器，不影响速率计算) */
void TrafficMonitor::resetTrafficStatistics()
{
    m_totalSamples = 0;
    m_totalBytesIn = 0;
    m_totalBytesOut = 0;
    m_totalRateSamples = 0;
    m_totalPeakRateExceededEvents = 0;
}
