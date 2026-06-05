/**
 * @file algo_3364.cpp
 */
#include "graph3364/algo_3364.h"
QVector<double> algo_3364::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
