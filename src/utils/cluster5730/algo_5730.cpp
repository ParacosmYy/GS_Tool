/**
 * @file algo_5730.cpp
 */
#include "cluster5730/algo_5730.h"
QVector<double> algo_5730::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
