/**
 * @file poly__322.cpp
 * @brief poly__322 implementation
 */
#include "poly322/poly__322.h"
QVector<double> poly__322::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

