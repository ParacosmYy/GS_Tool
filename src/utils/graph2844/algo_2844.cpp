/**
 * @file algo_2844.cpp
 */
#include "graph2844/algo_2844.h"
QVector<double> algo_2844::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
