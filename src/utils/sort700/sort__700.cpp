/**
 * @file sort__700.cpp
 * @brief sort__700 implementation
 */
#include "sort700/sort__700.h"
QVector<double> sort__700::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

