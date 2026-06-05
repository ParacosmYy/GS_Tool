/**
 * @file algo_5884.cpp
 */
#include "graph5884/algo_5884.h"
QVector<double> algo_5884::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
