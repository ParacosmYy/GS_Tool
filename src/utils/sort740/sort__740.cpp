/**
 * @file sort__740.cpp
 * @brief sort__740 implementation
 */
#include "sort740/sort__740.h"
QVector<double> sort__740::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

