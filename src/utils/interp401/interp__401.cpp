/**
 * @file interp__401.cpp
 * @brief interp__401 implementation
 */
#include "interp401/interp__401.h"
QVector<double> interp__401::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

