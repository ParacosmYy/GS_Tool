/**
 * @file algo_6560.cpp
 */
#include "sort6560/algo_6560.h"
QVector<double> algo_6560::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
