/**
 * @file interp__371.cpp
 * @brief interp__371 implementation
 */
#include "interp371/interp__371.h"
QVector<double> interp__371::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

