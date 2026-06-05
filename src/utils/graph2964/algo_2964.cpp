/**
 * @file algo_2964.cpp
 */
#include "graph2964/algo_2964.h"
QVector<double> algo_2964::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
