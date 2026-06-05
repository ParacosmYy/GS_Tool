/**
 * @file poly__522.cpp
 * @brief poly__522 implementation
 */
#include "poly522/poly__522.h"
QVector<double> poly__522::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

