/**
 * @file interp__651.cpp
 * @brief interp__651 implementation
 */
#include "interp651/interp__651.h"
QVector<double> interp__651::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

