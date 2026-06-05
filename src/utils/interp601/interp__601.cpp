/**
 * @file interp__601.cpp
 * @brief interp__601 implementation
 */
#include "interp601/interp__601.h"
QVector<double> interp__601::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

