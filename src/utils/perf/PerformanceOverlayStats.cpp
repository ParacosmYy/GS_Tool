/**
 * @file PerformanceOverlayStats.cpp
 * @brief 性能叠加显示控件 — FPS 统计与重置实现
 *
 * 从 PerformanceOverlay.cpp 拆分而来，包含 FPS 统计查询、
 * 阈值检查、性能摘要文本生成和统计重置方法。
 */

#include "utils/perf/PerformanceOverlay.h"

/** @brief 获取FPS统计数据(当前/最小/最大/平均/采样数) @return FpsStats结构体 */
FpsStats PerformanceOverlay::fpsStats() const
{
    FpsStats stats;
    stats.currentFps = m_currentFps;
    stats.minFps = m_minFps;
    stats.maxFps = m_maxFps;
    stats.sampleCount = m_fpsHistory.size();

    if (!m_fpsHistory.isEmpty()) {
        double sum = 0.0;
        for (double v : m_fpsHistory) {
            sum += v;
        }
        stats.avgFps = sum / m_fpsHistory.size();
    }
    return stats;
}

/** @brief 设置FPS警告阈值 @param threshold 警告阈值(FPS) */
void PerformanceOverlay::setFpsWarningThreshold(double threshold)
{
    m_fpsWarningThreshold = threshold;
}

/** @brief 查询当前是否低于FPS警告阈值 @return true=当前FPS低于阈值，false=正常或无数据 */
bool PerformanceOverlay::isBelowFpsThreshold() const
{
    return m_currentFps > 0 && m_currentFps < m_fpsWarningThreshold;
}

/** @brief 生成性能摘要文本(FPS极值+均值+内存) @return 格式化的性能摘要字符串 */
QString PerformanceOverlay::performanceSummary() const
{
    const FpsStats fps = fpsStats();
    const double mb = static_cast<double>(m_lastMemBytes) / (1024.0 * 1024.0);

    return tr("FPS: %1 (min: %2, max: %3, avg: %4) | MEM: %5 MB")
        .arg(fps.currentFps, 0, 'f', 1)
        .arg(fps.minFps, 0, 'f', 1)
        .arg(fps.maxFps, 0, 'f', 1)
        .arg(fps.avgFps, 0, 'f', 1)
        .arg(mb, 0, 'f', 1);
}

/** @brief 重置统计数据(清空FPS历史、极值归位、内存归零) */
void PerformanceOverlay::resetStats()
{
    m_fpsHistory.clear();
    m_minFps = 999.0;
    m_maxFps = 0.0;
    m_currentFps = 0.0;
    m_lastMemBytes = 0;
}

/** @brief 重置叠加层统计计数器(更新次数/低FPS警告/FPS采样/管线延迟采样/吞吐量采样) */
void PerformanceOverlay::resetOverlayStatistics()
{
    m_totalUpdates = 0;
    m_totalLowFpsWarnings = 0;
    m_totalFpsSamples = 0;
    m_totalPipelineLatencySamples = 0;
    m_totalThroughputSamples = 0;
}
