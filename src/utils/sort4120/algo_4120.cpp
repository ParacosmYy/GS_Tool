/**
 * @file algo_4120.cpp
 */
#include "sort4120/algo_4120.h"
QVector<double> algo_4120::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
