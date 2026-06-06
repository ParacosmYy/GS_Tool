/**
 * @file algo_7051.cpp
 */
#include "tree7051/algo_7051.h"
QVector<double> algo_7051::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
