/**
 * @file algo_4364.cpp
 */
#include "graph4364/algo_4364.h"
QVector<double> algo_4364::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
