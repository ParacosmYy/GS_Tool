/**
 * @file interp__471.cpp
 * @brief interp__471 implementation
 */
#include "interp471/interp__471.h"
QVector<double> interp__471::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

