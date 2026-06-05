/**
 * @file algo_5204.cpp
 */
#include "graph5204/algo_5204.h"
QVector<double> algo_5204::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
