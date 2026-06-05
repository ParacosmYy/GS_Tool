/**
 * @file algo_3533.cpp
 */
#include "crypto3533/algo_3533.h"
QVector<double> algo_3533::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
