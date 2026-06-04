/**
 * @file SignalLineMonitorStats.cpp
 * @brief 信号线监控器 — 统计计数器查询与重置实现
 *
 * 从 SignalLineMonitor.cpp 拆分而来，包含信号线变化/轮询/
 * 空闲/错误等统计 getter 和 resetStatistics 方法。
 */

#include "serial/signals/SignalLineMonitor.h"

/** @brief 获取信号线变化次数 @return 状态变化的总次数 */
quint64 SignalLineMonitor::changeCount() const
{
    return m_changeCount;
}

/** @brief 获取轮询已运行时长 @return 运行时长（秒） */
qint64 SignalLineMonitor::pollingDuration() const
{
    if (!isPolling()) return 0;
    return m_durationTimer.elapsed() / 1000;
}

/** @brief 获取累计轮询次数 @return 轮询总次数 */
quint64 SignalLineMonitor::totalPolls() const
{
    return m_totalPolls;
}

/** @brief 获取累计无变化轮询次数 @return 无变化轮询次数 */
quint64 SignalLineMonitor::totalIdlePolls() const
{
    return m_totalIdlePolls;
}

/** @brief 获取累计信号线变化事件次数 @return 每条线变化+1的累计次数 */
quint64 SignalLineMonitor::totalSignalChanges() const
{
    return m_totalSignalChanges;
}

/** @brief 获取累计被监控的信号线总条数 @return 轮询次数×线数的累计 */
quint64 SignalLineMonitor::totalLineMonitored() const
{
    return m_totalLineMonitored;
}

/** @brief 获取累计错误事件次数 @return 轮询失败/连接异常等错误累计 */
quint64 SignalLineMonitor::totalErrorEvents() const
{
    return m_totalErrorEvents;
}

/** @brief 重置统计计数(changeCount/totalPolls/totalIdlePolls/totalSignalChanges/totalLineMonitored/totalErrorEvents归零，重启计时器) */
void SignalLineMonitor::resetStatistics()
{
    m_changeCount = 0;
    m_totalPolls = 0;
    m_totalIdlePolls = 0;
    m_totalSignalChanges = 0;
    m_totalLineMonitored = 0;
    m_totalErrorEvents = 0;
    m_totalDtrChanges = 0;
    m_totalRtsChanges = 0;
    m_totalCtsChanges = 0;
    m_totalDsrChanges = 0;
    m_totalDcdChanges = 0;
    m_totalRiChanges = 0;
    m_peakChangeRate = 0;
    m_lastSecChanges = 0;
    m_lastPeakRateSec = 0;
    m_durationTimer.restart();
}
