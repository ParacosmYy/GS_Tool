/**
 * @file algo_6304.cpp
 */
#include "graph6304/algo_6304.h"
QVector<double> algo_6304::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
