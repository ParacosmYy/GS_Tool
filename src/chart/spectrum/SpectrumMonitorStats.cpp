/**
 * @file SpectrumMonitorStats.cpp
 * @brief 频谱监控引擎统计接口实现 — 计数器查询与重置
 *
 * 从 SpectrumMonitor.cpp 拆分，仅包含统计 getter 和 resetStatistics()。
 */

#include "chart/spectrum/SpectrumMonitor.h"

// ============================================================
// 统计 getter
// ============================================================

/** @brief 获取累计处理的采样点数 */
quint64 SpectrumMonitor::totalSamplesProcessed() const
{
    return m_totalSamplesProcessed;
}

/** @brief 获取累计执行的FFT次数 */
quint64 SpectrumMonitor::totalFftExecuted() const
{
    return m_totalFftExecuted;
}

/** @brief 获取累计处理的帧数 */
quint64 SpectrumMonitor::totalFramesProcessed() const
{
    return m_totalFramesProcessed;
}

/** @brief 获取峰值频率(Hz) */
double SpectrumMonitor::peakFrequency() const
{
    return m_peakFrequency;
}

/** @brief 获取峰值幅度(dBFS) */
double SpectrumMonitor::peakMagnitude() const
{
    return m_peakMagnitude;
}

// ============================================================
// 重置
// ============================================================

/** @brief 重置所有统计计数器为初始值 */
void SpectrumMonitor::resetStatistics()
{
    m_totalSamplesProcessed = 0;
    m_totalFftExecuted      = 0;
    m_totalFramesProcessed  = 0;
    m_peakFrequency         = 0.0;
    m_peakMagnitude         = -120.0;
}
