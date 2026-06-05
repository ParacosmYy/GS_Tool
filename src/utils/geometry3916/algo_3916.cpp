/**
 * @file algo_3916.cpp
 */
#include "geometry3916/algo_3916.h"
QVector<double> algo_3916::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
