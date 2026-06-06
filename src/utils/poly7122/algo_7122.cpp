/**
 * @file algo_7122.cpp
 */
#include "poly7122/algo_7122.h"
QVector<double> algo_7122::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
