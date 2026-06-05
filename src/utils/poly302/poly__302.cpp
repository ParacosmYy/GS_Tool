/**
 * @file poly__302.cpp
 * @brief poly__302 implementation
 */
#include "poly302/poly__302.h"
QVector<double> poly__302::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

