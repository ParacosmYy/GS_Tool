/**
 * @file algo_7071.cpp
 */
#include "tree7071/algo_7071.h"
QVector<double> algo_7071::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
