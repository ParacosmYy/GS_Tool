/**
 * @file algo_6264.cpp
 */
#include "graph6264/algo_6264.h"
QVector<double> algo_6264::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
