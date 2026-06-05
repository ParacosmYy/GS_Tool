/**
 * @file poly__372.cpp
 * @brief poly__372 implementation
 */
#include "poly372/poly__372.h"
QVector<double> poly__372::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

