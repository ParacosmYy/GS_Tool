/**
 * @file sort__340.cpp
 * @brief sort__340 implementation
 */
#include "sort340/sort__340.h"
QVector<double> sort__340::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

