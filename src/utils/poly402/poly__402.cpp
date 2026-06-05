/**
 * @file poly__402.cpp
 * @brief poly__402 implementation
 */
#include "poly402/poly__402.h"
QVector<double> poly__402::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

