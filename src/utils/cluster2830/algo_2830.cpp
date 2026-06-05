/**
 * @file algo_2830.cpp
 */
#include "cluster2830/algo_2830.h"
QVector<double> algo_2830::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
