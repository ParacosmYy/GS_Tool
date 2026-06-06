/**
 * @file algo_7462.cpp
 */
#include "poly7462/algo_7462.h"
QVector<double> algo_7462::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
