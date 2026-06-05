/**
 * @file poly__592.cpp
 * @brief poly__592 implementation
 */
#include "poly592/poly__592.h"
QVector<double> poly__592::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

