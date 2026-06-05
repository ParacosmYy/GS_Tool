/**
 * @file algo_5824.cpp
 */
#include "graph5824/algo_5824.h"
QVector<double> algo_5824::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
