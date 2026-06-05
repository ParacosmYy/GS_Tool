/**
 * @file PeakHoldDetector.cpp
 * @brief 峰值保持检测器实现 — 间隔极值跟踪
 */

#include "utils/peakhold/PeakHoldDetector.h"

#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
PeakHoldDetector::PeakHoldDetector(QObject* parent)
    : QObject(parent)
    , m_peak(0.0)
    , m_trough(0.0)
    , m_holdTime(0)
    , m_peakAge(0)
    , m_troughAge(0)
    , m_initialized(false)
{
}

/** @brief 更新采样值 @param value 采样值 */
void PeakHoldDetector::update(double value)
{
    ++m_stats.totalUpdates;

    if (!m_initialized) {
        /* 首次初始化 */
        m_peak = value;
        m_trough = value;
        m_initialized = true;
        m_peakAge = 0;
        m_troughAge = 0;
        ++m_stats.totalPeakChanges;
        emit peakChanged(m_peak);
        emit troughChanged(m_trough);
        return;
    }

    bool peakUpdated = false;
    bool troughUpdated = false;

    /* 更新峰值 */
    if (value > m_peak) {
        m_peak = value;
        m_peakAge = 0;
        peakUpdated = true;
    } else {
        ++m_peakAge;
        /* 保持时间到期，重新扫描窗口 */
        if (m_holdTime > 0 && m_peakAge >= m_holdTime) {
            m_peak = value;
            m_peakAge = 0;
            peakUpdated = true;
        }
    }

    /* 更新谷值 */
    if (value < m_trough) {
        m_trough = value;
        m_troughAge = 0;
        troughUpdated = true;
    } else {
        ++m_troughAge;
        /* 保持时间到期，重新扫描窗口 */
        if (m_holdTime > 0 && m_troughAge >= m_holdTime) {
            m_trough = value;
            m_troughAge = 0;
            troughUpdated = true;
        }
    }

    /* 发射变化信号 */
    if (peakUpdated) {
        ++m_stats.totalPeakChanges;
        emit peakChanged(m_peak);
    }
    if (troughUpdated) {
        ++m_stats.totalPeakChanges;
        emit troughChanged(m_trough);
    }
}

/** @brief 获取当前峰值(最大值) @return 当前峰值 */
double PeakHoldDetector::currentPeak() const
{
    return m_peak;
}

/** @brief 获取当前谷值(最小值) @return 当前谷值 */
double PeakHoldDetector::currentTrough() const
{
    return m_trough;
}

/** @brief 设置保持时间(采样数) @param samples 保持样本数 */
void PeakHoldDetector::setHoldTime(int samples)
{
    m_holdTime = qMax(0, samples);
}

/** @brief 重置检测器状态 */
void PeakHoldDetector::reset()
{
    m_peak = 0.0;
    m_trough = 0.0;
    m_peakAge = 0;
    m_troughAge = 0;
    m_initialized = false;
}

/** @brief 重置统计 */
void PeakHoldDetector::resetStatistics()
{
    m_stats = Stats{};
}
