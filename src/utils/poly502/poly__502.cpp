/**
 * @file poly__502.cpp
 * @brief poly__502 implementation
 */
#include "poly502/poly__502.h"
QVector<double> poly__502::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

