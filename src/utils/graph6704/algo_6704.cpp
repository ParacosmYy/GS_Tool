/**
 * @file algo_6704.cpp
 */
#include "graph6704/algo_6704.h"
QVector<double> algo_6704::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
