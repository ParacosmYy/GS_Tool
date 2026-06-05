/**
 * @file algo_5266.cpp
 */
#include "signal5266/algo_5266.h"
QVector<double> algo_5266::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
