/**
 * @file algo_3104.cpp
 */
#include "graph3104/algo_3104.h"
QVector<double> algo_3104::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
