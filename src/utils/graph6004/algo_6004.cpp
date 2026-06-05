/**
 * @file algo_6004.cpp
 */
#include "graph6004/algo_6004.h"
QVector<double> algo_6004::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
