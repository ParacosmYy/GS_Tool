/**
 * @file algo_4264.cpp
 */
#include "graph4264/algo_4264.h"
QVector<double> algo_4264::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
