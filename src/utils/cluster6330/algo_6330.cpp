/**
 * @file algo_6330.cpp
 */
#include "cluster6330/algo_6330.h"
QVector<double> algo_6330::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
