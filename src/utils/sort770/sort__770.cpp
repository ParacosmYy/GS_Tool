/**
 * @file sort__770.cpp
 * @brief sort__770 implementation
 */
#include "sort770/sort__770.h"
QVector<double> sort__770::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

