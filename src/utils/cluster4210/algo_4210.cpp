/**
 * @file algo_4210.cpp
 */
#include "cluster4210/algo_4210.h"
QVector<double> algo_4210::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
