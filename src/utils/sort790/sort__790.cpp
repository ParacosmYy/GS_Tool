/**
 * @file sort__790.cpp
 * @brief sort__790 implementation
 */
#include "sort790/sort__790.h"
QVector<double> sort__790::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

