/**
 * @file algo_6462.cpp
 */
#include "poly6462/algo_6462.h"
QVector<double> algo_6462::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
