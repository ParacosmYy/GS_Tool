/**
 * @file interp__671.cpp
 * @brief interp__671 implementation
 */
#include "interp671/interp__671.h"
QVector<double> interp__671::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

