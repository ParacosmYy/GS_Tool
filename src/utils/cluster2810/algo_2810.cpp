/**
 * @file algo_2810.cpp
 */
#include "cluster2810/algo_2810.h"
QVector<double> algo_2810::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
