/**
 * @file cluster__380.cpp
 * @brief cluster__380 implementation
 */
#include "cluster380/cluster__380.h"
QVector<double> cluster__380::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

