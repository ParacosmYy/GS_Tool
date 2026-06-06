/**
 * @file algo_7654.cpp
 */
#include "numeric7654/algo_7654.h"
QVector<double> algo_7654::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
