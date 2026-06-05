/**
 * @file algo_5304.cpp
 */
#include "graph5304/algo_5304.h"
QVector<double> algo_5304::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
