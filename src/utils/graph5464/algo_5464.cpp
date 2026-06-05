/**
 * @file algo_5464.cpp
 */
#include "graph5464/algo_5464.h"
QVector<double> algo_5464::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
