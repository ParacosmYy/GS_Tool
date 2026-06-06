/**
 * @file algo_7461.cpp
 */
#include "interp7461/algo_7461.h"
QVector<double> algo_7461::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
