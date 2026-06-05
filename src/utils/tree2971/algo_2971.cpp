/**
 * @file algo_2971.cpp
 */
#include "tree2971/algo_2971.h"
QVector<double> algo_2971::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
