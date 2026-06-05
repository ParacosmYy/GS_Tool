/**
 * @file algo_5004.cpp
 */
#include "graph5004/algo_5004.h"
QVector<double> algo_5004::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
