/**
 * @file algo_3404.cpp
 */
#include "graph3404/algo_3404.h"
QVector<double> algo_3404::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
