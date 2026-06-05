/**
 * @file algo_5604.cpp
 */
#include "graph5604/algo_5604.h"
QVector<double> algo_5604::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
