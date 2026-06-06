/**
 * @file algo_7480.cpp
 */
#include "sort7480/algo_7480.h"
QVector<double> algo_7480::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
