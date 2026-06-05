/**
 * @file poly__572.cpp
 * @brief poly__572 implementation
 */
#include "poly572/poly__572.h"
QVector<double> poly__572::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

