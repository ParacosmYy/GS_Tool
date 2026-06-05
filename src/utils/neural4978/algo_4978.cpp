/**
 * @file algo_4978.cpp
 */
#include "neural4978/algo_4978.h"
QVector<double> algo_4978::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
