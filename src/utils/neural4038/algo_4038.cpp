/**
 * @file algo_4038.cpp
 */
#include "neural4038/algo_4038.h"
QVector<double> algo_4038::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
