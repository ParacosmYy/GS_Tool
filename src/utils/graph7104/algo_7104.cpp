/**
 * @file algo_7104.cpp
 */
#include "graph7104/algo_7104.h"
QVector<double> algo_7104::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
