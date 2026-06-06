/**
 * @file algo_7304.cpp
 */
#include "graph7304/algo_7304.h"
QVector<double> algo_7304::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
