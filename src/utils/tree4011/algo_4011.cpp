/**
 * @file algo_4011.cpp
 */
#include "tree4011/algo_4011.h"
QVector<double> algo_4011::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
