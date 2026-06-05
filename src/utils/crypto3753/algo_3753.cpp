/**
 * @file algo_3753.cpp
 */
#include "crypto3753/algo_3753.h"
QVector<double> algo_3753::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
