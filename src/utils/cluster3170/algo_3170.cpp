/**
 * @file algo_3170.cpp
 */
#include "cluster3170/algo_3170.h"
QVector<double> algo_3170::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
