/**
 * @file algo_3391.cpp
 */
#include "tree3391/algo_3391.h"
QVector<double> algo_3391::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
