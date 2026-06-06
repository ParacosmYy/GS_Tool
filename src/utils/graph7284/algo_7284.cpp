/**
 * @file algo_7284.cpp
 */
#include "graph7284/algo_7284.h"
QVector<double> algo_7284::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
