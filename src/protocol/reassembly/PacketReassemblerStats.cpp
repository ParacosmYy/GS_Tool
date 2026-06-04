/**
 * @file PacketReassemblerStats.cpp
 * @brief 数据包重组引擎 — 统计重置方法
 *
 * 从 PacketReassembler.cpp 中拆分出的统计重置逻辑，职责:
 *   1. resetStatistics — 归零所有统计计数器和累计辅助变量
 *
 * 注意: resetStatistics() 不影响重组状态(缓冲区/分片计数等)，
 * 仅清零累计统计。如需完全重置，请同时调用 reset()。
 */

#include "protocol/reassembly/PacketReassembler.h"

// ============================================================================
// 统计重置
// ============================================================================

/**
 * @brief 重置所有统计计数器
 *
 * 归零所有 Stats 字段和累计辅助变量(m_sumFragmentsPerPacket, m_sumAssemblyTimeMs)。
 * 不影响当前重组状态(缓冲区数据、分片计数、期望长度等)。
 * 如需完全重置，请先调用 resetStatistics() 再调用 reset()。
 */
void PacketReassembler::resetStatistics()
{
    m_stats.totalFragmentsReceived = 0;
    m_stats.totalPacketsReassembled = 0;
    m_stats.totalPartialPackets = 0;
    m_stats.totalTimeoutDrops = 0;
    m_stats.totalOversizeDrops = 0;
    m_stats.totalBytesProcessed = 0;
    m_stats.avgFragmentsPerPacket = 0.0;
    m_stats.avgAssemblyTimeMs = 0.0;
    m_stats.peakFragmentCount = 0;
    m_stats.currentBufferSize = m_buffer.size();

    m_sumFragmentsPerPacket = 0;
    m_sumAssemblyTimeMs = 0;
}
