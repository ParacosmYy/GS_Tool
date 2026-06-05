/**
 * @file algo_3530.cpp
 */
#include "cluster3530/algo_3530.h"
QVector<double> algo_3530::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
