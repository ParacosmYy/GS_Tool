/**
 * @file algo_7050.cpp
 */
#include "cluster7050/algo_7050.h"
QVector<double> algo_7050::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
