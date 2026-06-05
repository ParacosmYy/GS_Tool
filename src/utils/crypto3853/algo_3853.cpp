/**
 * @file algo_3853.cpp
 */
#include "crypto3853/algo_3853.h"
QVector<double> algo_3853::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
