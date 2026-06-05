/**
 * @file algo_2944.cpp
 */
#include "graph2944/algo_2944.h"
QVector<double> algo_2944::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
