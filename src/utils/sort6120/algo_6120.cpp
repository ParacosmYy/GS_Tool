/**
 * @file algo_6120.cpp
 */
#include "sort6120/algo_6120.h"
QVector<double> algo_6120::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
