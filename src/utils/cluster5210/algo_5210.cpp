/**
 * @file algo_5210.cpp
 */
#include "cluster5210/algo_5210.h"
QVector<double> algo_5210::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
