/**
 * @file ExponentialMoving.cpp
 * @brief 指数移动平均/方差/协方差实现
 */

#include "utils/expmove/ExponentialMoving.h"

#include <QElapsedTimer>
#include <QtMath>

ExponentialMoving::ExponentialMoving(double alpha, QObject* parent)
    : QObject(parent), m_alpha(qBound(0.001, alpha, 1.0)),
      m_ema(0.0), m_emvar(0.0), m_emcov_x(0.0), m_emcov_y(0.0),
      m_emcov(0.0), m_initialized(false), m_timeSum(0.0) {}

void ExponentialMoving::setAlpha(double alpha)
{
    m_alpha = qBound(0.001, alpha, 1.0);
}

double ExponentialMoving::update(double value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_initialized) {
        m_ema = value;
        m_emvar = 0.0;
        m_initialized = true;
    } else {
        double diff = value - m_ema;
        m_ema = m_alpha * value + (1.0 - m_alpha) * m_ema;
        m_emvar = (1.0 - m_alpha) * (m_emvar + m_alpha * diff * diff);
    }

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalUpdates, 1ULL);

    emit valueUpdated(m_ema, m_emvar);
    return m_ema;
}

double ExponentialMoving::updateVariance(double value)
{
    update(value);
    return m_emvar;
}

QVector<double> ExponentialMoving::updateBatch(const QVector<double>& values)
{
    QVector<double> result;
    result.reserve(values.size());
    for (double v : values) {
        result.append(update(v));
    }
    return result;
}

double ExponentialMoving::updateCovariance(double x, double y)
{
    if (!m_initialized) {
        m_emcov_x = x;
        m_emcov_y = y;
        m_emcov = 0.0;
        m_initialized = true;
        return 0.0;
    }

    double dx = x - m_emcov_x;
    double dy = y - m_emcov_y;
    m_emcov_x = m_alpha * x + (1.0 - m_alpha) * m_emcov_x;
    m_emcov_y = m_alpha * y + (1.0 - m_alpha) * m_emcov_y;
    m_emcov = (1.0 - m_alpha) * (m_emcov + m_alpha * dx * dy);

    return m_emcov;
}

double ExponentialMoving::stdDev() const
{
    return qSqrt(qMax(m_emvar, 0.0));
}

void ExponentialMoving::reset()
{
    m_ema = 0.0;
    m_emvar = 0.0;
    m_emcov_x = 0.0;
    m_emcov_y = 0.0;
    m_emcov = 0.0;
    m_initialized = false;
    m_stats.totalResets++;
}

void ExponentialMoving::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
