/**
 * @file ComplementaryFilter.cpp
 * @brief 互补滤波器实现 — 多传感器融合
 */

#include "utils/complement/ComplementaryFilter.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ComplementaryFilter::ComplementaryFilter(QObject* parent)
    : QObject(parent)
    , m_alpha(0.98)
    , m_state(0.0)
    , m_timeSum(0.0)
{
}

/** @brief 设置融合系数 @param alpha 融合系数[0,1] */
void ComplementaryFilter::setAlpha(double alpha)
{
    m_alpha = qBound(0.0, alpha, 1.0);
}

/** @brief 单步更新
 *  @param sensor1 高频传感器值
 *  @param sensor2 低频传感器值
 *  @return 融合后状态 */
double ComplementaryFilter::update(double sensor1, double sensor2)
{
    QElapsedTimer timer;
    timer.start();

    /* 互补滤波: y = alpha * (y_prev + sensor1) + (1-alpha) * sensor2 */
    m_state = m_alpha * (m_state + sensor1) + (1.0 - m_alpha) * sensor2;

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalUpdates;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalUpdates);

    emit updateCompleted(m_state);
    return m_state;
}

/** @brief 向量形式更新
 *  @param sensor1 高频传感器向量
 *  @param sensor2 低频传感器向量
 *  @return 融合后状态向量 */
QVector<double> ComplementaryFilter::updateVector(
    const QVector<double>& sensor1,
    const QVector<double>& sensor2)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(sensor1.size(), sensor2.size());
    m_stateVector.resize(n);

    for (int i = 0; i < n; ++i) {
        m_stateVector[i] = m_alpha * (m_stateVector[i] + sensor1[i])
                         + (1.0 - m_alpha) * sensor2[i];
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalUpdates;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalUpdates);

    emit updateCompleted(m_stateVector.isEmpty() ? 0.0 : m_stateVector[0]);
    return m_stateVector;
}

/** @brief 重置滤波器状态 @param initialState 初始状态 */
void ComplementaryFilter::reset(double initialState)
{
    m_state = initialState;
    m_stateVector.fill(0.0);
}

/** @brief 重置统计 */
void ComplementaryFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
