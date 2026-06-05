/**
 * @file MinMaxScaler.cpp
 * @brief Min-Max特征缩放实现
 */

#include "utils/normalize2/MinMaxScaler.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
MinMaxScaler::MinMaxScaler(QObject* parent)
    : QObject(parent)
    , m_dataMin(0.0)
    , m_dataMax(1.0)
    , m_targetMin(0.0)
    , m_targetMax(1.0)
    , m_fitted(false)
    , m_timeSum(0.0)
{
}

/** @brief 拟合数据: 计算最小/最大值
 *  @param data 输入数据 */
void MinMaxScaler::fit(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_fitted = false;
        return;
    }

    m_dataMin = data[0];
    m_dataMax = data[0];

    for (int i = 1; i < data.size(); ++i) {
        if (data[i] < m_dataMin) m_dataMin = data[i];
        if (data[i] > m_dataMax) m_dataMax = data[i];
    }

    /* 防止除以零 */
    if (qAbs(m_dataMax - m_dataMin) < 1e-300) {
        m_dataMax = m_dataMin + 1.0;
    }

    m_fitted = true;

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalTransforms;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit fitCompleted(m_dataMin, m_dataMax);
}

/** @brief 变换数据: 缩放到目标范围
 *  @param data 输入数据
 *  @return 缩放后的数据 */
QVector<double> MinMaxScaler::transform(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    if (!m_fitted || data.isEmpty()) {
        return result;
    }

    double dataRange = m_dataMax - m_dataMin;
    double targetRange = m_targetMax - m_targetMin;

    result.resize(data.size());
    for (int i = 0; i < data.size(); ++i) {
        double normalized = (data[i] - m_dataMin) / dataRange;
        result[i] = m_targetMin + normalized * targetRange;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalTransforms;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(data.size());
    return result;
}

/** @brief 逆变换: 从目标范围还原
 *  @param data 缩放后的数据
 *  @return 还原后的数据 */
QVector<double> MinMaxScaler::inverseTransform(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    if (!m_fitted || data.isEmpty()) {
        return result;
    }

    double dataRange = m_dataMax - m_dataMin;
    double targetRange = m_targetMax - m_targetMin;

    result.resize(data.size());
    for (int i = 0; i < data.size(); ++i) {
        double denormalized = (data[i] - m_targetMin) / targetRange;
        result[i] = m_dataMin + denormalized * dataRange;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalTransforms;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(data.size());
    return result;
}

/** @brief 设置目标范围 @param min 最小值 @param max 最大值 */
void MinMaxScaler::setTargetRange(double min, double max)
{
    m_targetMin = qMin(min, max);
    m_targetMax = qMax(min, max);
    if (qAbs(m_targetMax - m_targetMin) < 1e-300) {
        m_targetMax = m_targetMin + 1.0;
    }
}

/** @brief 重置统计 */
void MinMaxScaler::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
