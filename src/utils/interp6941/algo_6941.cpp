/**
 * @file algo_6941.cpp
 */
#include "interp6941/algo_6941.h"
QVector<double> algo_6941::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
