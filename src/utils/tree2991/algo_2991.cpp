/**
 * @file algo_2991.cpp
 */
#include "tree2991/algo_2991.h"
QVector<double> algo_2991::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
