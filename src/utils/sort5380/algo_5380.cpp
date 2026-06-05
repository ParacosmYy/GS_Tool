/**
 * @file algo_5380.cpp
 */
#include "sort5380/algo_5380.h"
QVector<double> algo_5380::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
