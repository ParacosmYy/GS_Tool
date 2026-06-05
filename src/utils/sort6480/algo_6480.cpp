/**
 * @file algo_6480.cpp
 */
#include "sort6480/algo_6480.h"
QVector<double> algo_6480::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
