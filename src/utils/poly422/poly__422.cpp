/**
 * @file poly__422.cpp
 * @brief poly__422 implementation
 */
#include "poly422/poly__422.h"
QVector<double> poly__422::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

