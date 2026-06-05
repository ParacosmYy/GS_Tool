/**
 * @file sort__650.cpp
 * @brief sort__650 implementation
 */
#include "sort650/sort__650.h"
QVector<double> sort__650::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

