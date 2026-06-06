/**
 * @file algo_7711.cpp
 */
#include "tree7711/algo_7711.h"
QVector<double> algo_7711::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
