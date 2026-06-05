/**
 * @file interp__351.cpp
 * @brief interp__351 implementation
 */
#include "interp351/interp__351.h"
QVector<double> interp__351::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

