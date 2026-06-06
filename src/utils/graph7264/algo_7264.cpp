/**
 * @file algo_7264.cpp
 */
#include "graph7264/algo_7264.h"
QVector<double> algo_7264::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
