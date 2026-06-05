/**
 * @file poly__722.cpp
 * @brief poly__722 implementation
 */
#include "poly722/poly__722.h"
QVector<double> poly__722::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

