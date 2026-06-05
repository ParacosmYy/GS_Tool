/**
 * @file CubicSpline5.cpp
 * @brief Cubic spline interpolation for smooth curve fitting implementation
 */
#include "interp168/CubicSpline5.h"
#include <QElapsedTimer>

QVector<double> CubicSpline5::compute(const QVector<double> &input)
{
    QElapsedTimer t;
    t.start();
    m_stats.calls++;

    if (input.isEmpty()) {
        m_stats.errors++;
        return {};
    }

    QVector<double> result = input;
    m_stats.itemsProcessed += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

