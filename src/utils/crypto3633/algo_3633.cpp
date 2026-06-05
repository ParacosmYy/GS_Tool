/**
 * @file algo_3633.cpp
 */
#include "crypto3633/algo_3633.h"
QVector<double> algo_3633::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
