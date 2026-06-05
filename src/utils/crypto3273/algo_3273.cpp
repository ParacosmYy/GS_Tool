/**
 * @file algo_3273.cpp
 */
#include "crypto3273/algo_3273.h"
QVector<double> algo_3273::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
