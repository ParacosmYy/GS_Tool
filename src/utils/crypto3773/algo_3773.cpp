/**
 * @file algo_3773.cpp
 */
#include "crypto3773/algo_3773.h"
QVector<double> algo_3773::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
