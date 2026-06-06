/**
 * @file algo_7181.cpp
 */
#include "interp7181/algo_7181.h"
QVector<double> algo_7181::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
