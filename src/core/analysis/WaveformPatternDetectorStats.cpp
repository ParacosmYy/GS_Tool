/**
 * @file WaveformPatternDetectorStats.cpp
 * @brief 波形模式检测器 -- 统计重置实现
 */

#include "core/analysis/WaveformPatternDetector.h"

/** @brief 重置所有统计计数器为初始值
 *
 *  将Stats结构体所有字段归零，同时清空已检测模式列表。
 *  不影响缓冲区数据和在线统计状态(均值/方差)。
 */
void WaveformPatternDetector::resetStatistics()
{
    m_stats = Stats{};
    m_patterns.clear();
}
