/**
 * @file algo_4796.cpp
 */
#include "geometry4796/algo_4796.h"
QVector<double> algo_4796::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
