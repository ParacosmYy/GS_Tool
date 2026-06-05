/**
 * @file algo_3266.cpp
 */
#include "signal3266/algo_3266.h"
QVector<double> algo_3266::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
