/**
 * @file algo_7410.cpp
 */
#include "cluster7410/algo_7410.h"
QVector<double> algo_7410::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
