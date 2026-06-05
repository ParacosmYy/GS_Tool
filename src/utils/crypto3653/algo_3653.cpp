/**
 * @file algo_3653.cpp
 */
#include "crypto3653/algo_3653.h"
QVector<double> algo_3653::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
