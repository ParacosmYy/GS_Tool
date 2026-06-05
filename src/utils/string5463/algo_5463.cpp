/**
 * @file algo_5463.cpp
 */
#include "string5463/algo_5463.h"
QVector<double> algo_5463::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
