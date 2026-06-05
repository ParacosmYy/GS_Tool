/**
 * @file algo_3836.cpp
 */
#include "geometry3836/algo_3836.h"
QVector<double> algo_3836::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
