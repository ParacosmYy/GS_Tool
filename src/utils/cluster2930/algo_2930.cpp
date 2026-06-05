/**
 * @file algo_2930.cpp
 */
#include "cluster2930/algo_2930.h"
QVector<double> algo_2930::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
