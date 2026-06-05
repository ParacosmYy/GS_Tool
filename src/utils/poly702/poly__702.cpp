/**
 * @file poly__702.cpp
 * @brief poly__702 implementation
 */
#include "poly702/poly__702.h"
QVector<double> poly__702::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

