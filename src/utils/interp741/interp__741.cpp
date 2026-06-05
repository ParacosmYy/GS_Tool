/**
 * @file interp__741.cpp
 * @brief interp__741 implementation
 */
#include "interp741/interp__741.h"
QVector<double> interp__741::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

