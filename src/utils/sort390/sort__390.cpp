/**
 * @file sort__390.cpp
 * @brief sort__390 implementation
 */
#include "sort390/sort__390.h"
QVector<double> sort__390::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

