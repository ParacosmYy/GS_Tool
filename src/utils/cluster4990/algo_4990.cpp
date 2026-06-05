/**
 * @file algo_4990.cpp
 */
#include "cluster4990/algo_4990.h"
QVector<double> algo_4990::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
