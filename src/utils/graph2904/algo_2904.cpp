/**
 * @file algo_2904.cpp
 */
#include "graph2904/algo_2904.h"
QVector<double> algo_2904::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
