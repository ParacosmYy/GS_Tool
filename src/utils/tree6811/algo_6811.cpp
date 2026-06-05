/**
 * @file algo_6811.cpp
 */
#include "tree6811/algo_6811.h"
QVector<double> algo_6811::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
