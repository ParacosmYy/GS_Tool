/**
 * @file ProtocolSimulatorStats.cpp
 * @brief 协议响应模拟器 -- 统计重置方法
 *
 * 从 ProtocolSimulator.cpp 中拆分出的统计重置逻辑，职责:
 *   1. resetStatistics -- 归零所有统计计数器和累计辅助变量
 *
 * 注意: resetStatistics() 不影响模拟器运行状态(规则/待发送队列等)，
 * 仅清零累计统计。如需完全重置，请同时调用 stop()。
 */

#include "protocol/simulator/ProtocolSimulator.h"

// ============================================================================
// 统计重置
// ============================================================================

/**
 * @brief 重置所有统计计数器
 *
 * 归零所有 SimulationStats 字段和累计辅助变量(m_sumResponseTimeMs, m_responseTimeCount)。
 * 不影响当前模拟器运行状态(规则列表、待发送队列、运行标志等)。
 * 活跃规则计数会在重置后重新计算。
 */
void ProtocolSimulator::resetStatistics()
{
    m_stats.totalRequestsReceived = 0;
    m_stats.totalResponsesSent = 0;
    m_stats.totalMatches = 0;
    m_stats.totalMisses = 0;
    m_stats.totalDelayedResponses = 0;
    m_stats.totalBytesReceived = 0;
    m_stats.totalBytesSent = 0;
    m_stats.avgResponseTimeMs = 0.0;
    m_stats.peakPendingRequests = 0;

    /* 重新计算活跃规则数(不依赖累计值) */
    int activeCount = 0;
    for (const ResponseRule& rule : m_rules) {
        if (rule.enabled) {
            activeCount++;
        }
    }
    m_stats.activeRules = activeCount;

    /* 清零累计辅助变量 */
    m_sumResponseTimeMs = 0;
    m_responseTimeCount = 0;

    /* 重置每条规则的匹配计数 */
    for (ResponseRule& rule : m_rules) {
        rule.matchCount = 0;
    }
}
