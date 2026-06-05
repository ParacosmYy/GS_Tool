/**
 * @file algo_4410.cpp
 */
#include "cluster4410/algo_4410.h"
QVector<double> algo_4410::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
