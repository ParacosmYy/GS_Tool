/**
 * @file FrequencyCounterWidgetStats.cpp
 * @brief 频率计数器控件 — 统计查询与重置
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供 resetStatistics() 方法，清零所有内部计数器。
 */

#include "utils/frequency/FrequencyCounterWidget.h"

/**
 * @brief 重置所有计数器和统计信息，恢复初始状态
 *
 * 清零窗口计数器、历史峰值、累计总数和全局统计。
 * 不影响当前窗口大小和定时器运行状态。
 */
void FrequencyCounterWidget::resetStatistics()
{
    // 窗口计数器
    m_windowPackets = 0;
    m_windowBytes   = 0;
    m_windowPatterns.clear();

    // 历史统计
    m_lastPacketFreq    = 0.0;
    m_lastByteRate      = 0.0;
    m_peakPacketFreq    = 0.0;
    m_peakByteRateLocal = 0.0;
    m_totalPackets      = 0;
    m_totalBytes        = 0;
    m_freqSum           = 0.0;
    m_windowCount       = 0;

    // 全局统计
    m_stats = Stats{};
}
