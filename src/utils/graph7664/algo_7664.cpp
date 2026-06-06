/**
 * @file algo_7664.cpp
 */
#include "graph7664/algo_7664.h"
QVector<double> algo_7664::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
