/**
 * @file algo_5024.cpp
 */
#include "graph5024/algo_5024.h"
QVector<double> algo_5024::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
