/**
 * @file algo_3053.cpp
 */
#include "crypto3053/algo_3053.h"
QVector<double> algo_3053::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
