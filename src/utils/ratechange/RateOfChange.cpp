/**
 * @file RateOfChange.cpp
 * @brief 变化率检测器实现
 */

#include "utils/ratechange/RateOfChange.h"

#include <QtMath>

RateOfChange::RateOfChange(double threshold, QObject* parent)
    : QObject(parent), m_threshold(threshold), m_lastValue(0.0),
      m_lastRate(0.0), m_initialized(false),
      m_lastDirection(Direction::Stable) {}

double RateOfChange::update(double value, double dt)
{
    if (!m_initialized) {
        m_lastValue = value;
        m_initialized = true;
        m_stats.totalSamples++;
        return 0.0;
    }

    double rate = (dt > 1e-15) ? (value - m_lastValue) / dt : 0.0;
    m_lastValue = value;
    m_lastRate = rate;

    /* 更新统计 */
    m_stats.totalSamples++;
    m_stats.avgRate = m_stats.avgRate * (m_stats.totalSamples - 1) /
        m_stats.totalSamples + rate / m_stats.totalSamples;
    if (qAbs(rate) > m_stats.peakRate) m_stats.peakRate = qAbs(rate);

    /* 阈值告警 */
    if (qAbs(rate) > m_threshold) {
        m_stats.totalThresholdBreaches++;
        emit rateExceeded(rate, m_threshold);
    }

    /* 方向检测 */
    Direction newDir = Direction::Stable;
    if (rate > m_threshold * 0.1) newDir = Direction::Rising;
    else if (rate < -m_threshold * 0.1) newDir = Direction::Falling;

    if (newDir != m_lastDirection) {
        emit directionChanged(newDir);
        m_lastDirection = newDir;
    }

    return rate;
}

QVector<double> RateOfChange::updateBatch(const QVector<double>& values,
                                           double dt)
{
    QVector<double> rates;
    rates.reserve(values.size());
    for (double v : values) rates.append(update(v, dt));
    return rates;
}

void RateOfChange::setThreshold(double threshold)
{
    m_threshold = qMax(0.0, threshold);
}

RateOfChange::Direction RateOfChange::direction() const
{
    return m_lastDirection;
}

void RateOfChange::reset()
{
    m_initialized = false;
    m_lastValue = 0.0;
    m_lastRate = 0.0;
    m_lastDirection = Direction::Stable;
}

void RateOfChange::resetStatistics()
{
    m_stats = Stats{};
}
