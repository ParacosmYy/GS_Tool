/**
 * @file algo_6266.cpp
 */
#include "signal6266/algo_6266.h"
QVector<double> algo_6266::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
