/**
 * @file interp__571.cpp
 * @brief interp__571 implementation
 */
#include "interp571/interp__571.h"
QVector<double> interp__571::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

