/**
 * @file algo_6741.cpp
 */
#include "interp6741/algo_6741.h"
QVector<double> algo_6741::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
