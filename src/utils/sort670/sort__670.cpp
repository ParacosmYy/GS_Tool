/**
 * @file sort__670.cpp
 * @brief sort__670 implementation
 */
#include "sort670/sort__670.h"
QVector<double> sort__670::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

