/**
 * @file algo_2891.cpp
 */
#include "tree2891/algo_2891.h"
QVector<double> algo_2891::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
