/**
 * @file algo_6990.cpp
 */
#include "cluster6990/algo_6990.h"
QVector<double> algo_6990::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
