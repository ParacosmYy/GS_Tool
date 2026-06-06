/**
 * @file algo_6964.cpp
 */
#include "graph6964/algo_6964.h"
QVector<double> algo_6964::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
