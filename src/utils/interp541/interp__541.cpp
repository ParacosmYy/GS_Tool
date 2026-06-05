/**
 * @file interp__541.cpp
 * @brief interp__541 implementation
 */
#include "interp541/interp__541.h"
QVector<double> interp__541::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

