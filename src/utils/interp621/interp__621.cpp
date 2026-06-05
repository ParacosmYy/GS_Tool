/**
 * @file interp__621.cpp
 * @brief interp__621 implementation
 */
#include "interp621/interp__621.h"
QVector<double> interp__621::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

