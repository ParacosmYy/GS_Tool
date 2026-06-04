/**
 * @file ConnectionHealthMonitorCalc.cpp
 * @brief 连接健康监控器 — 质量评分计算逻辑
 *
 * 三维评分: 吞吐量(40%权重) + 错误率(35%权重) + 稳定性(25%权重)
 */

#include "core/health/ConnectionHealthMonitor.h"

/** @brief 综合计算质量评分(0-100)，由三维子评分加权得出 */
void ConnectionHealthMonitor::calculateQuality()
{
    double throughputPart = calcThroughputScore() * 0.40;
    double errorPart = calcErrorScore() * 0.35;
    double stabilityPart = calcStabilityScore() * 0.25;

    m_currentQuality = qBound(0.0, throughputPart + errorPart + stabilityPart, 100.0);

    /* 更新极值统计 */
    if (m_currentQuality < m_stats.lowestQuality) {
        m_stats.lowestQuality = m_currentQuality;
    }

    /* 更新最高错误率(需在非const上下文中执行) */
    qint64 elapsed = m_connectionTimer.elapsed();
    if (elapsed > 0 && m_stats.totalErrors > 0) {
        double errorsPerMinute = m_stats.totalErrors * 60000.0 / static_cast<double>(elapsed);
        if (errorsPerMinute > m_stats.highestErrorRate) {
            m_stats.highestErrorRate = errorsPerMinute;
        }
    }
}

/** @brief 计算吞吐量子评分(0-100) @return 基于当前吞吐量的分数 */
double ConnectionHealthMonitor::calcThroughputScore() const
{
    if (!m_connected) return 0.0;

    /* 非活跃连接(无数据传输)给予中等分数 */
    if (m_currentThroughput <= 0.0) return 70.0;

    /* 吞吐量评分曲线: 100B/s→40分, 1KB/s→60分, 10KB/s→80分, 100KB/s+→100分 */
    double logThroughput = qLn(m_currentThroughput + 1.0) / qLn(10.0);
    double score = 30.0 + logThroughput * 20.0;
    return qBound(0.0, score, 100.0);
}

/** @brief 计算错误率子评分(0-100) @return 基于错误率的分数 */
double ConnectionHealthMonitor::calcErrorScore() const
{
    if (m_stats.totalErrors == 0) return 100.0;

    /* 错误率 = 错误次数 / 运行分钟数 */
    qint64 elapsed = m_connectionTimer.elapsed();
    if (elapsed <= 0) return 100.0;

    double errorsPerMinute = m_stats.totalErrors * 60000.0 / static_cast<double>(elapsed);

    /* 评分曲线: 0错误/分→100分, 1错误/分→60分, 5+错误/分→0分 */
    if (errorsPerMinute < 0.5) return 100.0;
    if (errorsPerMinute < 1.0) return 80.0;
    if (errorsPerMinute < 2.0) return 60.0;
    if (errorsPerMinute < 5.0) return 30.0;
    return 0.0;
}

/** @brief 计算稳定性子评分(0-100) @return 基于重连次数和连续错误的分数 */
double ConnectionHealthMonitor::calcStabilityScore() const
{
    double score = 100.0;

    /* 每次重连扣10分 */
    score -= static_cast<double>(m_stats.totalReconnects) * 10.0;

    /* 连续错误扣分: 每个连续错误扣5分 */
    score -= static_cast<double>(m_consecutiveErrors) * 5.0;

    /* 断线时间占比扣分 */
    qint64 uptime = m_connected ? m_connectionTimer.elapsed() : 0;
    qint64 totalTime = uptime + m_totalDowntimeMs;
    if (totalTime > 0 && m_totalDowntimeMs > 0) {
        double downtimeRatio = static_cast<double>(m_totalDowntimeMs) / static_cast<double>(totalTime);
        score -= downtimeRatio * 50.0;
    }

    return qBound(0.0, score, 100.0);
}

/** @brief 重置所有统计计数器到初始值 */
void ConnectionHealthMonitor::resetStatistics()
{
    m_stats = Stats{};
    m_consecutiveErrors = 0;
    m_currentQuality = 100.0;
    m_currentThroughput = 0.0;
    m_totalDowntimeMs = 0;
    m_throughputHistory.clear();
}
