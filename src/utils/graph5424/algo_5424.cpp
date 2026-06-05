/**
 * @file algo_5424.cpp
 */
#include "graph5424/algo_5424.h"
QVector<double> algo_5424::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
