/**
 * @file sort__300.cpp
 * @brief sort__300 implementation
 */
#include "sort300/sort__300.h"
QVector<double> sort__300::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

