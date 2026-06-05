/**
 * @file algo_2850.cpp
 */
#include "cluster2850/algo_2850.h"
QVector<double> algo_2850::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
