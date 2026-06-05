/**
 * @file sort__420.cpp
 * @brief sort__420 implementation
 */
#include "sort420/sort__420.h"
QVector<double> sort__420::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

