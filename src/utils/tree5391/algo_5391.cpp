/**
 * @file algo_5391.cpp
 */
#include "tree5391/algo_5391.h"
QVector<double> algo_5391::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
