/**
 * @file matrix__325.cpp
 * @brief matrix__325 implementation
 */
#include "matrix325/matrix__325.h"
QVector<double> matrix__325::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

