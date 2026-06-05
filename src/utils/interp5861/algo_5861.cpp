/**
 * @file algo_5861.cpp
 */
#include "interp5861/algo_5861.h"
QVector<double> algo_5861::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
