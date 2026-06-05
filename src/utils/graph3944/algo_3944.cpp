/**
 * @file algo_3944.cpp
 */
#include "graph3944/algo_3944.h"
QVector<double> algo_3944::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
