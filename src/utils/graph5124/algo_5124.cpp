/**
 * @file algo_5124.cpp
 */
#include "graph5124/algo_5124.h"
QVector<double> algo_5124::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
