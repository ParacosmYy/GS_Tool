/**
 * @file algo_3253.cpp
 */
#include "crypto3253/algo_3253.h"
QVector<double> algo_3253::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
