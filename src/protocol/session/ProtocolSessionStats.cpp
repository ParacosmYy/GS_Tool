/**
 * @file ProtocolSessionStats.cpp
 * @brief 协议会话管理器 -- 统计重置与计算方法
 *
 * 从 ProtocolSession.cpp 中拆分出的统计逻辑，职责:
 *   1. resetStatistics -- 归零所有统计计数器和响应时间列表
 *   2. updateStats -- 全量重算统计值(avg/min/max/p95/successRate/timeoutRate)
 *   3. computeP95 -- 排序法计算P95响应时间
 *
 * 注意: resetStatistics() 不影响会话运行状态(当前请求/历史等)，
 * 仅清零累计统计。如需完全重置，请同时调用 startSession()。
 */

#include "protocol/session/ProtocolSession.h"

#include <algorithm>

// ============================================================================
// 统计重置
// ============================================================================

/**
 * @brief 重置所有统计计数器
 *
 * 归零所有 SessionStats 字段和响应时间列表。
 * 不影响会话状态和历史记录。
 */
void ProtocolSession::resetStatistics()
{
    m_stats.totalRequests = 0;
    m_stats.totalResponses = 0;
    m_stats.totalTimeouts = 0;
    m_stats.totalRetries = 0;
    m_stats.totalErrors = 0;
    m_stats.avgResponseTimeMs = 0.0;
    m_stats.minResponseTimeMs = 0.0;
    m_stats.maxResponseTimeMs = 0.0;
    m_stats.p95ResponseTimeMs = 0.0;
    m_stats.successRate = 0.0;
    m_stats.timeoutRate = 0.0;
    m_responseTimes.clear();
}

// ============================================================================
// 统计计算
// ============================================================================

/**
 * @brief 重新计算全部统计值
 *
 * 遍历响应时间列表计算avg/min/max/p95。
 * 根据历史记录计算成功率和超时率。
 */
void ProtocolSession::updateStats()
{
    if (m_responseTimes.isEmpty()) {
        m_stats.avgResponseTimeMs = 0.0;
        m_stats.minResponseTimeMs = 0.0;
        m_stats.maxResponseTimeMs = 0.0;
        m_stats.p95ResponseTimeMs = 0.0;
    } else {
        double sum = 0.0;
        double minVal = m_responseTimes.first();
        double maxVal = m_responseTimes.first();
        for (double t : m_responseTimes) {
            sum += t;
            if (t < minVal) { minVal = t; }
            if (t > maxVal) { maxVal = t; }
        }
        m_stats.avgResponseTimeMs = sum / static_cast<double>(m_responseTimes.size());
        m_stats.minResponseTimeMs = minVal;
        m_stats.maxResponseTimeMs = maxVal;
        m_stats.p95ResponseTimeMs = computeP95();
    }

    /* 成功率和超时率 */
    if (m_stats.totalRequests > 0) {
        double total = static_cast<double>(m_stats.totalRequests);
        m_stats.successRate = static_cast<double>(m_stats.totalResponses) / total;
        m_stats.timeoutRate = static_cast<double>(m_stats.totalTimeouts) / total;
    } else {
        m_stats.successRate = 0.0;
        m_stats.timeoutRate = 0.0;
    }
}

/**
 * @brief 计算P95响应时间
 * @return P95值(ms)，数据不足时返回最大值
 *
 * 算法: 对响应时间列表排序后取第95百分位。
 */
double ProtocolSession::computeP95() const
{
    if (m_responseTimes.isEmpty()) {
        return 0.0;
    }
    if (m_responseTimes.size() == 1) {
        return m_responseTimes.first();
    }

    QList<double> sorted = m_responseTimes;
    std::sort(sorted.begin(), sorted.end());

    int idx = static_cast<int>(std::ceil(sorted.size() * 0.95)) - 1;
    if (idx < 0) { idx = 0; }
    if (idx >= sorted.size()) { idx = sorted.size() - 1; }
    return sorted[idx];
}

/** @brief 记录单次响应时间到列表 @param timeMs 响应时间(ms) */
void ProtocolSession::recordResponseTime(double timeMs)
{
    m_responseTimes.append(timeMs);
    if (m_responseTimes.size() > 1000) {
        m_responseTimes.removeFirst();
    }
    if (timeMs > m_stats.maxResponseTimeMs) {
        m_stats.maxResponseTimeMs = timeMs;
    }
}
