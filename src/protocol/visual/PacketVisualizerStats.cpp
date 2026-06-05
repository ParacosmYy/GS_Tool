/**
 * @file PacketVisualizerStats.cpp
 * @brief PacketVisualizer 统计计数器重置实现
 *
 * 从 PacketVisualizer.cpp 拆分而来，仅包含 resetStatistics() 方法。
 */

#include "protocol/visual/PacketVisualizer.h"

/**
 * @brief 重置所有统计计数器为零
 *
 * 将 totalPacketsVisualized / totalFieldsRendered / totalZoomOperations /
 * totalFieldClicks 四个累计计数器归零。
 */
void PacketVisualizer::resetStatistics()
{
    m_stats.totalPacketsVisualized = 0;
    m_stats.totalFieldsRendered = 0;
    m_stats.totalZoomOperations = 0;
    m_stats.totalFieldClicks = 0;
}
