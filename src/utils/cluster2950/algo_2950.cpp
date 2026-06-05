/**
 * @file algo_2950.cpp
 */
#include "cluster2950/algo_2950.h"
QVector<double> algo_2950::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
