/**
 * @file poly__352.cpp
 * @brief poly__352 implementation
 */
#include "poly352/poly__352.h"
QVector<double> poly__352::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

