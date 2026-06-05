/**
 * @file sort__750.cpp
 * @brief sort__750 implementation
 */
#include "sort750/sort__750.h"
QVector<double> sort__750::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

