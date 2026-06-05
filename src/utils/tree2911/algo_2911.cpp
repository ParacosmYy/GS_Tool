/**
 * @file algo_2911.cpp
 */
#include "tree2911/algo_2911.h"
QVector<double> algo_2911::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
