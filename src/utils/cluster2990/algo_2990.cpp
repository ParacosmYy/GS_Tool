/**
 * @file algo_2990.cpp
 */
#include "cluster2990/algo_2990.h"
QVector<double> algo_2990::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
