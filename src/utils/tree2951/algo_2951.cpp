/**
 * @file algo_2951.cpp
 */
#include "tree2951/algo_2951.h"
QVector<double> algo_2951::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
