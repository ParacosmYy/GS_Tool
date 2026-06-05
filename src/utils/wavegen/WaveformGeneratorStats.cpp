/**
 * @file WaveformGeneratorStats.cpp
 * @brief WaveformGenerator 统计接口实现
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/wavegen/WaveformGenerator.h"

WaveGen::Stats WaveformGenerator::stats() const
{
    return m_stats;
}

void WaveformGenerator::resetStatistics()
{
    m_stats = WaveGen::Stats{};
}
