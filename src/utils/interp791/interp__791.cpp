/**
 * @file interp__791.cpp
 * @brief interp__791 implementation
 */
#include "interp791/interp__791.h"
QVector<double> interp__791::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

