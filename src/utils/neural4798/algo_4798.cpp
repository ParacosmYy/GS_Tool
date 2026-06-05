/**
 * @file algo_4798.cpp
 */
#include "neural4798/algo_4798.h"
QVector<double> algo_4798::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
