/**
 * @file algo_2914.cpp
 */
#include "numeric2914/algo_2914.h"
QVector<double> algo_2914::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
