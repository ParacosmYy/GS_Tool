/**
 * @file algo_7582.cpp
 */
#include "poly7582/algo_7582.h"
QVector<double> algo_7582::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
