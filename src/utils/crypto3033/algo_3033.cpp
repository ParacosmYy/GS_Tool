/**
 * @file algo_3033.cpp
 */
#include "crypto3033/algo_3033.h"
QVector<double> algo_3033::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
