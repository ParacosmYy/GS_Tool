/**
 * @file algo_6641.cpp
 */
#include "interp6641/algo_6641.h"
QVector<double> algo_6641::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
