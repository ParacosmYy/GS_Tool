/**
 * @file algo_5411.cpp
 */
#include "tree5411/algo_5411.h"
QVector<double> algo_5411::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
