/**
 * @file WaveformGeneratorStats.cpp
 * @brief 波形发生器统计查询与重置方法实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 从 WaveformGenerator.cpp 拆分而来,包含 stats() 和 resetStatistics()。
 */

#include "utils/waveform/WaveformGenerator.h"

/** @brief 获取统计信息快照 @return 当前统计数据的只读副本 */
WaveformGenerator::Stats WaveformGenerator::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器(生成次数/采样点数/转换次数/字节数/错误次数归零) */
void WaveformGenerator::resetStatistics()
{
    m_stats.totalGenerations    = 0;
    m_stats.totalSamples        = 0;
    m_stats.totalConversions    = 0;
    m_stats.totalBytesConverted = 0;
    m_stats.errorCount          = 0;
}
