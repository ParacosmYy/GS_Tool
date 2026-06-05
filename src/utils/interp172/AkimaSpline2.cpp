/**
 * @file AkimaSpline2.cpp
 * @brief Akima spline for visually smooth interpolation implementation
 */
#include "interp172/AkimaSpline2.h"
#include <QElapsedTimer>

QVector<double> AkimaSpline2::compute(const QVector<double> &input)
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

