/**
 * @file poly__652.cpp
 * @brief poly__652 implementation
 */
#include "poly652/poly__652.h"
QVector<double> poly__652::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

