/**
 * @file poly__602.cpp
 * @brief poly__602 implementation
 */
#include "poly602/poly__602.h"
QVector<double> poly__602::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

