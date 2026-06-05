/**
 * @file algo_2871.cpp
 */
#include "tree2871/algo_2871.h"
QVector<double> algo_2871::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
