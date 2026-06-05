/**
 * @file algo_5461.cpp
 */
#include "interp5461/algo_5461.h"
QVector<double> algo_5461::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
