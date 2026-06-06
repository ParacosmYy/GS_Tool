/**
 * @file algo_7564.cpp
 */
#include "graph7564/algo_7564.h"
QVector<double> algo_7564::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
