/**
 * @file algo_3213.cpp
 */
#include "crypto3213/algo_3213.h"
QVector<double> algo_3213::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
