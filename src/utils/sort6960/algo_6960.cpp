/**
 * @file algo_6960.cpp
 */
#include "sort6960/algo_6960.h"
QVector<double> algo_6960::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
