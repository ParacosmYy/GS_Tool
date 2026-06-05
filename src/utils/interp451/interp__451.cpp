/**
 * @file interp__451.cpp
 * @brief interp__451 implementation
 */
#include "interp451/interp__451.h"
QVector<double> interp__451::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

