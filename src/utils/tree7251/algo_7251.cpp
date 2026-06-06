/**
 * @file algo_7251.cpp
 */
#include "tree7251/algo_7251.h"
QVector<double> algo_7251::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
