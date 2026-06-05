/**
 * @file algo_5172.cpp
 */
#include "compress5172/algo_5172.h"
QVector<double> algo_5172::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
