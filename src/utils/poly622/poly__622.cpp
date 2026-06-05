/**
 * @file poly__622.cpp
 * @brief poly__622 implementation
 */
#include "poly622/poly__622.h"
QVector<double> poly__622::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

