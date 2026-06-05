/**
 * @file algo_6246.cpp
 */
#include "signal6246/algo_6246.h"
QVector<double> algo_6246::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
