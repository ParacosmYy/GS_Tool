/**
 * @file algo_5484.cpp
 */
#include "graph5484/algo_5484.h"
QVector<double> algo_5484::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
