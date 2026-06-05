/**
 * @file sort__490.cpp
 * @brief sort__490 implementation
 */
#include "sort490/sort__490.h"
QVector<double> sort__490::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

