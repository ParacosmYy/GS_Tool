/**
 * @file algo_6391.cpp
 */
#include "tree6391/algo_6391.h"
QVector<double> algo_6391::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
