/**
 * @file algo_6164.cpp
 */
#include "graph6164/algo_6164.h"
QVector<double> algo_6164::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
