/**
 * @file algo_6064.cpp
 */
#include "graph6064/algo_6064.h"
QVector<double> algo_6064::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
