/**
 * @file algo_5770.cpp
 */
#include "cluster5770/algo_5770.h"
QVector<double> algo_5770::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
