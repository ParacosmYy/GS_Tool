/**
 * @file algo_2916.cpp
 */
#include "geometry2916/algo_2916.h"
QVector<double> algo_2916::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
