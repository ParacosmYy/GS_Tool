/**
 * @file algo_7324.cpp
 */
#include "graph7324/algo_7324.h"
QVector<double> algo_7324::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
