/**
 * @file algo_5230.cpp
 */
#include "cluster5230/algo_5230.h"
QVector<double> algo_5230::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
