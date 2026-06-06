/**
 * @file algo_7002.cpp
 */
#include "poly7002/algo_7002.h"
QVector<double> algo_7002::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
