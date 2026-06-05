/**
 * @file cluster__360.cpp
 * @brief cluster__360 implementation
 */
#include "cluster360/cluster__360.h"
QVector<double> cluster__360::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

