/**
 * @file algo_5330.cpp
 */
#include "cluster5330/algo_5330.h"
QVector<double> algo_5330::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
