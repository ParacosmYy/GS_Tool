/**
 * @file algo_5550.cpp
 */
#include "cluster5550/algo_5550.h"
QVector<double> algo_5550::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
