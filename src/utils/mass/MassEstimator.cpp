/**
 * @file MassEstimator.cpp
 * @brief 加权均值/方差估计器实现
 */

#include "utils/mass/MassEstimator.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
MassEstimator::MassEstimator(QObject* parent)
    : QObject(parent)
    , m_weightSum(0.0)
    , m_weightedValueSum(0.0)
    , m_weightedSqSum(0.0)
    , m_lastMean(0.0)
    , m_lastVariance(0.0)
    , m_count(0)
    , m_timeSum(0.0)
{
}

/** @brief 更新估计器 @param value 样本值 @param weight 权重 */
void MassEstimator::update(double value, double weight)
{
    QElapsedTimer timer;
    timer.start();

    if (weight <= 0.0) weight = 1e-10; /* 防止除零 */

    m_weightSum += weight;
    m_weightedValueSum += weight * value;
    m_weightedSqSum += weight * value * value;
    m_count++;

    /* 更新缓存均值和方差 */
    m_lastMean = m_weightedValueSum / m_weightSum;
    if (m_count > 1 && m_weightSum > 0.0) {
        double meanSq = m_lastMean * m_lastMean;
        double avgSq = m_weightedSqSum / m_weightSum;
        m_lastVariance = avgSq - meanSq;
        if (m_lastVariance < 0.0) m_lastVariance = 0.0;
    } else {
        m_lastVariance = 0.0;
    }

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalUpdates > 0)
        ? m_timeSum / m_stats.totalUpdates : 0.0;

    emit estimateUpdated(m_lastMean, m_lastVariance);
}

/** @brief 获取加权均值 @return 均值 */
double MassEstimator::mean() const
{
    if (m_weightSum <= 0.0) return 0.0;
    return m_weightedValueSum / m_weightSum;
}

/** @brief 获取加权方差 @return 方差 */
double MassEstimator::variance() const
{
    return m_lastVariance;
}

/** @brief 获取加权标准差 @return 标准差 */
double MassEstimator::stdDev() const
{
    return std::sqrt(std::max(0.0, m_lastVariance));
}

/** @brief 获取样本计数 @return 计数 */
quint64 MassEstimator::count() const
{
    return m_count;
}

/** @brief 重置估计器 */
void MassEstimator::reset()
{
    m_weightSum = 0.0;
    m_weightedValueSum = 0.0;
    m_weightedSqSum = 0.0;
    m_lastMean = 0.0;
    m_lastVariance = 0.0;
    m_count = 0;
}

/** @brief 重置统计 */
void MassEstimator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
