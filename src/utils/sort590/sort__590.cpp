/**
 * @file sort__590.cpp
 * @brief sort__590 implementation
 */
#include "sort590/sort__590.h"
QVector<double> sort__590::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

