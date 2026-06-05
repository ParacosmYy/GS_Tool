/**
 * @file algo_3673.cpp
 */
#include "crypto3673/algo_3673.h"
QVector<double> algo_3673::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
