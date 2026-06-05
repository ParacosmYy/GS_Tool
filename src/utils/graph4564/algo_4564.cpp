/**
 * @file algo_4564.cpp
 */
#include "graph4564/algo_4564.h"
QVector<double> algo_4564::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
