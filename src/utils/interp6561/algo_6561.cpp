/**
 * @file algo_6561.cpp
 */
#include "interp6561/algo_6561.h"
QVector<double> algo_6561::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
