/**
 * @file cluster__330.cpp
 * @brief cluster__330 implementation
 */
#include "cluster330/cluster__330.h"
QVector<double> cluster__330::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

