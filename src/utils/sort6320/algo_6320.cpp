/**
 * @file algo_6320.cpp
 */
#include "sort6320/algo_6320.h"
QVector<double> algo_6320::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
