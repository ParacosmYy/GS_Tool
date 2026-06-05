/**
 * @file algo_6861.cpp
 */
#include "interp6861/algo_6861.h"
QVector<double> algo_6861::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
