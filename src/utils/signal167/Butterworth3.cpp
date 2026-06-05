/**
 * @file Butterworth3.cpp
 * @brief Butterworth filter design and application implementation
 */
#include "signal167/Butterworth3.h"
#include <QElapsedTimer>

QVector<double> Butterworth3::compute(const QVector<double> &input)
{
    QElapsedTimer t; t.start();
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

