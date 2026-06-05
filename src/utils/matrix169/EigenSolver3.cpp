/**
 * @file EigenSolver3.cpp
 * @brief Eigenvalue decomposition solver implementation
 */
#include "matrix169/EigenSolver3.h"
#include <QElapsedTimer>

QVector<double> EigenSolver3::compute(const QVector<double> &input)
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

