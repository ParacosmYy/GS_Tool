/**
 * @file ProtocolSequencerStats.cpp
 * @brief 协议序列器引擎 -- 统计重置与查询方法
 *
 * 从 ProtocolSequencer.cpp 中拆分出的统计逻辑，职责:
 *   1. resetStatistics -- 归零所有累计统计计数器
 *   2. stats -- 返回统计结构体的常引用
 */

#include "protocol/sequencer/ProtocolSequencer.h"

// ============================================================================
// 统计查询
// ============================================================================

/** @brief 获取运行统计的只读引用 @return 统计结构体常引用 */
const SequencerStats& ProtocolSequencer::stats() const
{
    return m_stats;
}

// ============================================================================
// 统计重置
// ============================================================================

/**
 * @brief 重置所有统计计数器
 *
 * 归零全部 SequencerStats 字段。不影响当前执行状态和已加载序列。
 */
void ProtocolSequencer::resetStatistics()
{
    m_stats.sequencesExecuted = 0;
    m_stats.sequencesSucceeded = 0;
    m_stats.sequencesFailed = 0;
    m_stats.stepsCompleted = 0;
    m_stats.stepsFailed = 0;
    m_stats.bytesSent = 0;
    m_stats.bytesReceived = 0;
    m_stats.totalDurationMs = 0;
}
