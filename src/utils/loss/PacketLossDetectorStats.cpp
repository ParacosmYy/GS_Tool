/**
 * @file PacketLossDetectorStats.cpp
 * @brief 丢包检测器 -- 统计查询与重置
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供 stats() 和 resetStatistics() 方法，
 * 运行时丢包检测见 @see PacketLossDetector.cpp。
 */

#include "utils/loss/PacketLossDetector.h"

/**
 * @brief 获取全局统计快照
 *
 * 返回包含丢包数、乱序数、重复数、丢包率等
 * 完整统计信息的结构体副本。
 *
 * @return Stats 结构体快照
 */
PacketLossDetector::Stats PacketLossDetector::stats() const
{
    Stats s;
    s.totalPacketsReceived   = m_stats.totalPacketsReceived;
    s.totalGapsDetected      = m_stats.totalGapsDetected;
    s.totalPacketsLost       = m_stats.totalPacketsLost;
    s.totalOutOfOrder        = m_stats.totalOutOfOrder;
    s.totalDuplicates        = m_stats.totalDuplicates;
    s.maxConsecutiveLoss     = m_stats.maxConsecutiveLoss;
    s.currentConsecutiveLoss = m_stats.currentConsecutiveLoss;
    s.lossRate               = m_stats.lossRate;
    s.sequenceRollovers      = m_stats.sequenceRollovers;
    return s;
}

/**
 * @brief 重置所有统计计数器
 *
 * 清零接收包数、丢包数、乱序数、重复数、连续丢包等计数器。
 * 不影响期望序列号、Gap 历史和序列号位宽配置。
 */
void PacketLossDetector::resetStatistics()
{
    m_stats.totalPacketsReceived   = 0;
    m_stats.totalGapsDetected      = 0;
    m_stats.totalPacketsLost       = 0;
    m_stats.totalOutOfOrder        = 0;
    m_stats.totalDuplicates        = 0;
    m_stats.maxConsecutiveLoss     = 0;
    m_stats.currentConsecutiveLoss = 0;
    m_stats.lossRate               = 0.0;
    m_stats.sequenceRollovers      = 0;
}
