/**
 * @file algo_7411.cpp
 */
#include "tree7411/algo_7411.h"
QVector<double> algo_7411::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
