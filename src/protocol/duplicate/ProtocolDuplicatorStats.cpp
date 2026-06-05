/**
 * @file ProtocolDuplicatorStats.cpp
 * @brief 协议流量复制器 -- 统计重置方法
 *
 * 从 ProtocolDuplicator.cpp 中拆分出的统计逻辑,职责:
 *   1. resetStatistics -- 归零所有统计计数器和延迟采样列表
 *
 * 注意: resetStatistics() 不影响录制库和回放状态,
 * 仅清零累计统计。如需完全重置,请另外清空录制库。
 */

#include "protocol/duplicate/ProtocolDuplicator.h"

// ============================================================================
// 统计重置
// ============================================================================

/**
 * @brief 重置所有统计计数器
 *
 * 归零所有 DuplicationStats 字段和延迟采样列表。
 * 不影响录制库内容和当前回放状态。
 */
void ProtocolDuplicator::resetStatistics()
{
    m_stats.totalRecorded = 0;
    m_stats.totalDuplicated = 0;
    m_stats.totalBytesRecorded = 0;
    m_stats.totalBytesDuplicated = 0;
    m_stats.duplicationCount = 0;
    m_stats.avgDelayMs = 0.0;
    m_delaySamples.clear();
}
