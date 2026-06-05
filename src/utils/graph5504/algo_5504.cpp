/**
 * @file algo_5504.cpp
 */
#include "graph5504/algo_5504.h"
QVector<double> algo_5504::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
